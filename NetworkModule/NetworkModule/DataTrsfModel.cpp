// ********************************************************************
// * 소 스 명: DataTrsfModel.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 20
// ********************************************************************
#include "pch.h"

#include <cstring>
#include <errno.h>

#include "DataTrsfModel.h"
#include "PacketUtil.h"
#include "ThreadPool.h"

using std::string;
using std::to_string;
using namespace DATATRSFER;
// ***************************************************************
// * 함 수 명: Initialize
// * 설    명: DataTrsfModel을 초기화한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 20
// ****************************************************************
void DataTrsfModel::Initialize(const string strIpAddress, const string strPortNo, const string strUdpPortNo)
{
    //ThreadPool 초기화
    ThreadPool::GetInstance();
    //실행전에 맴버변수 초기화
    m_bIsExit.store(false);

    m_strIpAddress = strIpAddress;
    m_strPortNo = strPortNo;
    m_strUdpPortNo = strUdpPortNo;

    m_nExternalServerSocket = INVALID_SOCKET;
    m_nExternalUdpSocket = INVALID_SOCKET;
    m_nInternalLoopbackSocket = INVALID_SOCKET;

    //select setting
    FD_ZERO(&m_strtReadSet);

    if (::WSAStartup(MAKEWORD(2, 2), &m_WinSockInfo) != 0)
    {
        PrintErrorLog("WSAStartup() error! ");
        return;
    }
}

// ***************************************************************
// * 함 수 명: InitializeLoopback
// * 설    명: Select의 안전한 종료를 위한 Loopback 소켓 설정
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 27
// ***************************************************************
bool_t DataTrsfModel::InitializeLoopback()
{
 
    // tcp socket 초기화
    m_nInternalLoopbackPipeS = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_nInternalLoopbackPipeS == INVALID_SOCKET)
    {
        PrintErrorLog("Loopback pipe (s) connection is fail");
        return false;
    }

    struct sockaddr_in strtPipeSAddr;

    void* ret_m = memset(&strtPipeSAddr, 0, sizeof(strtPipeSAddr));
    if (nullptr == ret_m)  PrintErrorLog("Fail to memset()");

    strtPipeSAddr.sin_family = AF_INET;
    if (InetPtonA(AF_INET, "127.0.0.1", &strtPipeSAddr.sin_addr) <= 0) {
        // 변환 실패 처리
        PrintErrorLog("Invalid IP Address Loopback");
        return false;
    }
    strtPipeSAddr.sin_port = 0;

    int_t ret_b = ::bind(m_nInternalLoopbackPipeS, reinterpret_cast<sockaddr*>(&strtPipeSAddr), sizeof(strtPipeSAddr));
    if (ret_b < 0)
    {
        PrintErrorLog("::bind() fail");
        return false;
    }

    int_t ret_l = ::listen(m_nInternalLoopbackPipeS,1);
    if (ret_l < 0)
    {
        PrintErrorLog("::listen() fail");
        return false;
    }

    int_t nSockaddrLen = sizeof(strtPipeSAddr);
    if (getsockname(m_nInternalLoopbackPipeS, reinterpret_cast<sockaddr*>(&strtPipeSAddr), &nSockaddrLen) == SOCKET_ERROR) {
        PrintErrorLog("::getsockname() fail");
        return false;
    }

    m_nInternalLoopbackPipeC = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_nInternalLoopbackPipeC == INVALID_SOCKET)
    {
        PrintErrorLog("Loopback pipe (c) connection is fail");
        return false;
    }
    //socket non blocking setting
    u_long on = 1;
    ioctlsocket(m_nInternalLoopbackPipeC, FIONBIO, &on);

    int_t ret_c = ::connect(m_nInternalLoopbackPipeC, reinterpret_cast<sockaddr*>(&strtPipeSAddr), sizeof(strtPipeSAddr));
    if (ret_c < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        PrintErrorLog("::connect() fail");
        return false;
    }

    m_nInternalLoopbackSocket = ::accept(m_nInternalLoopbackPipeS, nullptr, nullptr);
    if (m_nInternalLoopbackSocket == INVALID_SOCKET)
    {
        PrintErrorLog("Loopback socket accept is fail");
        return false;
    }
    return true;
}

// ***************************************************************
// * 함 수 명: ConnectServer
// * 설    명: Server와 연결을 맺는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 20
// ***************************************************************
bool_t DataTrsfModel::ConnectServer()
{
    PrintDebugLog(m_strIpAddress);
    PrintDebugLog(m_strPortNo);
    PrintDebugLog(m_strUdpPortNo);

    // tcp socket 초기화
    m_nExternalServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nExternalServerSocket == INVALID_SOCKET)
    {
        PrintErrorLog("server TCP connection is fail");
        return false;
    }

    struct sockaddr_in strtServerAddr;

    void* ret_m = memset(&strtServerAddr, 0, sizeof(strtServerAddr));
    if (nullptr == ret_m)  PrintErrorLog("Fail to memset()");

    strtServerAddr.sin_family = AF_INET;
    if (InetPtonA(AF_INET, m_strIpAddress.c_str(), &strtServerAddr.sin_addr) <= 0) {
        // 변환 실패 처리
        PrintErrorLog("Invalid IP Address format: " + m_strIpAddress);
        return false;
    }
    strtServerAddr.sin_port = htons(stoi(m_strPortNo, nullptr));
    //TODO: client에서는 소켓을 어떤 옵션을 넣어야 할지 공부
    SetSockOptValue(m_nExternalServerSocket,true);

    m_nExternalUdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_nExternalUdpSocket == INVALID_SOCKET)
    {
        PrintErrorLog("server UDP connection is fail");
        return false;
    }

    struct sockaddr_in strtUdpAddr;

    ret_m = memset(&strtUdpAddr, 0, sizeof(strtUdpAddr));
    if (nullptr == ret_m)  PrintErrorLog("Fail to memset()");

    strtUdpAddr.sin_family = AF_INET;
    if (InetPtonA(AF_INET, m_strIpAddress.c_str(), &strtUdpAddr.sin_addr) <= 0) {
        // 변환 실패 처리
        PrintErrorLog("Invalid IP Address format: " + m_strIpAddress);
        return false;
    }
    strtUdpAddr.sin_port = htons(std::stoi(m_strUdpPortNo, nullptr));


    m_strtServerUdpAddr = strtUdpAddr;
    //TODO: UpperServer소켓도 옵션셋팅해야 하는걸까...?
    SetSockOptValue(m_nExternalUdpSocket,false);

    int_t ret_c = ::connect(m_nExternalServerSocket, reinterpret_cast<sockaddr*>(&strtServerAddr), sizeof(strtServerAddr));
    if (ret_c < 0)
    {
        PrintErrorLog("::connect() fail");
        return false;
    }




    //socket non blocking setting
    u_long on = 1;
    ioctlsocket(m_nExternalServerSocket, FIONBIO, &on);
    //socket non blocking setting
    u_long on2 = 1;
    ioctlsocket(m_nExternalUdpSocket, FIONBIO, &on2);
    return true;
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessSendRoutine
// * 설    명: 연결된 서버에 패킷을 보낸다. 소켓 write이벤트 발생 시
// *          실질적으로 전송큐에 저장되어있는 것을 순차적으로 보내는 역할을 한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessSendRoutine()
{

    std::lock_guard<std::mutex> lockServerSendQueue(m_mutexQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    while (HasPendingData())
    {
        std::vector<char_t>& vecSendBuffer = PeekFrontQueue();
        const char_t* pSendDataPtr = vecSendBuffer.data() + m_nSentBytes;
        size_t nSendDataLen = vecSendBuffer.size() - m_nSentBytes;

        //TODO: send의 flag 찾아보기
        size_t byte_v = send(m_nExternalServerSocket, pSendDataPtr, nSendDataLen, 0);
        if (byte_v > 0)
        {
            m_nSentBytes += byte_v;
            if (m_nSentBytes == vecSendBuffer.size())
            {
                PopSendQueue();
                m_nSentBytes = 0;
            }
        }
        else
        {
            //전송버퍼가 다찼거나 보낼데이터 없음... 다음 out이벤트를 기다리자
            int_t err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) break;

            PrintErrorLog("send() err");
            return;
        }
    }
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessSendtoRoutine
// * 설    명: 연결된 서버에 udp패킷을 보낸다. 
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessSendtoRoutine(const char_t* pSendingData, size_t nSendDataLen)
{

    //TODO : sendto option check
    size_t byte_v = sendto(m_nExternalUdpSocket, pSendingData, nSendDataLen, 0, reinterpret_cast<sockaddr*>(&m_strtServerUdpAddr), sizeof(m_strtServerUdpAddr));
    if (byte_v < 0)
    {
        int_t err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
            PrintDebugLog("sendto buffer is full....");
            // TODO:송신 버퍼가 꽉 참. 재시도 로직 큐에 등록하거나 다른 처리가 필요
        }
        else
        {
            PrintErrorLog("sendto() fail");
        }
    }
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessRecvRoutine
// * 설    명: 연결된 서버의 패킷을 받는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessRecvRoutine()
{
    char_t szReadBuffer[BUFSIZE];
    while (true)
    {
        //TODO:recv flag에 대하여 공부
        int_t nReadLength = recv(m_nExternalServerSocket, szReadBuffer, sizeof(szReadBuffer), 0);
        if (nReadLength < 0)
        {
            int_t err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) break;
       
            PrintInfoLog("Unexpected Disconnect Server " + to_string(nReadLength));
            return;
        }
        else if (nReadLength == 0)  //server가 연결을 종료함 
        {
                //TODO : 종료한게 UpperServer라는뜻... 서버는 중지되면 안됨. 새로운 처리가 필요...
            PrintInfoLog("Disconnect from Server ");
            return;
        }
        else
        {
            ProcessTcpPacket(szReadBuffer, nReadLength);
            //읽은 데이터가 있는 상황
        }
    }
}

// ***************************************************************
// * 함 수 명: ExternalSocketProcessRecvfromRoutine
// * 설    명: 연결된 서버의 udp패킷을 받는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ExternalSocketProcessRecvfromRoutine()
{
    sockaddr_in strtClientAddr;
    int_t nClientAddrLen = sizeof(strtClientAddr);
    void* ret_m = memset(&strtClientAddr, 0, nClientAddrLen);
    if (nullptr == ret_m)  PrintErrorLog("Fail to memset()");

    char_t szReadBuffer[BUFSIZE];

    while (true)
    {
        //TODO:recvfrom flag에 대하여 공부
        int_t nReadLength = recvfrom(m_nExternalUdpSocket, szReadBuffer, sizeof(szReadBuffer), 0,
            reinterpret_cast<sockaddr*>(&strtClientAddr), &nClientAddrLen);
        if (nReadLength < 0)
        {
            int_t err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) break;

            PrintErrorLog("fail to recvfrom()");
            return;
        }
        else
        {
            //읽은 데이터가 있는 상황
            //strtClinetAddr는 서버의 그것과 동일할 것이므로 검증용아님 안받아도
            //무방하나 일단 받아놓기.
            ProcessUdpPacket(szReadBuffer, nReadLength, strtClientAddr);
        }
    }
}

// ***************************************************************
// * 함 수 명: StockPacketToServerSendingQueue
// * 설    명: 서버소켓의 전송큐에 전송할 데이터를
// *           추가한다. (실질적인 전송은 select쓰기 이벤트가 발생하면 이뤄짐)
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::StockPacketToServerSendingQueue(const char_t* pSendingData, size_t nSendDataLen)
{

    std::lock_guard<std::mutex> lockServerSendQueue(m_mutexQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    EnqueueSendQueue(pSendingData, nSendDataLen);

}

// ***************************************************************
// * 함 수 명: ProcessTcpPacket
// * 설    명: tcp패킷을 처리한다. 유니티에서 처리될 것은 결과큐에 넣는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ProcessTcpPacket(const char_t* pPacketBuf, const int_t nPacketSize)
{
    const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf);
    //client 로그인 요청
    if (pPacketHeader->nType == PACKET_TYPE::RESPONSE_LOGIN)
    {
        //서버에서 처리되는 client fd를 저장하고 이를 식별자로 서버에서 판단한다.
        m_nIdentifcationFd = ntohl(pPacketHeader->nID);
        const StLoginResponsePacketData* pPacket = reinterpret_cast<const StLoginResponsePacketData*>(pPacketBuf);
        
        //만약 로그인 실패라면 거절 패킷을 보낸다.
        if (pPacket->strtHeader.nFlagField != NETWORK_FLAG::LOGIN_SUCCESS)
        {
            StPacket strtLoginFailPacket;
            strtLoginFailPacket.strtHeader.nType = PACKET_TYPE::HELLO_UDP_RESPONSE;
            strtLoginFailPacket.strtHeader.nFlagField = pPacket->strtHeader.nFlagField;

            PushPacketToRecvQueue(reinterpret_cast<const char_t*>(&strtLoginFailPacket), sizeof(StPacket));
            return;
        }
        
        m_nUserMilitarySection = ntohs(pPacket->nUserMilitarySection);
        
        
        PrintInfoLog("response UpperServer ... RESPONSE_CONNECT!!" + to_string(ntohs(pPacket->nUserMilitarySection)));
        
        
        StPacket strtResponsePacket;
        PacketUtil::CreateRequestHelloUdp(m_nIdentifcationFd, &strtResponsePacket);
        //TODO : 1번 HelloUdp를 보낸다. (임시방편!!!받을때까지 처리하는 게 좋으니...추후에 다시 고치자.)
        for (int i = 0; i < 5; ++i)
        {
            DEBUG("HELLO PACKET SEND TO UPPER SERVER");
            ExternalSocketProcessSendtoRoutine(reinterpret_cast<char_t*>(&strtResponsePacket), sizeof(strtResponsePacket));
        }

    }
    else
    {
        PushPacketToRecvQueue(pPacketBuf, nPacketSize);
    }

}

// ***************************************************************
// * 함 수 명: ProcessUdpPacket
// * 설    명: udp패킷을 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 09
// ****************************************************************
// * 수 정 자: KJH
// * 수정일자: 2025. 08. 21
// ****************************************************************
void DataTrsfModel::ProcessUdpPacket(const char_t* pPacketBuf, const int_t nPacketSize, const sockaddr_in& strtClientAddr)
{
    const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pPacketBuf);

    PushPacketToRecvQueue(pPacketBuf, nPacketSize);
}

// ***************************************************************
// * 함 수 명: SetSockOptValue
// * 설    명: ExternalServerSocket의 옵션을 설정한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 04
// ****************************************************************
void DataTrsfModel::SetSockOptValue(const SOCKET  server_sock, const bool_t isTcp)
{
    //TODO: socket option 나중에 더 보완
    uint32_t nSockBufSize = DATATRSFER::BUFSIZE;
    
    const int_t nSocketOpt = 1;

    if (isTcp) 
    {
        //Nagle 알고리즘 끄기.
        int_t ret_k0 = setsockopt(server_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&nSocketOpt), sizeof(nSocketOpt));
        if (0 > ret_k0) PrintErrorLog("Fail to setsockopt()");
    }

    int_t ret_k1 = setsockopt(server_sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&nSockBufSize), sizeof(nSockBufSize));
    if (0 > ret_k1) PrintErrorLog("Fail to setsockopt()");

    int_t ret_k2 = setsockopt(server_sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&nSockBufSize), sizeof(nSockBufSize));
    if (0 > ret_k2) PrintErrorLog("Fail to setsockopt()");

    //socket non blocking setting
    //u_long on = 1;
    //ioctlsocket(server_sock, FIONBIO, &on);
}

// ***************************************************************
// * 함 수 명: ProcessSelectLoop
// * 설    명: select fd set에 소켓들을 등록하고 패킷을 받아 처리한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 06
// ****************************************************************
void DataTrsfModel::ProcessSelectLoop()
{
    while (!m_bIsExit.load())
    {
        FD_ZERO(&m_strtReadSet);

        //서버 tcp, udp socket 등록
        FD_SET(m_nExternalServerSocket, &m_strtReadSet);
        FD_SET(m_nExternalUdpSocket, &m_strtReadSet);
        FD_SET(m_nInternalLoopbackSocket, &m_strtReadSet);

        SOCKET maxFd = (m_nExternalServerSocket > m_nExternalUdpSocket) ? m_nExternalServerSocket : m_nExternalUdpSocket;
        maxFd = (m_nInternalLoopbackSocket > maxFd) ? m_nInternalLoopbackSocket : maxFd;

        //event발생시까지 blocking모드..
        int ret_s = select(int(maxFd + 1), &m_strtReadSet, nullptr, nullptr, nullptr);
        if (ret_s == SOCKET_ERROR)
        {
            PrintErrorLog("select() fail");
            m_bIsExit.store(true);
            return;
        }

        // TCP packet recv
        if (FD_ISSET(m_nExternalServerSocket, &m_strtReadSet))
        {
            PushFunThreadQueue([this]() {
                this->ExternalSocketProcessRecvRoutine();
            });
        }

        // UDP packet recv
        if (FD_ISSET(m_nExternalUdpSocket, &m_strtReadSet))
        {
            PushFunThreadQueue([this]() {
                this->ExternalSocketProcessRecvfromRoutine();
            });
        }

        //exit signal recv
        if (FD_ISSET(m_nInternalLoopbackSocket, &m_strtReadSet)) {
            PrintDebugLog("Network Module exit signal recv");
            break;
        }
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
    USER_ERROR(error_log);
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
// * 작성일자: 2025. 08. 21
// ***************************************************************
bool_t DataTrsfModel::Start(const string strIpAddress, const string strPortNo, const string strUdpPortNo)
{
    PrintDebugLog("DataTrsfModel::Start()");

    //DataTrsfModel 초기화
    Initialize(strIpAddress,strPortNo,strUdpPortNo);
    bool_t ret_i = InitializeLoopback();
    if (ret_i == false)
    {
        m_bIsExit.store(true);
        return false;
    }
    
    /////////////////////////////////////////////////////////Server connect process
    ret_i = ConnectServer();
    if (ret_i == false) 
    {
        m_bIsExit.store(true);
        return false;
    }

    /////////////////////////////////////////////////////////select event rcv process
    m_thrExternalRecvThread = std::thread([this]() {
        this->ProcessSelectLoop();
    });


    //////////////////////////////////////////////////////////packet send process
    m_thrExternalSendThread = std::thread([this]() {
        while (!m_bIsExit.load())
        {
            if (HasPendingData())
            {
                this->ExternalSocketProcessSendRoutine();
            }
        }
    });

    return true;
}

// ***************************************************************
// * 함 수 명: Finish
// * 설    명: dataTrsferModel을 끝낸다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ***************************************************************
void DataTrsfModel::Finish()
{

    PrintDebugLog("DataTrsfModel::Finish()");
    if (!m_bIsExit.load())
    {
        //select 깨우기 위해 exit 시그널을 보낸다.
        send(m_nInternalLoopbackPipeC, "x", 1, 0);
        m_bIsExit.store(true);
        m_thrExternalRecvThread.join();
        m_thrExternalSendThread.join();
    }

    //server socket close
    if (m_nExternalServerSocket != INVALID_SOCKET)
    {
        closesocket(m_nExternalServerSocket);
    }
    if (m_nExternalUdpSocket != INVALID_SOCKET) 
    {
        closesocket(m_nExternalUdpSocket);
    }

    //loopback socket close
    if (m_nInternalLoopbackPipeS != INVALID_SOCKET)
    {
        closesocket(m_nInternalLoopbackPipeS);
    }
    if (m_nInternalLoopbackPipeC != INVALID_SOCKET)
    {
        closesocket(m_nInternalLoopbackPipeC);
    }
    if (m_nInternalLoopbackSocket != INVALID_SOCKET)
    {
        closesocket(m_nInternalLoopbackSocket);
    }
    WSACleanup();
    PrintInfoLog("close ExternalServerSocket : ok");
    PrintInfoLog("close ExternalUdpSocket : ok");
    PrintInfoLog("close EpollFileDiscripter : ok");

}
// ***************************************************************
// * 함 수 명: PushPacketToRecvQueue
// * 설    명: 수신한 패킷을 수신 큐로 넣는다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ***************************************************************
void DataTrsfModel::PushPacketToRecvQueue(const char_t* cSendData, size_t nDataLen)
{
    std::lock_guard<std::mutex> lockRecvQueue(m_mutexRecvQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    m_deqRecvQueue.emplace_back(cSendData, cSendData + nDataLen);
}

// ***************************************************************
// * 함 수 명: PopPacketFromRecvQueue
// * 설    명: 수신한 패킷을 수신 큐에서 꺼낸다. 클라이언트 측에서 사용한다.
// *           반환값으로 총 읽어들인 바이트 수를 반환한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ***************************************************************
int_t DataTrsfModel::PopPacketFromRecvQueue(char_t* czOutBuffer, int_t nBufSize)
{
    std::lock_guard<std::mutex> lockRecvQueue(m_mutexRecvQueue);
    ////////////////////////////////////////////////////////////////////////////////////////critical section
    int_t nTotalReadByte = 0;
    while (!m_deqRecvQueue.empty()) {

        vector<char_t> czRecvPacket = m_deqRecvQueue.front();

        size_t copySize = czRecvPacket.size();
        if (nTotalReadByte + copySize > nBufSize) break;
        
        m_deqRecvQueue.pop_front();

        memcpy(czOutBuffer+nTotalReadByte, czRecvPacket.data(), copySize);
        nTotalReadByte += copySize;
    }
    return nTotalReadByte;
}
// ***************************************************************
// * 함 수 명: PushMessageToServerProcess
// * 설    명: 라이브러리에 패킷전송을 요청한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 09. 24
// ***************************************************************
void DataTrsfModel::PushLoginMessageToServerProcess(const int8_t nPacketType, const char_t* strUserId, const char_t* strUserPW) 
{
    //로그인 요청 패킷 전달
    if (nPacketType == PACKET_TYPE::REQUEST_LOGIN)
    {
        StLoginRequestPacketData strtPacket;
        PacketUtil::CreateRequestLoginPacket(strUserId, strUserPW,&strtPacket);

        StockPacketToServerSendingQueue(reinterpret_cast<char_t*>(&strtPacket), sizeof(strtPacket));
    }
}
// ***************************************************************
// * 함 수 명: PushMessageToServerProcess
// * 설    명: 라이브러리에 패킷전송을 요청한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ***************************************************************
void DataTrsfModel::PushMessageToServerProcess(const int8_t nPacketType, const int32_t xpos, const int32_t ypos,const int16_t nRequestObjId)
{
    //시뮬레이션 시작 패킷 전달
    if (nPacketType == PACKET_TYPE::REQUEST_SIM_START)
    {
        StStartRequestEnginePacket strtPacket;
        PacketUtil::CreateRequestSimulationStartPacket(m_nIdentifcationFd, &strtPacket);

        StockPacketToServerSendingQueue(reinterpret_cast<char_t*>(&strtPacket), sizeof(strtPacket));
    }
    //부대 정보 요청 패킷 전달
    else if (nPacketType == PACKET_TYPE::REQUEST_MILITARY_UNIT_INFO)
    {
        StRequestLoadMilitaryUnitInfoPacket strtPacket;
        PacketUtil::CreateRequestLoadMilitaryUnitPacket(m_nIdentifcationFd,m_nIdentifcationFd,m_nUserMilitarySection, &strtPacket);

        StockPacketToServerSendingQueue(reinterpret_cast<char_t*>(&strtPacket), sizeof(strtPacket));
    }
    //출격 명령 패킷 전달
    else if (nPacketType == PACKET_TYPE::REQUEST_ORDER_IN_SALLY)
    {
        StRequestOrderInSallyPacket strtPacket;
        PacketUtil::CreateRequestOrderInSallyPacket(m_nIdentifcationFd, m_nIdentifcationFd, nRequestObjId, xpos,ypos,&strtPacket);

        StockPacketToServerSendingQueue(reinterpret_cast<char_t*>(&strtPacket), sizeof(strtPacket));
    }
    //공격 명령 패킷 전달
    else if (nPacketType == PACKET_TYPE::REQUEST_ORDER_IN_ATTACK)
    {
        StRequestOrderInAttackPacket strtPacket;
        PacketUtil::CreateRequestOrderInAttackPacket(m_nIdentifcationFd, m_nIdentifcationFd, nRequestObjId, xpos, ypos, &strtPacket);
        DEBUG("artillery nRequestObjId : " + std::to_string(nRequestObjId));
        StockPacketToServerSendingQueue(reinterpret_cast<char_t*>(&strtPacket), sizeof(strtPacket));
    }
    else
    {
        PrintErrorLog("Do not find Correct Packet Type");
        return;
    }
}
// ***************************************************************
// * 함 수 명: PushMessageToServer
// * 설    명: 라이브러리에 패킷 전송을 요청한다. 클라이언트 측에서 사용한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 08. 21
// ***************************************************************
void DataTrsfModel::PushMessageToServer(const int8_t nPacketType,const int32_t xpos, const int32_t ypos,const int16_t nId)
{
    PushFunThreadQueue([this, nPacketType,xpos,ypos,nId]() {
        this->PushMessageToServerProcess(nPacketType,xpos,ypos,nId);
    });
}

// ***************************************************************
// * 함 수 명: PushLoginMessageToServer
// * 설    명: 라이브러리에 login패킷 전송을 요청한다. 클라이언트 측에서 사용한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 09. 24
// ***************************************************************
void DataTrsfModel::PushLoginMessageToServer(const int8_t nPacketType, const char_t* strUserId, const char_t* strUserPW)
{
    PushFunThreadQueue([this, nPacketType, strUserId, strUserPW]() {
        this->PushLoginMessageToServerProcess(nPacketType, strUserId,strUserPW);
    });
}


