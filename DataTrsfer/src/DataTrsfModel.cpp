
// ********************************************************************
// * 소 스 명: DataTrsfModel.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 01
// ********************************************************************
#include <unistd.h>
#include <thread>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <functional>

#include "DataTrsfModel.h"
#include "MACRO.h"
#include "CommonUtil.h"
#include "Config.h"
#include "PacketUtil.h"
#include "ThreadPool.h"
#include "NetObjectInfo.h"
#include "SharedMemControl.h"


using std::string;
using std::to_string;
using namespace DATATRSFER;
using namespace IPCSHAREDMEM;

// ***************************************************************
// * 함 수 명: Initialize
// * 설    명: DataTrsfModel을 초기화한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 01
// ****************************************************************
void DataTrsfModel::Initialize(const int32_t stnStartMode) 
{
    //실행전에 맴버변수 초기화
    m_clServerInfo.m_nProgramType = CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM;

    m_pLogManager = std::make_unique<LogManager>();
    m_pLogManager->InitializeLog();
    
    m_nDataTrsfStartMode = stnStartMode;

    //epoll setting
    m_nEpollFD = epoll_create1(0);

    if (stnStartMode == DATATRSF_START_MODE::INTERNAL_MODE)
    {
        //InitInternalProcess();
        //인터널 모드(상위서버)에서는 사용하지 않는다.
        m_nUpperServerSocket = -1;
        m_nUpperUdpSocket = -1;
        m_nUpperClientFd = -1;
    }
    m_bIsExit.store(false);
    
    //외부 클라이언트와 통신할 소켓을 초기화 한다.
    InitExternalServerSocket(stnStartMode);
    InitExternalUdpSocket(stnStartMode);
}

// ***************************************************************
// * 함 수 명: InitInternalProcess
// * 설    명: 내부 서버와 ipc통신연결을 초기화한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 14
// ***************************************************************
// * 설    명: 데이터베이스 서버연결을 초기화한다.
// * 수 정 자: KJH
// * 작성일자: 2025. 09. 23
// ***************************************************************
void DataTrsfModel::InitInternalProcess()
{
    m_pShmManager = std::make_unique<SharedMemControl>();
    m_pShmManager->InitializeShm(); 

    m_pDatabaseManager = std::make_unique<CDatabase>();
    bool_t ret_i = m_pDatabaseManager->InitializeDB();
    if (ret_i == false)
    {
        PrintErrorLog("fail Database initialize");
    }
    
}

// ***************************************************************
// * 함 수 명: ConnectUpperServer
// * 설    명: UpperServer와 연결을 맺는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 15
// ***************************************************************
void DataTrsfModel::ConnectUpperServer()
{
    auto& pConfig = CONFIG::ConfigManager::GetInstance();
    string strSectionName = DATATRSFER::EXT_SECTION_NAME;
    string strConfigPath  = DATATRSFER::CONFIG_PATH;
    string strIPKeyName = DATATRSFER::UPPER_IP_ADDRESS;
    string strPOKeyName = DATATRSFER::UPPER_PORT;
    string strUdpPOKeyname = DATATRSFER::UPPER_UDP_PORT; 

    const string strIpAddress = pConfig.GetValue(strIPKeyName,strSectionName,strConfigPath);
    const string strPortNo = pConfig.GetValue(strPOKeyName,strSectionName,strConfigPath);
    const string strUdpPortNo = pConfig.GetValue(strUdpPOKeyname,strSectionName,strConfigPath);
    PrintInfoLog("UPPER_IP_ADDRESS : " + strIpAddress);
    PrintInfoLog("UPPER_PORT : " + strPortNo);
    PrintInfoLog("UPPER_UDP_PORT : " + strUdpPortNo);

    //upper tcp socket 초기화
    m_nUpperServerSocket = socket(AF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in strtUpperServerAddr;

    void* ret_m = memset(&strtUpperServerAddr, 0, sizeof(strtUpperServerAddr));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 
    
    strtUpperServerAddr.sin_family = AF_INET;
	strtUpperServerAddr.sin_addr.s_addr = inet_addr(strIpAddress.c_str());
	strtUpperServerAddr.sin_port = htons(stoi(strPortNo, nullptr));
    /// TODO: UpperServer소켓도 옵션셋팅해야 하는걸까...?
    //SetSockOptValue(m_nUpperServerSocket);

    m_nUpperUdpSocket = socket(AF_INET,SOCK_DGRAM,0);
       
    struct sockaddr_in strtUpperUdpAddr;

    ret_m = memset(&strtUpperUdpAddr, 0, sizeof(strtUpperUdpAddr));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 
    
    strtUpperUdpAddr.sin_family = AF_INET;
	strtUpperUdpAddr.sin_addr.s_addr = inet_addr(strIpAddress.c_str());
	strtUpperUdpAddr.sin_port = htons(stoi(strUdpPortNo, nullptr));

    m_strtUpperUdpAddr = strtUpperUdpAddr;
    /// TODO: UpperServer소켓도 옵션셋팅해야 하는걸까...?
    //SetSockOptValue(m_nUpperUdpSocket);

    const int_t nConnectResult = connect(m_nUpperServerSocket, reinterpret_cast<sockaddr*>(&strtUpperServerAddr), sizeof(strtUpperServerAddr));
    if (-1 == nConnectResult)
    {
        PrintErrorLog("UpperServer Connect error");
        close(m_nUpperServerSocket);
        return;
    }

    PrintInfoLog("Connect UpperServer");

    //connect가 되면 자기의 정보를 보내야한다.
    /////////////////////////////연결후 Udp정보를 교환하는 과정까지.
    /// TODO: 키값을 더 안전하게 할 수 있는 방법을 생각해보자...
    string strKeySectionName = DATATRSFER::KEY_SECTION_NAME;
    string strKeyName        = DATATRSFER::DATATRSFER_KEY;

    const string strKeyNo = pConfig.GetValue(strKeyName,strKeySectionName,strConfigPath);

    PrintInfoLog("KEY : " + strKeyNo);

    //UpperServer에게 연결요청!
    //전송 큐를 사용해도 되겠지만, 제일 먼저의 연결일 것이므로 그냥 send하자
    StPacket strtRequestPacket;
    PacketUtil::CreateRequestConnectPacket(std::stoi(strKeyNo),&strtRequestPacket);
    ssize_t byte_v = send(m_nUpperServerSocket,&strtRequestPacket,sizeof(strtRequestPacket),0);
    if (byte_v == -1)
    {
        /// TODO: send가 실패하면 계속 접속시도해야하나...? send가 실패한다는 것이 정확히 어떤것을
        //의미하는지 정리하자
        PrintErrorLog("UpperServerConnect() Fail...");
        PrintErrorLog("send() fail");
        return;
    }

    //상위 서버의 연결 응답을 기다린다.
    char_t szReadBuffer[BUFSIZE];
    ssize_t nTotalReadLength = 0;
    ssize_t nReadLength = 0;
    while((nReadLength = recv(m_nUpperServerSocket, szReadBuffer, sizeof(szReadBuffer),0)) > 0) 
    {
        nTotalReadLength+=nReadLength;
        if (nTotalReadLength != sizeof(StPacket))
        {
            continue;
            //packet을 다받을 때까지 기다린다. 패킷이 쪼개져서 올 수도 있으니...
        }
        StPacketHeader* strtRcvPacket = reinterpret_cast<StPacketHeader*>(szReadBuffer);

        ProcessUpperTcpPacket(szReadBuffer,nTotalReadLength);
        nTotalReadLength=0;
        
        //상위 서버와 udp정보를 다 교환했다면,
        if (strtRcvPacket->nType == HELLO_UDP_RESPONSE) 
        {
            break;
        }
    }

    /////////////////////////////////////////일관된 처리를 위해 상위서버소켓을 epoll에 등록한다.
    ////////////////////////////Upper Tcp Socket net object  등록 (epoll 등록과 같음)
    auto clConnObjInfo = std::make_shared<NetObjectInfo>();
        
	clConnObjInfo->m_nTcpSocket = m_nUpperServerSocket;
	clConnObjInfo->m_nConnStatus = CLIENT_STATE::CONNECT;
    clConnObjInfo->m_nProgramType = CLIENT_TYPE::UPPER_ENGINE_SERVER;
	clConnObjInfo->m_strIpAddress = inet_ntoa(strtUpperServerAddr.sin_addr);			
	clConnObjInfo->m_LoginTIME = time(nullptr);
    clConnObjInfo->m_strtUdpAddr = m_strtUpperUdpAddr;
    RegisterNetObjectInfo(clConnObjInfo);
    
    ////////////////////////////Upper Udp Socket epoll 등록
    epoll_event strtEpollEvent;
    ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    strtEpollEvent.events = EPOLLIN | EPOLLET;
    strtEpollEvent.data.fd = m_nUpperUdpSocket;
    
    //socket을 넌블락킹으로 설정
    fcntl(m_nUpperUdpSocket, F_SETFL, fcntl(m_nUpperUdpSocket,F_GETFL,0) | O_NONBLOCK);
    epoll_ctl(m_nEpollFD, EPOLL_CTL_ADD, m_nUpperUdpSocket, &strtEpollEvent);

    
}

// ***************************************************************
// * 함 수 명: InternalProcessSendRoutine
// * 설    명: 내부서버에 데이터를 보낸다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 14
// ***************************************************************
void DataTrsfModel::InternalProcessSendRoutine(const char_t* pSendingData, size_t nSendDataLen)
{
    const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pSendingData);
    /// TODO: 나중에 필요하다면 헤더파싱후 다른 로직을 짠다.
    m_pShmManager->PutWriteShm(pSendingData,nSendDataLen);
    /*
    if (pPacketHeader->nType == PACKET_TYPE::REQUEST_SIM_START)
    {
        const StStartRequestEnginePacket* pRequestEngineStartPacket = reinterpret_cast<const StStartRequestEnginePacket*>(pSendingData);
        m_pShmManager->PutWriteShm(pSendingData,nSendDataLen);
        PrintDebugLog("Send Internal Shared Memory Data : simulation start");
    */
}

// ***************************************************************
// * 함 수 명: InternalProcessReceiveRoutine
// * 설    명: 내부서버가 보낸 데이터를 받는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 14
// ***************************************************************
// * 설    명: 엔진과 데이터베이스 서버간 패킷 연결을 처리한다.
// * 수 정 자: KJH
// * 수정일자: 2025. 09. 23
// ****************************************************************
void DataTrsfModel::InternalProcessReceiveRoutine()
{
    char_t czReadBuffer[BUFSIZE];

    void* ret_m = memset(czReadBuffer, 0, sizeof(BUFSIZE));
    if( nullptr == ret_m )  PrintErrorLog("Fail to memset()");

    int_t nTotalBytes = m_pShmManager->GetReadShm(czReadBuffer, sizeof(czReadBuffer));

    int_t offset = 0;
    while (offset < nTotalBytes) {
        StPacketHeader* pPacket = reinterpret_cast<StPacketHeader*>(czReadBuffer + offset);

        //simengine에서 부대정보요청 응답
        if (pPacket->nType == PACKET_TYPE::RESPONSE_MILITARY_UNIT_INFO)
        { 
            StResponseLoadMilitaryUnitInfoPacket* pPacket = reinterpret_cast<StResponseLoadMilitaryUnitInfoPacket*>(czReadBuffer + offset);
            PrintDebugLog("Response militaryUnit info");
            StResponseLoadMilitaryUnitInfoPacket strtPacket;

            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);

            PacketUtil::CreateResponseLoadMilitaryUnitPacket(nClientId,pPacket, &strtPacket);
/*
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false) 
            {
                //라우팅 정보에 없는 (강제 종료 했거나) user client 이므로 패킷을 보내지않음. 또는 그에 따른 처리를 이곳에서 한다.
                offset +=SHM_DATA_SIZE;
                continue;
            }
*/
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StResponseLoadMilitaryUnitInfoPacket));
            
            offset +=SHM_DATA_SIZE;
        }
        //simulation start response 수신
        else if (pPacket->nType == PACKET_TYPE::RESPONSE_SIM_START)
        {
            StStartResponseEnginePacket* pPacket = reinterpret_cast<StStartResponseEnginePacket*>(czReadBuffer + offset);
            //offset += sizeof(StStartResponseEnginePacket);
            offset +=SHM_DATA_SIZE;
            PrintDebugLog("Response simulation start!!");

            StStartResponseEnginePacket strtResponsePacket;
            PacketUtil::CreateResponseSimulationStartPacket(&strtResponsePacket);
            //boardcast로 보내야댐. broadcast function을 새로 나중에 만들자.
            for (auto NetObj : m_mNetInfo)
            {
                int32_t NetID = NetObj.first;
                //자기자신이면 넘어감
                if (NetID == m_nExternalServerSocket) continue;

                StockPacketToClientSendingQueue(NetID,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
            }

        }
        //출격명령 응답 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::RESPONSE_ORDER_IN_SALLY)
        { 
            StResponseOrderInSallyPacket* pPacket = reinterpret_cast<StResponseOrderInSallyPacket*>(czReadBuffer + offset);
            PrintDebugLog("Response order in sally");
            StResponseOrderInSallyPacket strtPacket;
            PacketUtil::CreateResponseOrderInSallyPacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);
            
            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
/*
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false) 
            {
                //라우팅 정보에 없는 (강제 종료 했거나) user client 이므로 패킷을 보내지않음. 또는 그에 따른 처리를 이곳에서 한다.
                offset +=SHM_DATA_SIZE;
                continue;
            }
*/
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StResponseOrderInSallyPacket));
            offset +=SHM_DATA_SIZE;
        }
        //미사일 발사명령 응답 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::RESPONSE_ORDER_IN_ATTACK)
        { 
            StResponseOrderInAttackPacket* pPacket = reinterpret_cast<StResponseOrderInAttackPacket*>(czReadBuffer + offset);
            PrintDebugLog("Response order in attack");
            StResponseOrderInAttackPacket strtPacket;
            PacketUtil::CreateResponseOrderInAttackPacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);
            
            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
            //PrintDebugLog("artillery id : "+to_string(ntohs(strtPacket.strtArtilleryInfo.nID)));
            //PrintDebugLog("artillery total missile : "+to_string(ntohl(strtPacket.strtArtilleryInfo.nTotalMissile)));
            //PrintDebugLog("missile id : "+to_string(ntohl(strtPacket.strtMissileInfo.nID)));
/*
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false) 
            {
                //라우팅 정보에 없는 (강제 종료 했거나) user client 이므로 패킷을 보내지않음. 또는 그에 따른 처리를 이곳에서 한다.
                offset +=SHM_DATA_SIZE;
                continue;
            }
*/
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StResponseOrderInAttackPacket));
            offset +=SHM_DATA_SIZE;
        }
        //자주포 위치 동기화 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::SYNC_ARTILLERY_COORDINATE)
        { 
            StSyncArtilleryCoordinatePacket* pPacket = reinterpret_cast<StSyncArtilleryCoordinatePacket*>(czReadBuffer + offset);
            PrintDebugLog("artillery unit moved packet recv");
            StSyncArtilleryCoordinatePacket strtPacket;
            PacketUtil::CreateSyncArtilleryCoordinatePacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);

            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
/*            
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false)
            {
                //routing info에 없는상황
                offset +=SHM_DATA_SIZE;
                continue;
            }
*/
            //tcp로 보낼 시
            if (pPacket->strtHeader.nFlagField == NETWORK_FLAG::TCP)
            {
                StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncArtilleryCoordinatePacket));
            }
            //udp로 보낼 시 
            else if (pPacket->strtHeader.nFlagField == NETWORK_FLAG::UDP)
            {
                ExternalSocketProcessSendtoRoutine(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncArtilleryCoordinatePacket));
            }
            offset +=SHM_DATA_SIZE;
        }
        //미사일 위치 동기화 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::SYNC_MISSILE_COORDINATE)
        { 
            StSyncMissileCoordinatePacket* pPacket = reinterpret_cast<StSyncMissileCoordinatePacket*>(czReadBuffer + offset);
            PrintDebugLog("Missile moved packet recv");
            StSyncMissileCoordinatePacket strtPacket;
            PacketUtil::CreateSyncMissileCoordinatePacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);

            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
/*            
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false)
            {
                //routing info에 없는상황
                offset +=SHM_DATA_SIZE;
                continue;
            }
*/
            //tcp로 보낼 시
            if (pPacket->strtHeader.nFlagField == NETWORK_FLAG::TCP)
            {
                StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncMissileCoordinatePacket));
            }
            //udp로 보낼 시 
            else if (pPacket->strtHeader.nFlagField == NETWORK_FLAG::UDP)
            {
                ExternalSocketProcessSendtoRoutine(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncMissileCoordinatePacket));
            }
            offset +=SHM_DATA_SIZE;
        }
        //자주포 피격 이벤트 동기화 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::SYNC_ARTILLERY_ATTACKED)
        { 
            StSyncArtilleryAttackedPacket* pPacket = reinterpret_cast<StSyncArtilleryAttackedPacket*>(czReadBuffer + offset);
            PrintDebugLog("artillery unit attacked packet recv");
            StSyncArtilleryAttackedPacket strtPacket;
            PacketUtil::CreateSyncArtilleryAttackedPacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);

            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
/*          
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false)
            {
                //routing info에 없는상황
                offset +=SHM_DATA_SIZE;
                continue;

            }                
*/
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncArtilleryAttackedPacket));
            offset +=SHM_DATA_SIZE;
        }
        //부대 피격 이벤트 동기화 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::SYNC_MILITARY_UNIT_ATTACKED)
        { 
            StSyncMilitaryUnitAttackedPacket* pPacket = reinterpret_cast<StSyncMilitaryUnitAttackedPacket*>(czReadBuffer + offset);
            PrintDebugLog("military unit attacked packet recv");
            StSyncMilitaryUnitAttackedPacket strtPacket;
            PacketUtil::CreateSyncMilitaryUnitAttackedPacket(ntohl(pPacket->strtHeader.nID),pPacket, &strtPacket);

            int32_t nClientId = ntohl(pPacket->strtHeader.nID);
            int32_t nUserConnectId = ntohl(pPacket->nUserClientFd);
/*            
            if (IsThereNetRoutingInfo(nClientId,nUserConnectId) == false)
            {
                //routing info에 없는상황
                offset +=SHM_DATA_SIZE;
                continue;
            }                
*/
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncMilitaryUnitAttackedPacket));
            offset +=SHM_DATA_SIZE;
        }
        //엔진 시간 동기화 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::SYNC_SIM_TIME)
        { 
            StSyncSimulationTimePacket* pPacket = reinterpret_cast<StSyncSimulationTimePacket*>(czReadBuffer + offset);
            //PrintDebugLog("sync engine time packet recv");
            StSyncSimulationTimePacket strtPacket;
            PacketUtil::CreateSyncSimulationTimePacket(&strtPacket, pPacket);
            
            //boardcast로 보내야댐.
            for (auto NetObj : m_mNetInfo)
            {
                int32_t NetID = NetObj.first;
                //자기자신이면 넘어감
                if (NetID == m_nExternalServerSocket) continue;

                StockPacketToClientSendingQueue(NetID,reinterpret_cast<char_t*>(&strtPacket),sizeof(strtPacket));
            }
            offset +=SHM_DATA_SIZE;
        }
        //엔진의 부대정보 요청 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::REQUEST_LOAD_MILITARY_UNIT_FROM_DB)
        {
            PrintDebugLog("load military info from database packet recv");
            auto militaryUnits = m_pDatabaseManager->DownloadMilitaryUnitInfo();
            int_t nLength      = militaryUnits.size();
            int_t nCount       = 0;
            for (auto militaryUnit : militaryUnits)
            {
                nCount++;
                int8_t bIsFinish = NETWORK_FLAG::NONE;
                if (nCount == nLength)
                {
                    bIsFinish = NETWORK_FLAG::FIN;
                }
                StResponseLoadMilitaryUnitInfoFromDBPacket strtResponseFromDatabasePacket;
                PacketUtil::CreateResponseLoadMilitaryUnitFromDB(bIsFinish, &militaryUnit,&strtResponseFromDatabasePacket);
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtResponseFromDatabasePacket),sizeof(strtResponseFromDatabasePacket));
            }
            offset+=SHM_DATA_SIZE;
        }
        else
        {
            PrintErrorLog("not found packet id");
            offset +=SHM_DATA_SIZE;
        }

    }

}

// ***************************************************************
// * 함 수 명: InitExternalServerSocket
// * 설    명: 클라이언트와 통신할 Tcp소켓을 초기화하고 accept상태로 만든다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 02
// ***************************************************************
void DataTrsfModel::InitExternalServerSocket(const int32_t stnStartMode) 
{
    auto& pConfig = CONFIG::ConfigManager::GetInstance();
    string strSectionName = stnStartMode == 0 ? DATATRSFER::INT_SECTION_NAME : DATATRSFER::EXT_SECTION_NAME;
    string strConfigPath  = DATATRSFER::CONFIG_PATH;
    string strIPKeyName = stnStartMode == 0 ? DATATRSFER::IP_ADDRESS : DATATRSFER::EXTERNAL_IP_ADDR;
    string strPOKeyName = stnStartMode == 0 ? DATATRSFER::PORT : DATATRSFER::EXTERNAL_PORT;

    const string strIpAddress = pConfig.GetValue(strIPKeyName,strSectionName,strConfigPath);
    const string strPortNo = pConfig.GetValue(strPOKeyName,strSectionName,strConfigPath);
    PrintInfoLog("EXTERNAL_IP_ADDRESS : " + strIpAddress);
    PrintInfoLog("EXTERNAL_PORT : " + strPortNo);
    
    //external tcp socket 초기화
    m_nExternalServerSocket = socket(AF_INET,SOCK_STREAM,0);
    
    
    struct sockaddr_in strtServerAddr;

    void* ret_m = memset(&strtServerAddr, 0, sizeof(strtServerAddr));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 
    
    strtServerAddr.sin_family = AF_INET;
	strtServerAddr.sin_addr.s_addr = inet_addr(strIpAddress.c_str());
	strtServerAddr.sin_port = htons(stoi(strPortNo, nullptr));

    SetSockOptValue(m_nExternalServerSocket);

    m_clServerInfo.m_strIpAddress = strIpAddress;
    m_clServerInfo.m_nTcpSocket   = m_nExternalServerSocket;

    int_t nBindResult = bind(m_nExternalServerSocket, reinterpret_cast<struct sockaddr*>(&strtServerAddr), sizeof(strtServerAddr));
    if (-1 == nBindResult) PrintErrorLog("Fail to bind()");
	int_t nListenResult = listen(m_nExternalServerSocket, DATATRSFER::MAX_BACKLOG); //TODO:listen등 소켓 옵션들을 공부하쟈.
    if (-1 == nListenResult) PrintErrorLog(" Fail to listen()");

    epoll_event strtEpollEvent;
    ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    strtEpollEvent.events = EPOLLIN | EPOLLET;
    strtEpollEvent.data.fd = m_nExternalServerSocket;
    
    //socket을 넌블락킹으로 설정
    fcntl(m_nExternalServerSocket, F_SETFL, fcntl(m_nExternalServerSocket,F_GETFL,0) | O_NONBLOCK);
    epoll_ctl(m_nEpollFD, EPOLL_CTL_ADD, m_nExternalServerSocket, &strtEpollEvent);
}

// ***************************************************************
// * 함 수 명: InitExternalUdpSocket
// * 설    명: 클라이언트와 통신할 Udp소켓을 초기화한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 09
// ***************************************************************
void DataTrsfModel::InitExternalUdpSocket(const int32_t stnStartMode) 
{
    auto& pConfig = CONFIG::ConfigManager::GetInstance();
    string strSectionName = stnStartMode == 0 ? DATATRSFER::INT_SECTION_NAME : DATATRSFER::EXT_SECTION_NAME;
    string strConfigPath  = DATATRSFER::CONFIG_PATH;
    string strIPKeyName = stnStartMode == 0 ? DATATRSFER::IP_ADDRESS : DATATRSFER::EXTERNAL_IP_ADDR;
    string strPOKeyName = stnStartMode == 0 ? DATATRSFER::UDP_PORT : DATATRSFER::EXTERNAL_UDP_PORT;

    const string strIpAddress = pConfig.GetValue(strIPKeyName,strSectionName,strConfigPath);
    const string strPortNo = pConfig.GetValue(strPOKeyName,strSectionName,strConfigPath);
    PrintInfoLog("EXTERNAL_IP_ADDRESS : " + strIpAddress);
    PrintInfoLog("EXTERNAL_UDP_PORT : " + strPortNo);
    
    //external udp socket 초기화
    m_nExternalUdpSocket = socket(AF_INET,SOCK_DGRAM,0);
    
    
    struct sockaddr_in strtServerAddr;

    void* ret_m = memset(&strtServerAddr, 0, sizeof(strtServerAddr));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 
    
    strtServerAddr.sin_family = AF_INET;
	strtServerAddr.sin_addr.s_addr = inet_addr(strIpAddress.c_str());
	strtServerAddr.sin_port = htons(stoi(strPortNo, nullptr));

    SetSockOptValue(m_nExternalUdpSocket);

    m_clServerInfo.m_strIpAddress = strIpAddress;
    m_clServerInfo.m_strtUdpAddr  = strtServerAddr;

    int_t nBindResult = bind(m_nExternalUdpSocket, reinterpret_cast<struct sockaddr*>(&strtServerAddr), sizeof(strtServerAddr));
    if (-1 == nBindResult) PrintErrorLog("Fail to bind()");

    epoll_event strtEpollEvent;
    ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));
	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    strtEpollEvent.events = EPOLLIN | EPOLLET;
    strtEpollEvent.data.fd = m_nExternalUdpSocket;
    
    //socket을 넌블락킹으로 설정
    fcntl(m_nExternalUdpSocket, F_SETFL, fcntl(m_nExternalUdpSocket,F_GETFL,0) | O_NONBLOCK);
    epoll_ctl(m_nEpollFD, EPOLL_CTL_ADD, m_nExternalUdpSocket, &strtEpollEvent);
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessAcceptRoutine
// * 설    명: 외부 네트워크 객체 접속을 accept한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 08
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessAcceptRoutine() 
{
    //ServerSocket이 non-blocking 이고, epoll event가 엣지 트리거 이므로 루프.
    while(true) 
    {
        struct sockaddr_in strtClientAddr;
	    int_t nClientSocket = 0;
	    uint32_t nClientAddrLength = sizeof(strtClientAddr);

        void* ret_m = memset(&strtClientAddr, 0, nClientAddrLength);
        if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

        nClientSocket = accept(m_nExternalServerSocket, reinterpret_cast<struct sockaddr*>(&strtClientAddr), &nClientAddrLength);
        if (nClientSocket < 0)
        {
            //만약 수신버퍼에 있는 모든 연결을 수락했다면
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;

            PrintErrorLog("fail to accept() ");
            return;
        }

        //SO_LINGER option 추가
        struct linger ling;
        ling.l_onoff = 1;
        ling.l_linger = 0;

        setsockopt( nClientSocket , SOL_SOCKET, SO_LINGER , &ling, sizeof(ling));
        
	    auto clConnObjInfo = std::make_shared<NetObjectInfo>();
        
	    clConnObjInfo->m_nTcpSocket = nClientSocket;
	    clConnObjInfo->m_nConnStatus = CLIENT_STATE::CONNECT_WAIT;
        clConnObjInfo->m_nProgramType = CLIENT_TYPE::NONE_SYSTEM;
	    clConnObjInfo->m_strIpAddress = inet_ntoa(strtClientAddr.sin_addr);			
	    clConnObjInfo->m_LoginTIME = time(nullptr);
        /// TODO: 검증된 사용자인지 검증후 등록
        RegisterNetObjectInfo(clConnObjInfo);
        PrintInfoLog("Connect Client IP : " + clConnObjInfo->m_strIpAddress + " , SocketId : " + to_string(clConnObjInfo->m_nTcpSocket));
        //send request 사용자 아이디 비번 등...

    }
    
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessSendRoutine
// * 설    명: 연결된 네트워크 객체에 패킷을 보낸다. EPOLLOUT이벤트 발생 시
// *          실질적으로 전송큐에 저장되어있는 것을 순차적으로 보내는 역할을 한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 10
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessSendRoutine(int32_t nClientFd) 
{  
    std::shared_ptr<NetObjectInfo> clClientObjectInfo = nullptr;
    
    {
        ////////////////////////////////////////////////////////////////////////////////////////critical section
        std::lock_guard<mutex> lock(m_NetMapMutex);
        if (m_mNetInfo.find(nClientFd) == m_mNetInfo.end()) 
        {
            PrintErrorLog("ExternalSocketProcessSendRoutine() : not found clientID");
            return;
        }
        clClientObjectInfo = m_mNetInfo[nClientFd];
    }
    
    /// TODO: 동시에 m_mNetInfo[nClient]에 접근할 가능성?? 읽기만 한다면 문제없음 그러나
    //동시에 클래스 값을 바꾼다거나...하는 경우가 있는지 생각해볼 문제...
    //하지만 애초에 m_mNetInfo 자체에 lock을 걸면 비효율적, 객체의 전송 큐접근시 lock을 걸까?
    //나중에 더 좋은 방안을 강구해보자...
    //auto clClientObjectInfo = m_mNetInfo[nClientFd];

    std::lock_guard<std::mutex> lockClientSendQueue(clClientObjectInfo->m_mutexQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    while(clClientObjectInfo->HasPendingData())
    {
        //PrintDebugLog("sending queue size : "+to_string(clClientObjectInfo->m_deqSendQueue.size()));
        std::vector<char_t>& vecSendBuffer = clClientObjectInfo->PeekFrontQueue();
        const char_t* pSendDataPtr = vecSendBuffer.data() + clClientObjectInfo->m_nSentBytes;
        size_t nSendDataLen = vecSendBuffer.size() - clClientObjectInfo->m_nSentBytes;

        ///TODO: send의 flag 찾아보기
        ssize_t byte_v = send(nClientFd, pSendDataPtr, nSendDataLen, 0);
        //PrintDebugLog("actually send");
        if (byte_v > 0) 
        {
            clClientObjectInfo->m_nSentBytes += byte_v;
            if (clClientObjectInfo->m_nSentBytes == vecSendBuffer.size())
            {
                clClientObjectInfo->PopSendQueue();
                clClientObjectInfo->m_nSentBytes = 0;
            }
        }
        else 
        {
            //전송버퍼가 다찼거나 보낼데이터 없음... 다음 out이벤트를 기다리자
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            
            PrintErrorLog("send() err");
            RemoveNetObjectInfo(nClientFd);
            return;
        }
    }
    //모든 전송이 끝이라면 EPOLLOUT 이벤트 받지 말기.
    epoll_event strtEpollEvent;
    void* ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));
    if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    strtEpollEvent.events = EPOLLIN | EPOLLET;
    
    strtEpollEvent.data.fd = nClientFd;
    epoll_ctl(m_nEpollFD, EPOLL_CTL_MOD, nClientFd, &strtEpollEvent);
    
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessSendtoRoutine
// * 설    명: 연결된 네트워크 객체에 udp패킷을 보낸다. 
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 13
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessSendtoRoutine(int32_t nClientFd, const char_t* pSendingData, size_t nSendDataLen) 
{
    sockaddr_in strtClientAddr;
    try
    {
        auto clConnObjInfo = m_mNetInfo.at(nClientFd);
        strtClientAddr = clConnObjInfo->m_strtUdpAddr;
    }
    //만일 nClientFd가 종료된 상태라면
    catch(const std::out_of_range& eErrorMessage)
    {
        //상위서버에 없다고 패킷을 보내야함..
        PrintErrorLog("Do not Exist ClientFd in Session Infomation!");
        return;
    }

    /// TODO: sendto option check
    ssize_t byte_v = sendto(m_nExternalUdpSocket, pSendingData, nSendDataLen, 0, reinterpret_cast<sockaddr*>(&strtClientAddr), sizeof(strtClientAddr));
    if (byte_v < 0) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            PrintDebugLog("sendto buffer is full....");
            /// TODO: 송신 버퍼가 꽉 참. 재시도 로직 큐에 등록하거나 다른 처리가 필요
        } 
        else 
        {
            PrintErrorLog("sendto() fail");
        }
    }


    
}

// ***************************************************************
// * 함 수 명: ProcessSendtoRoutineToUpperServer
// * 설    명: 상위 데이터 전달 처리 서버에 udp패킷을 보낸다. 
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 24
// ****************************************************************
void DataTrsfModel::ProcessSendtoRoutineToUpperServer(const char_t* pSendingData, size_t nSendDataLen) 
{
    /// TODO: sendto option check
    ssize_t byte_v = sendto(m_nUpperUdpSocket, pSendingData, nSendDataLen, 0, reinterpret_cast<sockaddr*>(&m_strtUpperUdpAddr), sizeof(m_strtUpperUdpAddr));
    if (byte_v < 0) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            PrintDebugLog("sendto buffer is full....");
            /// TODO:송신 버퍼가 꽉 참. 재시도 로직 큐에 등록하거나 다른 처리가 필요
            return;
        } 
        else 
        {
            PrintErrorLog("sendto() fail");
            return;
        }
    }

    
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessRecvRoutine
// * 설    명: 연결된 네트워크 객체의 패킷을 받는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 08
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessRecvRoutine(const int_t nClientFd) 
{
    char_t szReadBuffer[BUFSIZE];
    while(true) 
    {
        ///TODO:recv flag에 대하여 공부
        ssize_t nReadLength = recv(nClientFd,szReadBuffer,sizeof(szReadBuffer),0);
        if (nReadLength < 0) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            PrintErrorLog("fail to recv()");
            if (m_nUpperServerSocket == nClientFd)
            {
                ///TODO: 종료한게 UpperServer라는뜻... 서버는 중지되면 안됨. 새로운 처리가 필요...
                PrintInfoLog("Unexpected Disconnect UpperServer SocketId : " + to_string(m_nUpperServerSocket));
                return;
            }
            PrintInfoLog("Unexpected Disconnect Client SocketId : " + to_string(nClientFd));
            RemoveNetObjectInfo(nClientFd);
            return;
        }
        else if (nReadLength == 0)  //client가 연결을 종료함 
        {
            if (m_nUpperServerSocket == nClientFd)
            {
                ///TODO: 종료한게 UpperServer라는뜻... 서버는 중지되면 안됨. 새로운 처리가 필요...
                PrintInfoLog("Disconnect UpperServer SocketId : " + to_string(m_nUpperServerSocket));
                return;   
            }
            if (DATATRSF_START_MODE::EXTERNAL_MODE == m_nDataTrsfStartMode)
            {
                StUpdateRoutingPath strtUpdateRoutingPathPacket;
                PacketUtil::CreateUpdateRoutingPathPacket(m_nUpperClientFd,nClientFd,&strtUpdateRoutingPathPacket);
                StockPacketToClientSendingQueue(m_nUpperClientFd,reinterpret_cast<char_t*>(&strtUpdateRoutingPathPacket),sizeof(strtUpdateRoutingPathPacket));
            }
            PrintInfoLog("Disconnect Client SocketId : " + to_string(nClientFd));
            RemoveNetObjectInfo(nClientFd);
            return;
        }
        else 
        {
            if (nClientFd == m_nUpperServerSocket) ProcessUpperTcpPacket(szReadBuffer, nReadLength);
            else ProcessTcpPacket(szReadBuffer,nReadLength,nClientFd);
            //읽은 데이터가 있는 상황
        }
    }
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessRecvfromRoutine
// * 설    명: 연결된 네트워크 객체의 udp패킷을 받는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 09
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessRecvfromRoutine(int_t nUdpSocketFd) 
{
    sockaddr_in strtClientAddr;
    socklen_t nClientAddrLen = sizeof(strtClientAddr);
    void* ret_m = memset(&strtClientAddr, 0, nClientAddrLen);
    if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    char_t szReadBuffer[BUFSIZE];

    while(true) 
    {
        ///TODO: recvfrom flag에 대하여 공부
        ssize_t nReadLength = recvfrom(nUdpSocketFd,szReadBuffer,sizeof(szReadBuffer),0,
                                        reinterpret_cast<sockaddr*>(&strtClientAddr), &nClientAddrLen);
        if (nReadLength < 0) 
        {       
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            
            PrintErrorLog("fail to recvfrom()");
            return;
        }
        else
        {
            //읽은 데이터가 있는 상황
            ///TODO: recvfrom은 레벨트리거로 동작시켜서 하나하나가 쓰레드처리하는 것이 낫지 않을까?? 의문
            if (nUdpSocketFd == m_nUpperUdpSocket) ProcessUpperUdpPacket(szReadBuffer, nReadLength, strtClientAddr);
            else ProcessUdpPacket(szReadBuffer, nReadLength, strtClientAddr);
        }
    }
}

// ***************************************************************
// * 함 수 명: StockPacketToClientSendingQueue
// * 설    명: clientTcpSocket을 받아 해당 세션의 전송큐에 전송할 데이터를
// *           추가한다. (실질적인 전송은 EPOLLOUT이벤트가 발생하면 이뤄짐)
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 10
// ****************************************************************
void DataTrsfModel::StockPacketToClientSendingQueue(int32_t nClientFd, const char_t* pSendingData, size_t nSendDataLen)
{
    std::shared_ptr<NetObjectInfo> clClientObjectInfo = nullptr;
    
    {
        ////////////////////////////////////////////////////////////////////////////////////////critical section
        std::lock_guard<mutex> lock(m_NetMapMutex);
        if (m_mNetInfo.find(nClientFd) == m_mNetInfo.end()) return;
        clClientObjectInfo = m_mNetInfo[nClientFd];
    }

    std::lock_guard<std::mutex> lockClientSendQueue(clClientObjectInfo->m_mutexQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    if (clClientObjectInfo->HasPendingData())
        clClientObjectInfo->EnqueueSendQueue(pSendingData,nSendDataLen);
    else{
        clClientObjectInfo->EnqueueSendQueue(pSendingData,nSendDataLen);
        //EPOLLOUT이 꺼져있다면 다시 켜기
        epoll_event strtEpollEvent;
        void* ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));
        if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

        strtEpollEvent.events = EPOLLIN | EPOLLET | EPOLLOUT;
    
        strtEpollEvent.data.fd = nClientFd;
        epoll_ctl(m_nEpollFD, EPOLL_CTL_MOD, nClientFd, &strtEpollEvent);
    }
    
}

// ***************************************************************
// * 함 수 명: ProcessTcpPacket
// * 설    명: tcp패킷을 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 11
// ****************************************************************
// * 수 정 자: KJH
// * 수정일자: 2025. 09. 05
// ****************************************************************

void DataTrsfModel::ProcessTcpPacket(const char_t* pPacketBuf,const ssize_t nPacketSize,int32_t nClientId) 
{
    ssize_t offset = 0;
    while(offset < nPacketSize)
    {
        const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf+offset);
        //client 로그인 요청
        if (pPacketHeader->nType == PACKET_TYPE::REQUEST_LOGIN) 
        {
            
            const StLoginRequestPacketData* pPacketLoginRequest = reinterpret_cast<const StLoginRequestPacketData*>(pPacketBuf+offset);
            
            
            offset+=sizeof(StLoginRequestPacketData);
            PrintDebugLog(pPacketLoginRequest->nUserId);
            PrintDebugLog(pPacketLoginRequest->nUserPw);

            //하위서버라면
            if (m_mNetInfo[nClientId]->m_nProgramType == CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM)
            {
                PrintDebugLog("Request engine start from data communication system");
                string strUserId = pPacketLoginRequest->nUserId;
                string strUserPW = pPacketLoginRequest->nUserPw;
                //디비 조회 검증 절차 등 수행
                int_t ret_a = m_pDatabaseManager->AuthLoginInfo(strUserId,strUserPW);
                
                
                StLoginResponsePacketData strtResponseLoginPacket;
                PacketUtil::CreateResponseLoginPacket(nClientId,ntohl(pPacketLoginRequest->nUserClientFd),
                            USER_AUTHORITY::ADMIN, MILITARY_ID::ADMIN_ID, ret_a, &strtResponseLoginPacket);
                StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtResponseLoginPacket),sizeof(strtResponseLoginPacket));    

            }
            //모의통제도구라면
            else 
            {
                //client program type 저장
                m_mNetInfo[nClientId]->m_nProgramType = ntohl(pPacketHeader->nID);
                //실제 모의통제도구인지 확인 절차 필요하나 여기선 스킵
                PrintDebugLog("Request engine start from engine control client");
                StLoginRequestPacketData strtRequestLoginPacket;
                PacketUtil::CreateRequestLoginPacket(m_nUpperClientFd, nClientId,pPacketLoginRequest,&strtRequestLoginPacket);

                StockPacketToClientSendingQueue(m_nUpperServerSocket,reinterpret_cast<char_t*>(&strtRequestLoginPacket),sizeof(strtRequestLoginPacket));
            }

            //PrintInfoLog("request login clientFd : " + to_string(nClientId));
            //StLoginResponsePacketData strtResponsePacket;
            //PacketUtil::CreateResponseLoginPacket(nClientId, USER_AUTHORITY::ADMIN, MILITARY_ID::ADMIN_ID, &strtResponsePacket);
            //StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));

        }
        //하위 서버 연결 요청
        else if (pPacketHeader->nType == PACKET_TYPE::REQUEST_CONNECT)
        {
            offset+=sizeof(StPacket);
            //1.sysconfig key값과 일치하는지 확인
            /// TODO: 3회 이상 틀렸거나 할 때 조치같은것이 필요하긴 할텐데....skip!
            PrintInfoLog("request UpperServer connect : " + to_string(nClientId));
            auto& pConfig = CONFIG::ConfigManager::GetInstance();
            const string strKeySectionName = DATATRSFER::KEY_SECTION_NAME;
            const string strKeyName        = DATATRSFER::DATATRSFER_KEY;
            const string strConfigPath     = DATATRSFER::CONFIG_PATH;
            const string strKeyNo = pConfig.GetValue(strKeyName,strKeySectionName,strConfigPath);

            if (ntohl(pPacketHeader->nID) != std::stoi(strKeyNo)) 
            {
                PrintInfoLog("Check datatrsferkey : fail " + to_string(nClientId));
                ///TODO: refuse packet을 날려야함.
                return;
            }
            PrintInfoLog("Check datatrsferkey : ok" + to_string(nClientId));
            //하위 서버 넷오브젝트 설정
            m_mNetInfo[nClientId]->m_nProgramType = CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM;

            StPacket strtResponsePacket;
            PacketUtil::CreateResponseConnectPacket(nClientId, &strtResponsePacket);
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //하위서버의 라우팅 패스 정보 업데이트 패킷수신
        else if (pPacketHeader->nType == PACKET_TYPE::UPDATE_ROUTING_INFO)
        {
            const StUpdateRoutingPath* pUpdateRoutingInfoPacket = reinterpret_cast<const StUpdateRoutingPath*>(pPacketBuf+offset);
            offset+=sizeof(StUpdateRoutingPath);
//            RemoveNetRoutingValueInfo(nClientId,ntohl(pUpdateRoutingInfoPacket->nUserClientFd));
        }
        //모의통제도구의 엔진 시작 요청
        else if (pPacketHeader->nType == PACKET_TYPE::REQUEST_SIM_START)
        {
            offset+=sizeof(StStartRequestEnginePacket);
            StStartRequestEnginePacket strtRequestEngineStartPacket;
            PacketUtil::CreateRequestSimulationStartPacket(nClientId,&strtRequestEngineStartPacket);
            //하위서버라면
            if (m_mNetInfo[nClientId]->m_nProgramType == CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM)
            {
                PrintDebugLog("Request engine start from data communication system");
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtRequestEngineStartPacket),sizeof(strtRequestEngineStartPacket));
            }
            //모의통제도구라면
            else 
            {
                //실제 모의통제도구인지 확인 절차 필요하나 여기선 스킵
                PrintDebugLog("Request engine start from engine control client");
                StockPacketToClientSendingQueue(m_nUpperServerSocket,reinterpret_cast<char_t*>(&strtRequestEngineStartPacket),sizeof(strtRequestEngineStartPacket));
            }

        }
        //클라이언트 객체정보 요청
        else if (pPacketHeader->nType == PACKET_TYPE::REQUEST_MILITARY_UNIT_INFO)
        {
            const StRequestLoadMilitaryUnitInfoPacket* pRequestLoadObjPacket = reinterpret_cast<const StRequestLoadMilitaryUnitInfoPacket*>(pPacketBuf+offset);
            offset+=sizeof(StRequestLoadMilitaryUnitInfoPacket);
            StRequestLoadMilitaryUnitInfoPacket strtRequestLoadObjPacket;
            //하위서버라면
            if (m_mNetInfo[nClientId]->m_nProgramType == CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM)
            {
                PrintDebugLog("Request obj info from data communication system");
                PacketUtil::CreateRequestLoadMilitaryUnitPacket(nClientId,ntohl(pRequestLoadObjPacket->nUserClientFd),ntohs(pRequestLoadObjPacket->nUserMilitarySection),&strtRequestLoadObjPacket);
//                RegisterNetRoutingInfo(nClientId,ntohl(pRequestLoadObjPacket->nUserClientFd));
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtRequestLoadObjPacket),sizeof(strtRequestLoadObjPacket));
            }
            //전장상황도라면
            else 
            {
                PacketUtil::CreateRequestLoadMilitaryUnitPacket(m_nUpperClientFd,nClientId,ntohs(pRequestLoadObjPacket->nUserMilitarySection),&strtRequestLoadObjPacket);
                PrintDebugLog("Request obj info from engine control client");
                StockPacketToClientSendingQueue(m_nUpperServerSocket,reinterpret_cast<char_t*>(&strtRequestLoadObjPacket),sizeof(strtRequestLoadObjPacket));
            }
        }
        //Order in Sally 패킷 수신
        else if (pPacketHeader->nType == PACKET_TYPE::REQUEST_ORDER_IN_SALLY)
        {
            const StRequestOrderInSallyPacket* pRequestOrderInSallyPacket = reinterpret_cast<const StRequestOrderInSallyPacket*>(pPacketBuf+offset);
            offset+=sizeof(StRequestOrderInSallyPacket);
            StRequestOrderInSallyPacket strtRequestOrderInSallyPacket;
            //하위서버라면
            if (m_mNetInfo[nClientId]->m_nProgramType == CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM)
            {
                PrintDebugLog("Request order in sally from communication system");
                PacketUtil::CreateRequestOrderInSallyPacket(nClientId,ntohl(pRequestOrderInSallyPacket->nUserClientFd),ntohs(pRequestOrderInSallyPacket->nUserMilitarySection),ntohl(pRequestOrderInSallyPacket->nDstXPos),ntohl(pRequestOrderInSallyPacket->nDstYPos),&strtRequestOrderInSallyPacket);
//                RegisterNetRoutingInfo(nClientId,ntohl(pRequestOrderInSallyPacket->nUserClientFd));
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtRequestOrderInSallyPacket),sizeof(strtRequestOrderInSallyPacket));
            }
            //전장상황도라면
            else 
            {
                PacketUtil::CreateRequestOrderInSallyPacket(m_nUpperClientFd,nClientId,ntohs(pRequestOrderInSallyPacket->nUserMilitarySection),ntohl(pRequestOrderInSallyPacket->nDstXPos),ntohl(pRequestOrderInSallyPacket->nDstYPos),&strtRequestOrderInSallyPacket);
                PrintDebugLog("Request order in sally packet recv from client");
                StockPacketToClientSendingQueue(m_nUpperServerSocket,reinterpret_cast<char_t*>(&strtRequestOrderInSallyPacket),sizeof(strtRequestOrderInSallyPacket));
            }
        }
        //Order in Attack 패킷 수신
        else if (pPacketHeader->nType == PACKET_TYPE::REQUEST_ORDER_IN_ATTACK)
        {
            const StRequestOrderInAttackPacket* pRequestOrderInAttackPacket = reinterpret_cast<const StRequestOrderInAttackPacket*>(pPacketBuf+offset);
            offset+=sizeof(StRequestOrderInAttackPacket);
            StRequestOrderInAttackPacket strtRequestOrderInAttackPacket;
            //하위서버라면
            if (m_mNetInfo[nClientId]->m_nProgramType == CLIENT_TYPE::DATA_COMMUNICATION_SYSTEM)
            {
                PrintDebugLog("Request order in attack from communication system");
                PacketUtil::CreateRequestOrderInAttackPacket(nClientId,ntohl(pRequestOrderInAttackPacket->nUserClientFd),ntohs(pRequestOrderInAttackPacket->nArtilleryId),ntohl(pRequestOrderInAttackPacket->nDstXPos),ntohl(pRequestOrderInAttackPacket->nDstYPos),&strtRequestOrderInAttackPacket);
//                RegisterNetRoutingInfo(nClientId,ntohl(pRequestOrderInAttackPacket->nUserClientFd));
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtRequestOrderInAttackPacket),sizeof(strtRequestOrderInAttackPacket));
            }
            //전장상황도라면
            else 
            {
                PacketUtil::CreateRequestOrderInAttackPacket(m_nUpperClientFd,nClientId,ntohs(pRequestOrderInAttackPacket->nArtilleryId),ntohl(pRequestOrderInAttackPacket->nDstXPos),ntohl(pRequestOrderInAttackPacket->nDstYPos),&strtRequestOrderInAttackPacket);
                PrintDebugLog("Request order in attack packet recv from client");
                PrintDebugLog("art id : "+to_string(ntohs(pRequestOrderInAttackPacket->nArtilleryId)));
                StockPacketToClientSendingQueue(m_nUpperServerSocket,reinterpret_cast<char_t*>(&strtRequestOrderInAttackPacket),sizeof(strtRequestOrderInAttackPacket));
            }
        }
        else 
        {
            ERROR("Do not find Correct Packet Header Info");
            return;
        }
    }
    

}

// ***************************************************************
// * 함 수 명: ProcessUdpPacket
// * 설    명: udp패킷을 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 09
// ****************************************************************
void DataTrsfModel::ProcessUdpPacket(const char_t* pPacketBuf,const ssize_t nPacketSize,const sockaddr_in& strtClientAddr) 
{
    ssize_t offset = 0;
    while(offset<nPacketSize)
    {
        const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf+offset);
        //Upd sockaddr 저장을 위한 Hello udp 과정
        if (pPacketHeader->nType == PACKET_TYPE::HELLO_UDP) 
        {
            offset+=sizeof(StPacketHeader);
            int32_t nClientId = ntohl(pPacketHeader->nID);
            if (m_mNetInfo.find(nClientId) == m_mNetInfo.end()) 
            {
                PrintErrorLog("ProcessUdpPacket() : not found session information");
                return;
            }
            //이미 처리되었다면 무시
            if (m_mNetInfo[nClientId]->m_nConnStatus == CLIENT_STATE::CONNECT) return;

            m_mNetInfo[nClientId]->m_strtUdpAddr = strtClientAddr;
            m_mNetInfo[nClientId]->m_nConnStatus = CLIENT_STATE::CONNECT;//지금 분명 Connect한 상황인데...왜 Response를 안보내
            //send helloUdp sccess tcp packet to client fd!!
            PrintInfoLog("HelloUdp from clientFd : " + to_string(nClientId));
            StPacket strtResponsePacket;
            PacketUtil::CreateResponseHelloUdp(nClientId, &strtResponsePacket);
            StockPacketToClientSendingQueue(nClientId,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));

        }
    }


}

// ***************************************************************
// * 함 수 명: ProcessUpperTcpPacket
// * 설    명: 상위 서버에서 수신 된 tcp패킷을 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 24
// ****************************************************************
void DataTrsfModel::ProcessUpperTcpPacket(const char_t* pPacketBuf,const ssize_t nPacketSize) 
{
    ssize_t offset = 0;
    while(offset < nPacketSize)
    {
        const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf+offset);
        //상위 서버의 연결응답
        if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_CONNECT) 
        {
            offset+=sizeof(StPacket);
            //상위 서버에서 처리되는 client fd를 저장하고 이를 식별자로 상위서버에서 판단한다.
            m_nUpperClientFd = htonl(pPacketHeader->nID);

            PrintInfoLog("response UpperServer ... RESPONSE_CONNECT");
            StPacket strtResponsePacket;
            PacketUtil::CreateRequestHelloUdp(m_nUpperClientFd, &strtResponsePacket);
            /// TODO: 5번 HelloUdp를 보낸다. (임시방편!!!받을때까지 처리하는 게 좋으니...추후에 다시 고치자.)
            for(int i = 0; i < 5; ++i)
            {
                DEBUG("HELLO PACKET SEND TO UPPER SERVER");
                ProcessSendtoRoutineToUpperServer(reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
            }

        }
        //엔진의 hello udp 응답
        else if (pPacketHeader->nType == PACKET_TYPE::HELLO_UDP_RESPONSE)
        {
            offset+=sizeof(StPacket);
            PrintInfoLog("Complete update udp info in UpperServer ... ");

        }
        //상위서버의 로그인 응답
        else if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_LOGIN)
        {
            const StLoginResponsePacketData* pRespnseLoginPacket = reinterpret_cast<const StLoginResponsePacketData*>(pPacketHeader);
            offset+=sizeof(StLoginResponsePacketData);
            PrintDebugLog("Response login packet receive");
            StLoginResponsePacketData strtResponseLoginPacket;
            int32_t nDestination = ntohl(pRespnseLoginPacket->nUserClientFd);

            PacketUtil::CreateResponseLoginToClientPacket(nDestination,pRespnseLoginPacket,&strtResponseLoginPacket);
            StockPacketToClientSendingQueue(nDestination,reinterpret_cast<char_t*>(&strtResponseLoginPacket),sizeof(strtResponseLoginPacket));
        }
        //엔진의 시뮬레이션 시작 응답
        else if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_SIM_START)
        {
            offset+=sizeof(StStartResponseEnginePacket);
            PrintDebugLog("Response engine start simulation!!!");
            StStartResponseEnginePacket strtResponsePacket;
            PacketUtil::CreateResponseSimulationStartPacket(&strtResponsePacket);
            //boardcast로 보내야댐.
            for (auto NetObj : m_mNetInfo)
            {
                int32_t NetID = NetObj.first;
                //자기자신이거나 상위서버면 넘어감.(상위서버에는 보낼 필요 없으니)
                if (NetID == m_nExternalServerSocket || NetID == m_nUpperServerSocket) continue;
                StockPacketToClientSendingQueue(NetID,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
            }
        }
        //엔진의 부대정보 전송
        else if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_MILITARY_UNIT_INFO)
        {
            PrintDebugLog("Response militaryUnitInfo from engine");
            const StResponseLoadMilitaryUnitInfoPacket* pResponseLoadObjPacket = reinterpret_cast<const StResponseLoadMilitaryUnitInfoPacket*>(pPacketBuf+offset);
            offset+=sizeof(StResponseLoadMilitaryUnitInfoPacket);
            PrintDebugLog(to_string(ntohs(pResponseLoadObjPacket->strtMilitaryInfo.nID))+" "+to_string(ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nXpos)));
            StResponseLoadMilitaryUnitInfoPacket strtResponseLoadObjPacket;
            PacketUtil::CreateResponseLoadMilitaryUnitPacket(ntohl(pResponseLoadObjPacket->nUserClientFd), pResponseLoadObjPacket ,&strtResponseLoadObjPacket);
            StockPacketToClientSendingQueue(ntohl(strtResponseLoadObjPacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponseLoadObjPacket),sizeof(strtResponseLoadObjPacket));
        }
        //엔진의 출격명령 응답 전송
        else if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_ORDER_IN_SALLY)
        {
            PrintDebugLog("Response OrderInSally from engine");
            const StResponseOrderInSallyPacket* pResponsePacket = reinterpret_cast<const StResponseOrderInSallyPacket*>(pPacketBuf+offset);
            offset+=sizeof(StResponseOrderInSallyPacket);
            StResponseOrderInSallyPacket strtResponsePacket;
            PacketUtil::CreateResponseOrderInSallyPacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            //DEBUG("artillery id : "+ to_string(ntohs(pResponsePacket->strtArtilleryInfo.nID)));
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 발사명령 응답 전송
        else if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_ORDER_IN_ATTACK)
        {
            PrintDebugLog("Response OrderInAttack from engine");
            const StResponseOrderInAttackPacket* pResponsePacket = reinterpret_cast<const StResponseOrderInAttackPacket*>(pPacketBuf+offset);
            offset+=sizeof(StResponseOrderInAttackPacket);
            StResponseOrderInAttackPacket strtResponsePacket;
            PacketUtil::CreateResponseOrderInAttackPacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            DEBUG("artillery id : "+ to_string(ntohs(pResponsePacket->strtArtilleryInfo.nID)));
            DEBUG("artillery total missile : "+to_string(ntohl(pResponsePacket->strtArtilleryInfo.nTotalMissile)));
            DEBUG("missile id : "+to_string(ntohl(pResponsePacket->strtMissileInfo.nID)));
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 자주포객체 위치 동기화 패킷 전송
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_ARTILLERY_COORDINATE)
        {
        
            const StSyncArtilleryCoordinatePacket* pResponsePacket = reinterpret_cast<const StSyncArtilleryCoordinatePacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncArtilleryCoordinatePacket);
            StSyncArtilleryCoordinatePacket strtResponsePacket;
            PacketUtil::CreateSyncArtilleryCoordinatePacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 미사일객체 위치 동기화 패킷 전송
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_MISSILE_COORDINATE)
        {
            PrintDebugLog("Sync missile arrived!");
            const StSyncMissileCoordinatePacket* pResponsePacket = reinterpret_cast<const StSyncMissileCoordinatePacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncMissileCoordinatePacket);
            StSyncMissileCoordinatePacket strtResponsePacket;
            PacketUtil::CreateSyncMissileCoordinatePacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            //DEBUG("artillery id : "+ to_string(ntohs(pResponsePacket->strtArtilleryInfo.nID)));
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 자주포 피격이벤트 패킷 전송
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_ARTILLERY_ATTACKED)
        {
            const StSyncArtilleryAttackedPacket* pResponsePacket = reinterpret_cast<const StSyncArtilleryAttackedPacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncArtilleryAttackedPacket);
            StSyncArtilleryAttackedPacket strtResponsePacket;
            PacketUtil::CreateSyncArtilleryAttackedPacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 부대 피격이벤트 패킷 전송
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_MILITARY_UNIT_ATTACKED)
        {
            const StSyncMilitaryUnitAttackedPacket* pResponsePacket = reinterpret_cast<const StSyncMilitaryUnitAttackedPacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncMilitaryUnitAttackedPacket);
            StSyncMilitaryUnitAttackedPacket strtResponsePacket;
            PacketUtil::CreateSyncMilitaryUnitAttackedPacket(ntohl(pResponsePacket->nUserClientFd), pResponsePacket ,&strtResponsePacket);
            StockPacketToClientSendingQueue(ntohl(strtResponsePacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
        }
        //엔진의 시간 동기화 패킷 전송
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_SIM_TIME)
        {
            const StSyncSimulationTimePacket* pResponsePacket = reinterpret_cast<const StSyncSimulationTimePacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncSimulationTimePacket);
            //PrintDebugLog("Sync simulation time recv , current time : "+std::to_string(ntohl(pResponsePacket->nCurrentSimulationTime)));
            StSyncSimulationTimePacket strtResponsePacket;
            PacketUtil::CreateSyncSimulationTimePacket(&strtResponsePacket,pResponsePacket);
            //boardcast로 보내야댐.
            for (auto NetObj : m_mNetInfo)
            {
                int32_t NetID = NetObj.first;
                //자기자신이거나 상위서버면 넘어감.(상위서버에는 보낼 필요 없으니)
                if (NetID == m_nExternalServerSocket || NetID == m_nUpperServerSocket) continue;
                StockPacketToClientSendingQueue(NetID,reinterpret_cast<char_t*>(&strtResponsePacket),sizeof(strtResponsePacket));
            }
        }
        else 
        {
            ERROR("Do not find Correct Packet Header Info");
            return;
        }
    }
    

}

// ***************************************************************
// * 함 수 명: ProcessUpperUdpPacket
// * 설    명: 상위 서버에서 수신 된 udp패킷을 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 24
// ****************************************************************
void DataTrsfModel::ProcessUpperUdpPacket(const char_t* pPacketBuf,const ssize_t nPacketSize,const sockaddr_in& strtClientAddr) 
{
    ssize_t offset = 0;
    while(offset < nPacketSize)
    {
        const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf+offset);
    
        if (pPacketHeader->nType == PACKET_TYPE::SYNC_ARTILLERY_COORDINATE)
        {
            //PrintDebugLog("sync artillery coordinate udp....recv");
            const StSyncArtilleryCoordinatePacket* pPacket = reinterpret_cast<const StSyncArtilleryCoordinatePacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncArtilleryCoordinatePacket);
            StSyncArtilleryCoordinatePacket strtPacket;
            PacketUtil::CreateSyncArtilleryCoordinatePacket(ntohl(pPacket->nUserClientFd),pPacket, &strtPacket);
            
            //DEBUG("보내는 클라 fd :" + to_string(ntohl(pPacket->strtHeader.nID)));
            //DEBUG("client fd : " + to_string(ntohl(pPacket->nUserClientFd)));
            ExternalSocketProcessSendtoRoutine(ntohl(strtPacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncArtilleryCoordinatePacket));
        }
        else if (pPacketHeader->nType == PACKET_TYPE::SYNC_MISSILE_COORDINATE)
        {
            PrintDebugLog("sync missile coordinate udp....recv");
            const StSyncMissileCoordinatePacket* pPacket = reinterpret_cast<const StSyncMissileCoordinatePacket*>(pPacketBuf+offset);
            offset+=sizeof(StSyncMissileCoordinatePacket);
            StSyncMissileCoordinatePacket strtPacket;
            PacketUtil::CreateSyncMissileCoordinatePacket(ntohl(pPacket->nUserClientFd),pPacket, &strtPacket);
            
            ExternalSocketProcessSendtoRoutine(ntohl(strtPacket.strtHeader.nID),reinterpret_cast<char_t*>(&strtPacket),sizeof(StSyncMissileCoordinatePacket));
        }
    }
    


}

// ***************************************************************
// * 함 수 명: SetSockOptValue
// * 설    명: ExternalServerSocket의 옵션을 설정한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 04
// ****************************************************************
void DataTrsfModel::SetSockOptValue( const int_t  server_sock)
{
    ///TODO: socket option 나중에 더 보완
	uint32_t nSockBufSize = DATATRSFER::BUFSIZE;
    const socklen_t sockLen = sizeof(nSockBufSize);

    int_t ret_k = setsockopt(server_sock, SOL_SOCKET, SO_SNDBUF, &nSockBufSize, sockLen);
    if( 0 > ret_k ) PrintErrorLog("Fail to setsockopt()");

    int_t ret_k2 = setsockopt(server_sock, SOL_SOCKET, SO_RCVBUF, &nSockBufSize, sockLen);
    if( 0 > ret_k2 ) PrintErrorLog("Fail to setsockopt()");

    const int_t nSocketOpt = 1;     //  기존 bind로 할당된 소켓자원을 프로세스가 재사용
    int_t ret_k3 = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &nSocketOpt, sizeof(nSocketOpt));
    if( 0 > ret_k3 ) PrintErrorLog("Fail to setsockopt()");

}

// ***************************************************************
// * 함 수 명: RegisterNetObjectInfo
// * 설    명: m_mNetInfo 와 epoll에 감시할 fd를 등록하고 소켓을 설정한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::RegisterNetObjectInfo(const std::shared_ptr<NetObjectInfo> clNetObjInfo) 
{
    int32_t nFd = clNetObjInfo->m_nTcpSocket;
    epoll_event strtEpollEvent;
    void* ret_m = memset(&strtEpollEvent, 0, sizeof(strtEpollEvent));

	if( nullptr == ret_m )  PrintErrorLog("Fail to memset()"); 

    strtEpollEvent.events = EPOLLIN | EPOLLET ;
    
    strtEpollEvent.data.fd = nFd;
    std::lock_guard<std::mutex> lock(m_NetMapMutex);
    //////////////////////////////////////////////////////////////////critical section
    if (m_mNetInfo.find(nFd) != m_mNetInfo.end()) 
    {
        //현재 netInfo에 접속하고 있는 세션이라면
        PrintErrorLog("RegisterNetObjectInfo() : already exist session");
    }
    else {
        //socket을 넌블락킹으로 설정
        fcntl(nFd, F_SETFL, fcntl(nFd,F_GETFL,0) | O_NONBLOCK);

        epoll_ctl(m_nEpollFD, EPOLL_CTL_ADD, nFd, &strtEpollEvent);
        m_mNetInfo.insert(std::make_pair(nFd,clNetObjInfo));
        
    }
}

// ***************************************************************
// * 함 수 명: ModifyNetObjectInfo
// * 설    명: Net에 저장된 key 를 받아 
// *           m_mNetInfo를 수정한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 09
// ****************************************************************
void DataTrsfModel::ModifyNetObjectInfo(const int32_t& nNetObjectFd) 
{
    
} 

// ***************************************************************
// * 함 수 명: RemoveNetObjectInfo
// * 설    명: Net에 저장된 fd, key 를 받아 
// *           m_mNetInfo의 정보와 FD를 삭제한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 08
// ****************************************************************
void DataTrsfModel::RemoveNetObjectInfo(const int_t& nNetObjectFd) 
{
    //1. epoll에서 등록 삭제
    //2. NetInfo map에서 삭제
    //3. 소켓 file discriptor 닫기.
    //map의 at에 키가 없다면 예외처리도 해주자...
    //lock 처리도 해줘야 할듯...
    epoll_ctl(m_nEpollFD, EPOLL_CTL_DEL, nNetObjectFd, nullptr);

    //std::lock_guard<std::mutex> lock(m_NetMapMutex);
    if (m_nUpperServerSocket == nNetObjectFd)
    {
        ///TODO: Upper서버가 문제가 있어 종료가 되어버렸을때 어떻게 대처할지... 고민해보자
        return;
    }

    
    std::lock_guard<std::mutex> lock(m_NetMapMutex);
    /////////////////////////////////////////////////////////////////critical section

    m_mNetInfo.erase(nNetObjectFd);
    close(nNetObjectFd);
    
} 

// ***************************************************************
// * 함 수 명: RegisterNetRoutingInfo
// * 설    명: 하위서버fd를 key로 하위서버가 가진 클라이언트 fd를 value
// *           로 저장한다. 
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 14
// ****************************************************************
void DataTrsfModel::RegisterNetRoutingInfo(const int32_t nNetObjectFd, const int32_t nRoutingObjectFd)
{
    std::lock_guard<std::mutex> lock(m_NetMapMutex);
    if(m_mNetInfo.find(nNetObjectFd) != m_mNetInfo.end())
    {
        m_mNetInfo[nNetObjectFd]->InsertRoutingSet(nRoutingObjectFd);
    }

}

// ***************************************************************
// * 함 수 명: RemoveNetRoutingInfo
// * 설    명: 하위 데이터 전달 서버fd의 routing info를 삭제한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 14
// ****************************************************************
void DataTrsfModel::RemoveNetRoutingInfo(const int_t& nNetObjectFd, const int32_t& nRoutingObjectFd) 
{
    std::lock_guard<std::mutex> lock(m_NetMapMutex);
    if(m_mNetInfo.find(nNetObjectFd) != m_mNetInfo.end())
    {
        m_mNetInfo[nNetObjectFd]->RemoveRoutingSet(nRoutingObjectFd);
    }
    
} 

// ***************************************************************
// * 함 수 명: IsThereNetRoutingInfo
// * 설    명: routing path map에 클라이언트 정보가 저장되있는지 리턴해준다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 14
// ****************************************************************
bool_t DataTrsfModel::IsThereNetRoutingInfo(const int_t &nNetObjectFd, const int32_t &nRoutingObjectFd)
{
    std::lock_guard<std::mutex> lock(m_NetMapMutex);
    if(m_mNetInfo.find(nNetObjectFd) != m_mNetInfo.end())
    {
        m_mNetInfo[nNetObjectFd]->HasRoutingInfo(nRoutingObjectFd);
    }
}

// ***************************************************************
// * 함 수 명: PushFunThreadQueue
// * 설    명: ThreadPool에 함수를 넣는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::PushFunThreadQueue(const std::function<void()> task)
{
    ThreadPool::GetInstance().Enqueue(task);
}

// ***************************************************************
// * 함 수 명: PrintInfoLog
// * 설    명: Info log를 출력한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::PrintInfoLog(const string& info_log) const 
{
    INFO(info_log);
}

// ***************************************************************
// * 함 수 명: PrintErrorLog
// * 설    명: Error log를 출력한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::PrintErrorLog(const string& error_log) const 
{
    ERROR(error_log);
}

// ***************************************************************
// * 함 수 명: PrintDebugLog
// * 설    명: Debug log를 출력한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::PrintDebugLog(const string& debug_log) const 
{
    DEBUG(debug_log);
}

// ***************************************************************
// * 함 수 명: Start
// * 설    명: dataTrsferModel을 시작한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 01
// ***************************************************************
void DataTrsfModel::Start(const int32_t stnStartMode) 
{
    PrintDebugLog("DataTrsfModel::Start()");

    Initialize(stnStartMode);

    if (DATATRSF_START_MODE::INTERNAL_MODE == stnStartMode)
    {
        InitInternalProcess();
        /////////////////////////////////////db test
        // SELECT 실행
        //auto results = m_pDatabaseManager->Query("SELECT id, pw FROM user");
        //for (auto& row : results) 
        //{
        //    std::cout << "User: " << row[0] << ", PW: " << row[1] << std::endl;
        //}
        //////////////////////////////////////
    }
    else if(DATATRSF_START_MODE::EXTERNAL_MODE == stnStartMode) 
    {
        ConnectUpperServer();
    }
    else
    {
        PrintErrorLog("START PARAM RANGE : 0 ~ ");
        return;
    }
    /////////////////////////////////////////////////////////Internal process
    if (DATATRSF_START_MODE::INTERNAL_MODE == stnStartMode)
    {
        m_thrInternalRecvThread = std::thread([this](){
            this->InternalProcessStart();
        });
    }
    
    /////////////////////////////////////////////////////////External process
    while(true) 
    {
        epoll_event eEvents[DATATRSFER::MAX_EVENTS];
        int_t nEventCount = epoll_wait(m_nEpollFD, eEvents, DATATRSFER::MAX_EVENTS, -1);
        //TODO: epoll 옵션에 대하여 더 공부하자

        for (int_t i=0; i<nEventCount; ++i) 
        {
            int_t nCurrentFd = eEvents[i].data.fd;
            //클라이언트 접속 시도
            if (m_nExternalServerSocket == nCurrentFd) 
            {
                PushFunThreadQueue([this]() {
                    this->ExternalSocketProcessAcceptRoutine();
                });
                continue;
            }
            //클라이언트 udp 송신
            if (m_nExternalUdpSocket == nCurrentFd)
            {
                PushFunThreadQueue([this, nCurrentFd]() {
                    this->ExternalSocketProcessRecvfromRoutine(nCurrentFd);
                });
                continue;
            }
            //UpperServer udp 송신
            if (m_nUpperUdpSocket == nCurrentFd)
            {
                PushFunThreadQueue([this, nCurrentFd]() {
                    this->ExternalSocketProcessRecvfromRoutine(nCurrentFd);
                });
                continue;
            }

            //클라이언트/UpperServer tcp 송수신...
            //한 fd에 동시에 발생할 수 있으므로 continue X
            //읽기 가능 이벤트라면
            if (eEvents[i].events & EPOLLIN)
            {
                PushFunThreadQueue([this, nCurrentFd]() {
                this->ExternalSocketProcessRecvRoutine(nCurrentFd);
                });
            }
            //쓰기 가능 이벤트라면
            if (eEvents[i].events & EPOLLOUT)
            {
                PushFunThreadQueue([this,nCurrentFd](){
                    this->ExternalSocketProcessSendRoutine(nCurrentFd);
                });
            }
            if (eEvents[i].events & (EPOLLERR | EPOLLHUP)) 
            {
                //client와 연결이 끊김
                //TODO: 클라이언트 세션 종료처리
                PrintErrorLog("Unexpected Disconnecting!! client : " + to_string(nCurrentFd));
                RemoveNetObjectInfo(nCurrentFd);
                

            }
        
        }
    }
}

// ***************************************************************
// * 함 수 명: InternalProcessStart
// * 설    명: Inter process commuication을 시작한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 15
// ***************************************************************
void DataTrsfModel::InternalProcessStart() 
{
    while(!m_bIsExit.load()) 
    {
        InternalProcessReceiveRoutine();
    }
}
// ***************************************************************
// * 함 수 명: Finish
// * 설    명: dataTrsferModel을 끝낸다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 02
// ***************************************************************
void DataTrsfModel::Finish() 
{

    PrintDebugLog("DataTrsfModel::Finish()");
    if (m_nDataTrsfStartMode == DATATRSF_START_MODE::INTERNAL_MODE)
    {
        m_bIsExit.store(true);
        m_pShmManager->WakeUpSemWaitBlocking();
        m_thrInternalRecvThread.join();
    }
    
    //연결된 모든 클라이언트객체 닫기
    for (auto& nConnInfo : m_mNetInfo) 
    {
        int_t nConnFd = nConnInfo.first;
        if (m_nUpperServerSocket == nConnFd) continue;

        close(nConnFd);

        PrintInfoLog("close Connect Object , FD : " + to_string(nConnFd) + " : ok");
    }
    if (m_nDataTrsfStartMode == DATATRSF_START_MODE::EXTERNAL_MODE)
    {
        close(m_nUpperServerSocket);
        close(m_nUpperUdpSocket);
        PrintInfoLog("close UpperServerTcpSocket : ok");
        PrintInfoLog("close UpperServerUdpSocket : ok");
    }
    close(m_nExternalServerSocket);
    close(m_nExternalUdpSocket);
    close(m_nEpollFD);
    PrintInfoLog("close ExternalServerSocket : ok");
    PrintInfoLog("close ExternalUdpSocket    : ok");
    PrintInfoLog("close EpollFileDiscripter  : ok");

    m_pLogManager.reset();
    m_pShmManager.reset();
    m_pDatabaseManager.reset();
    PrintInfoLog("close LogManager : ok");
    PrintInfoLog("close ShmManager : ok");
    PrintInfoLog("close DBManager  : ok");
}