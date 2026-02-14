// ********************************************************************
// * 소스정의: SimEngineDataTrsfModel.cpp
// * 설    명: engine 파트에서 데이터 전달처리 모델을 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
#include "SimEngineDataTrsfModel.h"
#include "MilitaryUnitModel.h"
#include "DEVSim.h"
#include "Global.h"

#include <cstring>
#include <netinet/in.h>



namespace DATATRSFER{

// ********************************************************************
// * 함 수 명: Initialize
// * 설    명: SimEngineDataTrsfModel을 초기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
void SimEngineDataTrsfModel::Initialize()
{
    m_bIsExit.store(false);
    //ipc초기화
    m_pShmManager = std::make_unique<SharedMemControl>();
    m_pShmManager->InitializeShm();

    m_pSimulator = std::make_unique<DEVSim>();
    m_pMainModel = std::make_shared<MainCoupledModel>("Main");
    m_pMilitaryModel = m_pMainModel->GetMilitaryModel();

    //Output message를 외부로 보낼 콜백함수를 등록한다.
    m_pSimulator->SetOutputCallback([this](const Message& OutputMessage){
        //DEBUG("external callback function call!!");
        this->OutputSendMessageParse(OutputMessage);
    });

}

// ********************************************************************
// * 함 수 명: InternalProcessStart
// * 설    명: inter process commuication을 시작한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
void SimEngineDataTrsfModel::InternalProcessStart() 
{
    while(!m_bIsExit.load()) 
    {
        InternalProcessReceiveRoutine();
    }
}

// ***************************************************************
// * 함 수 명: InternalProcessSendRoutine
// * 설    명: 공유메모리에 메세지를 보낸다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 31
// ***************************************************************
void SimEngineDataTrsfModel::InternalProcessSendRoutine(const char_t* pSendingData, size_t nSendDataLen)
{
    const StPacketHeader* pPacketHeader = reinterpret_cast<const StPacketHeader*>(pSendingData);
    //TODO: 나중에 필요하다면 헤더파싱후 다른 로직을 짠다.
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
// * 설    명: 공유메모리로 부터 메세지를 수신한다.
// * 작 성 자: KJH
// * 작성일자: 2025. 07. 31
// ***************************************************************
void SimEngineDataTrsfModel::InternalProcessReceiveRoutine()
{
    char_t czReadBuffer[BUFSIZE];

    void* ret_m = memset(czReadBuffer, 0, sizeof(BUFSIZE));
    if( nullptr == ret_m )  ERROR("Fail to memset()");

    int_t nTotalBytes = m_pShmManager->GetReadShm(czReadBuffer, sizeof(czReadBuffer));
    int_t offset = 0;
    while (offset < nTotalBytes) {
        StPacketHeader* pPacket = reinterpret_cast<StPacketHeader*>(czReadBuffer + offset);

        //시뮬레이션 시작 패킷 수신
        if (pPacket->nType == PACKET_TYPE::REQUEST_SIM_START)
        {
            DEBUG("START SIMENGINE PACKET RECV");
            
            StStartRequestEnginePacket* pPacket = reinterpret_cast<StStartRequestEnginePacket*>(czReadBuffer + offset);  
            //offset += sizeof(StStartRequestEnginePacket);
            offset +=SHM_DATA_SIZE;
            //이미 시뮬레이션이 실행중일시...
            if (m_pSimulator->IsRunningSimEngine()) continue;
            m_pSimulator->SendStartSignalToEngine();

            /////////////////////////////////////////////공유메모리로 보내기...
            StStartResponseEnginePacket strtResponseStartpacket;
            PacketUtil::CreateResponseSimulationStartPacket(&strtResponseStartpacket);
            InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtResponseStartpacket), sizeof(strtResponseStartpacket));
        }
        //데이터 베이스 서버로부터 부대 정보 응답 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::RESPONSE_LOAD_MILITARY_UNIT_FROM_DB)
        {
            DEBUG("response Load military unit infomation from database");
            StResponseLoadMilitaryUnitInfoFromDBPacket* pPacket = reinterpret_cast<StResponseLoadMilitaryUnitInfoFromDBPacket*>(czReadBuffer + offset);
            offset +=SHM_DATA_SIZE;

            int16_t militaryId = htons(pPacket->strtMilitaryInfo.nID);
            int_t nXpos = htonl(pPacket->strtMilitaryInfo.nXpos);
            int_t nYpos = htonl(pPacket->strtMilitaryInfo.nYpos);
            int_t totalArtillery = htonl(pPacket->strtMilitaryInfo.nTotalArtillery);
            m_pMilitaryModel->SetMilitaryUnitPortConnect(militaryId,nXpos,nYpos,totalArtillery);

            if (pPacket->nIsFin == NETWORK_FLAG::FIN)
            {
                m_bIsLoadScenarioData.store(true);
            }
        }
        //객체 정보 요청 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::REQUEST_MILITARY_UNIT_INFO)
        {
            DEBUG("Load Obj infomation RECV");
            StRequestLoadMilitaryUnitInfoPacket* pPacket = reinterpret_cast<StRequestLoadMilitaryUnitInfoPacket*>(czReadBuffer + offset);  
            //offset += sizeof(StRequestLoadMilitaryUnitInfoPacket);
            offset +=SHM_DATA_SIZE;
            
            //DEBUG("recv packet header id: " + to_string(ntohl(pPacket->strtHeader.nID)));
            //DEBUG("recv packet user id: " + to_string(ntohl(pPacket->nUserClientFd)));
            //DEBUG("user send militaryId : " + to_string(ntohs(pPacket->nUserMilitarySection)));
            int16_t nRequestMilitaryUnit = ntohs(pPacket->nUserMilitarySection);
            
            std::vector<int16_t> vecMilitaryId;
            //admin 계정이라면 : 모든 부대정보를 모두 보내줘야함.
            if (MILITARY_ID::ADMIN_ID == nRequestMilitaryUnit)
            {
                vecMilitaryId = m_pMilitaryModel->GetAllMilitaryUnit();
            }
            else
            {
                vecMilitaryId.push_back(nRequestMilitaryUnit);
            }

            for(int_t i = 0; i < vecMilitaryId.size(); ++i)
            {
                int16_t nRequestId = vecMilitaryId[i];
                CMilitaryUnit strtMilitaryUnit;
                int8_t  nFlag = (i == vecMilitaryId.size()-1)?NETWORK_FLAG::FIN:NETWORK_FLAG::NONE;
                bool_t ret_g = m_pMilitaryModel->GetMilitaryUnit(nRequestId,&strtMilitaryUnit);
                if (!ret_g)
                {
                    ERROR("Not exist militaryId : " + to_string(nRequestId));
                }
                //user identification 업데이트
                m_pMilitaryModel->SetUserIdentificationToMilitaryUnit(ntohl(pPacket->nUserClientFd), ntohl(pPacket->strtHeader.nID),nRequestId);
                DEBUG("militaryId : " + to_string(strtMilitaryUnit.m_nID)+ " ,total Artillery : " + to_string(strtMilitaryUnit.m_nTotalArtillery));
            
                /////////////////////////////////////////////공유메모리로 보내기..
                StResponseLoadMilitaryUnitInfoPacket strtMilitaryUnitPacketInfo;
                DEBUG("header id : "+ to_string(ntohl(pPacket->strtHeader.nID)) + " user client fd : " + to_string(ntohl(pPacket->nUserClientFd)));
                PacketUtil::CreateResponseLoadMilitaryUnitPacket(ntohl(pPacket->strtHeader.nID),ntohl(pPacket->nUserClientFd),nFlag,&strtMilitaryUnit,&strtMilitaryUnitPacketInfo);
                //PacketUtil::CreateResponseLoadMilitaryUnitPacket(ntohl(pPacket->strtHeader.nID),8,&strtMilitaryUnit,&strtMilitaryUnitPacketInfo);
                InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtMilitaryUnitPacketInfo), sizeof(strtMilitaryUnitPacketInfo));
            }
            
            
        }
        //Order in Sally 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::REQUEST_ORDER_IN_SALLY)
        {
            DEBUG("Order IN Sally packet Recv");
            StRequestOrderInSallyPacket* pPacket = reinterpret_cast<StRequestOrderInSallyPacket*>(czReadBuffer + offset);  
            //offset += sizeof(StRequestOrderInSallyPacket);
            offset +=SHM_DATA_SIZE;
            CMilitaryUnit strtMilitaryUnit;
            CArtillery strtArtillery;
            
            //TODO : user가 보낸 패킷을 검증하는 작업이 필요함

            int_t nSallyArtilleryId = m_pMilitaryModel->GetNextSallyArtillery(ntohs(pPacket->nUserMilitarySection));
            if (nSallyArtilleryId == -1)
            {
                ERROR("Can not find artillery in military unit");
                continue;
            }
            bool_t ret_g = m_pMilitaryModel->GetMilitaryUnit(ntohs(pPacket->nUserMilitarySection),&strtMilitaryUnit);
            if (!ret_g)
            {
                ERROR("Not exist militaryId : " + to_string(ntohs(pPacket->nUserMilitarySection)));
                continue;
            }
            //sally 명령이므로 가지고 있는 자주포수 - 1해서 보내준다. (실제 업데이트는 명령수행후 발생)
            strtMilitaryUnit.m_nTotalArtillery--;
            ret_g = m_pMilitaryModel->GetArtillery(nSallyArtilleryId,&strtArtillery);
            if (!ret_g)
            {
                ERROR("Not exist ArtilleryId : " + to_string(nSallyArtilleryId));
                continue;
            }
            //user identification 업데이트
            m_pMilitaryModel->SetUserIdentificationToArtillery(ntohl(pPacket->nUserClientFd), ntohl(pPacket->strtHeader.nID), strtArtillery.m_nID);


            Message msgOrderInSally;
            msgOrderInSally.SetPort(P_main_IN);
            //msgOrderInSally.SetPort("EXTERNAL_" + to_string(ntohs(pPacket->nUserMilitarySection)));
            msgOrderInSally.SetContent(C_mil_ORDER_IN_SALLY);
            msgOrderInSally.SetDetail(ObjectID::MILITARY_UNIT,ntohs(pPacket->nUserMilitarySection),ntohl(pPacket->nDstXPos),ntohl(pPacket->nDstYPos));
            m_pSimulator->InjectExternalEvent(msgOrderInSally);
            //DEBUG("user send militaryId : " + to_string(htons(pPacket->nUserMilitarySection)));
            //bool_t ret_g = m_pModel->GetMilitaryUnit(htons(pPacket->nUserMilitarySection),&strtMilitaryUnit);
            //if (!ret_g)
            //{
            //    ERROR("Not exist militaryId : " + to_string(htons(pPacket->nUserMilitarySection)));
            //}
            //DEBUG("militaryId : " + to_string(strtMilitaryUnit.m_nID)+ " ,total Artillery : " + to_string(strtMilitaryUnit.m_nTotalArtillery));
            
            /////////////////////////////////////////////공유메모리로 보내기..
            StResponseOrderInSallyPacket strtResponseOrderInSally;
            PacketUtil::CreateResponseOrderInSallyPacket(ntohl(pPacket->strtHeader.nID),ntohl(pPacket->nUserClientFd),&strtMilitaryUnit,&strtArtillery,&strtResponseOrderInSally);
            InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtResponseOrderInSally), sizeof(strtResponseOrderInSally));

        }
        //Order in Attack 패킷 수신
        else if (pPacket->nType == PACKET_TYPE::REQUEST_ORDER_IN_ATTACK)
        {
            DEBUG("Order IN Attack packet Recv");
            StRequestOrderInAttackPacket* pPacket = reinterpret_cast<StRequestOrderInAttackPacket*>(czReadBuffer + offset);  
            //offset += sizeof(StRequestOrderInSallyPacket);
            offset +=SHM_DATA_SIZE;
            CArtillery strtArtillery;
            CMissile   strtMissile;            
            

            bool_t ret_g = m_pMilitaryModel->GetArtillery(ntohs(pPacket->nArtilleryId),&strtArtillery);
            if (!ret_g)
            {
                ERROR("Not exist ArtilleryId : " + to_string(ntohs(pPacket->nArtilleryId)));
                continue;
            }

            int_t nAttackMissileId = m_pMilitaryModel->GetNextMissile(ntohs(pPacket->nArtilleryId));
            if (nAttackMissileId == -1)
            {
                ERROR("Can not find missile in artillery");
                continue;
            }
            
            //attack 명령이므로 가지고 있는 미사일수 - 1해서 보내준다. (실제 업데이트는 명령수행후 발생)
            strtArtillery.m_nTotalMissile--;
            ret_g = m_pMilitaryModel->InitMissilePos(nAttackMissileId,strtArtillery.m_nXPos,strtArtillery.m_nYPos);
            ret_g = m_pMilitaryModel->GetMissile(nAttackMissileId,&strtMissile);
            if (!ret_g)
            {
                ERROR("Not exist MissileId : " + to_string(nAttackMissileId));
                continue;
            }
            //user identification 업데이트
            //TODO : missile 위치를 자주포 위치로 업데이트 후 보내야댐.
            m_pMilitaryModel->SetUserIdentificationToMissile(ntohl(pPacket->nUserClientFd), ntohl(pPacket->strtHeader.nID), strtMissile.m_nID);


            Message msgOrderInAttack;
            msgOrderInAttack.SetPort(P_main_IN);
            //msgOrderInSally.SetPort("EXTERNAL_" + to_string(ntohs(pPacket->nUserMilitarySection)));
            msgOrderInAttack.SetContent(C_art_ORDER_IN_ATTACK);
            msgOrderInAttack.SetDetail(ObjectID::ARTILLERY,ntohs(pPacket->nArtilleryId),ntohl(pPacket->nDstXPos),ntohl(pPacket->nDstYPos));
            m_pSimulator->InjectExternalEvent(msgOrderInAttack);
            //DEBUG("user send militaryId : " + to_string(htons(pPacket->nUserMilitarySection)));
            //bool_t ret_g = m_pModel->GetMilitaryUnit(htons(pPacket->nUserMilitarySection),&strtMilitaryUnit);
            //if (!ret_g)
            //{
            //    ERROR("Not exist militaryId : " + to_string(htons(pPacket->nUserMilitarySection)));
            //}
            //DEBUG("militaryId : " + to_string(strtMilitaryUnit.m_nID)+ " ,total Artillery : " + to_string(strtMilitaryUnit.m_nTotalArtillery));
            
            /////////////////////////////////////////////공유메모리로 보내기..
            StResponseOrderInAttackPacket strtResponseOrderInAttack;
            PacketUtil::CreateResponseOrderInAttackPacket(ntohl(pPacket->strtHeader.nID),ntohl(pPacket->nUserClientFd),&strtArtillery,&strtMissile,&strtResponseOrderInAttack);
            InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtResponseOrderInAttack), sizeof(strtResponseOrderInAttack));

        }
    }

}

// ********************************************************************
// * 함 수 명: OutputSendMessageParse
// * 설    명: Output y message를 파싱후 공유메모리로 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 11
// ********************************************************************
void SimEngineDataTrsfModel::OutputSendMessageParse(const Message& pOutputMessage)
{
    string strMessageContent = pOutputMessage.GetContent();
    //DEBUG("GEtContent : "+strMessageContent);
    //timeControl timeout으로 주기적으로 시간 동기화 패킷을 보낸다.
    if (strMessageContent == ModelConfig::Content::C_tic_TIMEOUT)
    {
        StSyncSimulationTimePacket strtSendPacket;
        PacketUtil::CreateSyncSimulationTimePacket(pOutputMessage.GetTime(),&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));
    }
    //자주포객체 이동중 현재 좌표를 보낸다. 도착이면 tcp, 아니면 udp로 전장상황도에 뿌려준다.
    else if (strMessageContent == ModelConfig::Content::C_art_MOVED)
    {
        //bool_t bIsArrive = pOutputMessage.GetNetFlag();
        StSyncArtilleryCoordinatePacket strtSendPacket;
        CArtillery strtArtillery;  
        bool_t ret_g = m_pMilitaryModel->GetArtillery(pOutputMessage.GetObjId(),&strtArtillery);
        if (!ret_g)
        {
            ERROR("Not exist ArtilleryId : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        bool_t bIsArrive = m_pMilitaryModel->IsArrivedFromArtillery(pOutputMessage.GetObjId());
        PacketUtil::CreateSyncArtilleryCoordinatePacket(strtArtillery.m_nCommunicationIdentification,strtArtillery.m_nUserIdentification,bIsArrive,&strtArtillery,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));
    }
    //미사일객체 이동중 현재 좌표를 보낸다. 도착이면 tcp, 아니면 udp로 전장상황도에 뿌려준다.
    else if (strMessageContent == ModelConfig::Content::C_mis_MOVED)
    {
        //bool_t bIsArrive = pOutputMessage.GetNetFlag();
        StSyncMissileCoordinatePacket strtSendPacket;
        CMissile strtMissile;  
        bool_t ret_g = m_pMilitaryModel->GetMissile(pOutputMessage.GetObjId(),&strtMissile);
        if (!ret_g)
        {
            ERROR("Not exist MissileId : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        bool_t bIsArrive = m_pMilitaryModel->IsArrivedFromMissile(pOutputMessage.GetObjId());
        PacketUtil::CreateSyncMissileCoordinatePacket(strtMissile.m_nCommunicationIdentification,strtMissile.m_nUserIdentification,bIsArrive,&strtMissile,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));

        //missile 객체 주위 오브젝트 타격.
        if (bIsArrive)
        {   
            int32_t xpos = strtMissile.m_nXPos;
            int32_t ypos = strtMissile.m_nYPos;
            std::vector<Message> vecAttackedMsg = m_pMilitaryModel->FindIncludeAttackedObject(xpos,ypos);
            for (auto msgAttackedByMissile : vecAttackedMsg)
            {
                m_pSimulator->InjectExternalEvent(msgAttackedByMissile);
            }
        }
    }
    //artillery attacked : 클라이언트는 받고서 hp가 0인지 판단하여 객체가 파괴되었는지 판단한다.
    else if (strMessageContent == ModelConfig::Content::C_art_ATTACKED)
    {
        //bool_t bIsArrive = pOutputMessage.GetNetFlag();
        StSyncArtilleryAttackedPacket strtSendPacket;
        CArtillery strtArtillery;  
        bool_t ret_g = m_pMilitaryModel->GetArtillery(pOutputMessage.GetObjId(),&strtArtillery);
        if (!ret_g)
        {
            ERROR("Not exist ArtilleryId : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        //bool_t bIsDead = m_pMilitaryModel->IsArrivedFromArtillery(pOutputMessage.GetObjId());
        PacketUtil::CreateSyncArtilleryAttackedPacket(strtArtillery.m_nCommunicationIdentification,strtArtillery.m_nUserIdentification,&strtArtillery,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));
    }
    //military unit attacked
    else if (strMessageContent == ModelConfig::Content::C_mil_ATTACKED)
    {
        StSyncMilitaryUnitAttackedPacket strtSendPacket;
        CMilitaryUnit strtMilitaryUnit;  
        bool_t ret_g = m_pMilitaryModel->GetMilitaryUnit(pOutputMessage.GetObjId(),&strtMilitaryUnit);
        if (!ret_g)
        {
            ERROR("Not exist MilitaryUnit Id : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        PacketUtil::CreateSyncMilitaryUnitAttackedPacket(strtMilitaryUnit.m_nCommunicationIdentification,strtMilitaryUnit.m_nUserIdentification,&strtMilitaryUnit,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));
    }
    //artillery destroy
    else if (strMessageContent == ModelConfig::Content::C_art_DESTROY)
    {
       //bool_t bIsArrive = pOutputMessage.GetNetFlag();
        StSyncArtilleryAttackedPacket strtSendPacket;
        CArtillery strtArtillery;  
        bool_t ret_g = m_pMilitaryModel->GetArtillery(pOutputMessage.GetObjId(),&strtArtillery);
        if (!ret_g)
        {
            ERROR("Not exist ArtilleryId : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        //bool_t bIsDead = m_pMilitaryModel->IsArrivedFromArtillery(pOutputMessage.GetObjId());
        PacketUtil::CreateSyncArtilleryAttackedPacket(strtArtillery.m_nCommunicationIdentification,strtArtillery.m_nUserIdentification,&strtArtillery,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));

        ////////////////////////////////////////////////////////////////죽은 객체 처리해야댐
        m_pMilitaryModel->RemoveArtilleryInfo(pOutputMessage.GetObjId());
    }
    //military unit destory
    else if (strMessageContent == ModelConfig::Content::C_mil_DESTROY)
    {
        StSyncMilitaryUnitAttackedPacket strtSendPacket;
        CMilitaryUnit strtMilitaryUnit;  
        bool_t ret_g = m_pMilitaryModel->GetMilitaryUnit(pOutputMessage.GetObjId(),&strtMilitaryUnit);
        if (!ret_g)
        {
            ERROR("Not exist MilitaryUnit Id : " + to_string(pOutputMessage.GetObjId()));
            return;
        }
        PacketUtil::CreateSyncMilitaryUnitAttackedPacket(strtMilitaryUnit.m_nCommunicationIdentification,strtMilitaryUnit.m_nUserIdentification,&strtMilitaryUnit,&strtSendPacket);
        InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtSendPacket), sizeof(strtSendPacket));

        ////////////////////////////////////////////////////////////////죽은 객체 처리해야댐
        m_pMilitaryModel->RemoveMilitaryUnitInfo(pOutputMessage.GetObjId());
    }
}

// ********************************************************************
// * 함 수 명: Start
// * 설    명: SimEngineDataTrsfModel을 시작한다
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
void SimEngineDataTrsfModel::Start()
{
    //초기화
    Initialize();

    ///////////////////////////////////////////////internal process
    m_thrInternalRecvThread = std::thread([this](){
        this->InternalProcessStart();
    }); 

    /////////////////////////////////////////////////database로 시나리오 요청
    StRequestLoadMilitaryUnitInfoFromDBPacket strtRequestToDatabasePacket;
    PacketUtil::CreateRequestLoadMilitaryUnitFromDB(&strtRequestToDatabasePacket);
    InternalProcessSendRoutine(reinterpret_cast<char_t*>(&strtRequestToDatabasePacket),sizeof(strtRequestToDatabasePacket));
    
    //database에서 시나리오 데이터(부대정보)를 다 받을 때까지 대기
    while(m_bIsLoadScenarioData.load() == false);

    m_pSimulator->StartEngine(m_pMainModel);
    m_thrInternalRecvThread.join();
    m_pSimulator->JoinEngineThread(); 
}

// ********************************************************************
// * 함 수 명: Finish
// * 설    명: SimEngineDataTrsfModel을 끝낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
void SimEngineDataTrsfModel::Finish()
{
    m_pSimulator->StopEngine();
    m_bIsExit.store(true);
}
}