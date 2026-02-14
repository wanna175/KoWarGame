// ********************************************************************
// * 소스정의: PacketUtil.h
// * 설    명: packet protocol 과 관련한 유틸함수들을 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
#include "PacketUtil.h"
#include "TypeDef.h"
#include "MACRO.h"
#include "Database.h"

#include <vector>
#include <string>
#include <netinet/in.h>

using std::string;
using std::vector;

using namespace DATATRSFER;
using namespace DATABASE;

// ********************************************************************
// * 함 수 명: ParsePacketheaderInfo
// * 설    명: packet에서 header정보를 뽑는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
vector<string> PacketUtil::ParsePacketHeaderInfo(const char_t* pPacket)
{

}

// ********************************************************************
// * 함 수 명: ParsePacketBodyInfo
// * 설    명: packet에서 body정보를 뽑는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
vector<string> PacketUtil::ParsePacketBodyInfo(const char_t* pSBIPacket)
{

}

// ********************************************************************
// * 함 수 명: ParsePacketInfo
// * 설    명: 패킷을 파싱한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
int_t PacketUtil::ParsePacketInfo(const char_t* pPacket)
{

}

// ********************************************************************
// * 함 수 명: CreateResponseLoginpacket
// * 설    명: accept후에 클라이언트가 보낸 로그인 요청응답(0번 서버용)
// *           1.   로그인 성공 실패 여부 반환
// *           2.   성공시 clientFd(clientID)전송후 항상 포함하도록...
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
void PacketUtil::CreateResponseLoginPacket(int32_t nClientFd,int32_t nUserClientFd, int8_t nUserAuth, int16_t nUserMilitaryId, int8_t nLoginState, StLoginResponsePacketData* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_LOGIN;
    pStrtPacket->strtHeader.nID   = htonl(nClientFd);
    pStrtPacket->nUserClientFd    = htonl(nUserClientFd);
    pStrtPacket->nUserAuthority   = nUserAuth;
    pStrtPacket->nUserMilitarySection = htons(nUserMilitaryId); 

    switch(nLoginState)
    {
        //not found username 
        case -1:
            pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::LOGIN_NO_FOUND_ID;
            break;
        //db error
        case -2:
            pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::SERVER_ERROR;
            break;
        //not correct password
        case  0:
            pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::LOGIN_NO_FOUND_PW;
            break;
        //login success
        case  1:
            pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::LOGIN_SUCCESS;
            break;
        default:
            pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::SERVER_ERROR;
            break;
    }
}

// ********************************************************************
// * 함 수 명: CreateResponseLoginToClientPacket
// * 설    명: accept후에 클라이언트가 보낸 로그인 요청응답(1번 서버용)
// *           1.   로그인 성공 실패 여부 반환
// *           2.   성공시 clientFd(clientID)전송후 항상 포함하도록...
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
void PacketUtil::CreateResponseLoginToClientPacket(int32_t nClientFd,const StLoginResponsePacketData* pRecvPacket,StLoginResponsePacketData* pSendPacket)
{
    pSendPacket->strtHeader.nType = pRecvPacket->strtHeader.nType;
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;

    pSendPacket->nUserAuthority = pRecvPacket->nUserAuthority;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;
    pSendPacket->nUserMilitarySection = pRecvPacket->nUserMilitarySection;
}

// ********************************************************************
// * 함 수 명: CreateRequestLoginPacket
// * 설    명: accept후에 user에게 로그인 정보를 요청
// *           clientFd(clientID)전송후 항상 포함하도록...
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
void PacketUtil::CreateRequestLoginPacket(int32_t nClientFd, int32_t nUserClientFd, const StLoginRequestPacketData* pRecvPacket, StLoginRequestPacketData* pSendPacket)
{
    pSendPacket->strtHeader.nType = PACKET_TYPE::REQUEST_LOGIN;
    pSendPacket->strtHeader.nID   = htonl(nClientFd);
    pSendPacket->nUserClientFd    = htonl(nUserClientFd);

    memcpy(pSendPacket->nUserId, pRecvPacket->nUserId, sizeof(pSendPacket->nUserId));
    memcpy(pSendPacket->nUserPw, pRecvPacket->nUserPw, sizeof(pSendPacket->nUserPw));

}

// ********************************************************************
// * 함 수 명: CreateRequettHelloUdp
// * 설    명: 로그인 성공 후에 client가 보낼 HelloUdp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 11
// ********************************************************************
void PacketUtil::CreateRequestHelloUdp(int32_t nClientFd, struct _Packet* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::HELLO_UDP;
    pStrtPacket->strtHeader.nID   = htonl(nClientFd);

}

// ********************************************************************
// * 함 수 명: CreateResponseHelloUdp
// * 설    명: 로그인 성공 후에 client가 보낸 helloUDP에 대한 응답
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
void PacketUtil::CreateResponseHelloUdp(int32_t nClientFd, struct _Packet* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::HELLO_UDP_RESPONSE;
    pStrtPacket->strtHeader.nID   = htonl(nClientFd);
    pStrtPacket->strtHeader.nFlagField = NETWORK_FLAG::LOGIN_SUCCESS;

}

// ********************************************************************
// * 함 수 명: CreateRequestConnectPacket
// * 설    명: 상위 데이터전달처리기에 접속요청을 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ********************************************************************
void PacketUtil::CreateRequestConnectPacket(int32_t nKey, struct _Packet* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_CONNECT;
    pStrtPacket->strtHeader.nID   = htonl(nKey);

}

// ********************************************************************
// * 함 수 명: CreateResponseConnectPacket
// * 설    명: 상위 데이터전달처리기에 접속응답을 보낸다.          
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ********************************************************************
void PacketUtil::CreateResponseConnectPacket(int32_t nClientFd, struct _Packet* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_CONNECT;
    pStrtPacket->strtHeader.nID   = htonl(nClientFd);

}
// ********************************************************************
// * 함 수 명: CreateUpdateRoutingPathPacket
// * 설    명: 상위 데이터전달처리기에 하위서버의 클라이언트 파일디스크립터를 삭제요청 한다.   
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
void PacketUtil::CreateUpdateRoutingPathPacket(int32_t nUpperFd, int32_t nClientFd, StUpdateRoutingPath *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::UPDATE_ROUTING_INFO;
    pStrtPacket->strtHeader.nID   = htonl(nUpperFd);
    pStrtPacket->nUserClientFd    = htonl(nClientFd);
}

// ********************************************************************
// * 함 수 명: CreateRequestSimulationStartPacket
// * 설    명: SimEngine과 연결된 상위 데이터전달처리기에 시뮬레이션 시작 
// *           패킷을 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
void PacketUtil::CreateRequestSimulationStartPacket(int32_t nClientFd, StStartRequestEnginePacket* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_SIM_START;
    pStrtPacket->strtHeader.nID   = htonl(nClientFd);

}

// ********************************************************************
// * 함 수 명: CreateResponseSimulationStartPacket
// * 설    명: broadcast 로 모든 클라이언트에게 시뮬레이션 시작을 알린다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
void PacketUtil::CreateResponseSimulationStartPacket(StStartResponseEnginePacket* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_SIM_START;
    //pStrtPacket->strtHeader.nID   = htonl(nClientFd);

}

// ********************************************************************
// * 함 수 명: CreateRequestLoadMilitaryUnitPacket
// * 설    명: 부대id에 맞게 필요한 객체정보를 요청한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
void PacketUtil::CreateRequestLoadMilitaryUnitPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, StRequestLoadMilitaryUnitInfoPacket *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_MILITARY_UNIT_INFO;
    pStrtPacket->strtHeader.nID = htonl(nThisFd);
    pStrtPacket->nUserClientFd = htonl(nClientFd);
    pStrtPacket->nUserMilitarySection = htons(nMilitaryId);
}

// ********************************************************************
// * 함 수 명: CreateResponseLoadMilitaryUnitPacket
// * 설    명: 부대id에 맞게 필요한 객체정보를 요청한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
void PacketUtil::CreateResponseLoadMilitaryUnitPacket(int32_t nClientFd, const StResponseLoadMilitaryUnitInfoPacket *pRecvPacket, StResponseLoadMilitaryUnitInfoPacket *pSendPacket)
{
    //*pSendPacket = *pRecvPacket;

    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_MILITARY_UNIT_INFO;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;
    pSendPacket->strtMilitaryInfo.nID = pRecvPacket->strtMilitaryInfo.nID;
    pSendPacket->strtMilitaryInfo.nTotalArtillery = pRecvPacket->strtMilitaryInfo.nTotalArtillery;
    pSendPacket->strtMilitaryInfo.nUnitHealth = pRecvPacket->strtMilitaryInfo.nUnitHealth;
    pSendPacket->strtMilitaryInfo.nXpos = pRecvPacket->strtMilitaryInfo.nXpos;
    pSendPacket->strtMilitaryInfo.nYpos = pRecvPacket->strtMilitaryInfo.nYpos;
    pSendPacket->nIsFin = pRecvPacket->nIsFin;

}

// ********************************************************************
// * 함 수 명: CreateRequestOrderInSallyPacket
// * 설    명: 부대에 자주포 출격명령 요청을 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 01
// ********************************************************************
void PacketUtil::CreateRequestOrderInSallyPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, int32_t nDstX, int32_t nDstY, StRequestOrderInSallyPacket *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_ORDER_IN_SALLY;
    pStrtPacket->strtHeader.nID = htonl(nThisFd);
    pStrtPacket->nUserClientFd = htonl(nClientFd);
    pStrtPacket->nUserMilitarySection = htons(nMilitaryId);
    pStrtPacket->nDstXPos = htonl(nDstX);
    pStrtPacket->nDstYPos = htonl(nDstY);
}

// ********************************************************************
// * 함 수 명: CreateResponseOrderInSallyPacket
// * 설    명: 부대에 자주포 출격명령 요청에 대한 응답을 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 01
// ********************************************************************
void PacketUtil::CreateResponseOrderInSallyPacket(int32_t nClientFd, const StResponseOrderInSallyPacket *pRecvPacket, StResponseOrderInSallyPacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_ORDER_IN_SALLY;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //부대정보
    pSendPacket->strtMilitaryInfo.nID = pRecvPacket->strtMilitaryInfo.nID;
    pSendPacket->strtMilitaryInfo.nTotalArtillery = pRecvPacket->strtMilitaryInfo.nTotalArtillery;
    pSendPacket->strtMilitaryInfo.nUnitHealth = pRecvPacket->strtMilitaryInfo.nUnitHealth;
    pSendPacket->strtMilitaryInfo.nXpos = pRecvPacket->strtMilitaryInfo.nXpos;
    pSendPacket->strtMilitaryInfo.nYpos = pRecvPacket->strtMilitaryInfo.nYpos;

    //자주포정보
    pSendPacket->strtArtilleryInfo.nID = pRecvPacket->strtArtilleryInfo.nID;
    pSendPacket->strtArtilleryInfo.nTotalMissile = pRecvPacket->strtArtilleryInfo.nTotalMissile;
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;
}

// ********************************************************************
// * 함 수 명: CreateRequestOrderInAttackPacket
// * 설    명: 자주포에 미사일 발사명령 요청을 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
void PacketUtil::CreateRequestOrderInAttackPacket(int32_t nThisFd, int32_t nClientFd, int16_t nArtilleyId, int32_t nDstX, int32_t nDstY, StRequestOrderInAttackPacket *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_ORDER_IN_ATTACK;
    pStrtPacket->strtHeader.nID = htonl(nThisFd);
    pStrtPacket->nUserClientFd = htonl(nClientFd);
    pStrtPacket->nArtilleryId = htons(nArtilleyId);
    pStrtPacket->nDstXPos = htonl(nDstX);
    pStrtPacket->nDstYPos = htonl(nDstY);
}

// ********************************************************************
// * 함 수 명: CreateResponseOrderInAttackPacket
// * 설    명: 자주포에 미사일 발사명령 요청에 대한 응답을 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
void PacketUtil::CreateResponseOrderInAttackPacket(int32_t nClientFd, const StResponseOrderInAttackPacket *pRecvPacket, StResponseOrderInAttackPacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_ORDER_IN_ATTACK;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //미사일정보
    pSendPacket->strtMissileInfo.nID = pRecvPacket->strtMissileInfo.nID;
    pSendPacket->strtMissileInfo.nXpos = pRecvPacket->strtMissileInfo.nXpos;
    pSendPacket->strtMissileInfo.nYpos = pRecvPacket->strtMissileInfo.nYpos;

    //자주포정보
    pSendPacket->strtArtilleryInfo.nID = pRecvPacket->strtArtilleryInfo.nID;
    pSendPacket->strtArtilleryInfo.nTotalMissile = pRecvPacket->strtArtilleryInfo.nTotalMissile;
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;
}

// ********************************************************************
// * 함 수 명: CreateSyncArtilleryCoordinatePacket
// * 설    명: 자주포위치를 시뮬레이션 엔진과 동기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 11
// ********************************************************************
void PacketUtil::CreateSyncArtilleryCoordinatePacket(int32_t nClientFd, const StSyncArtilleryCoordinatePacket *pRecvPacket, StSyncArtilleryCoordinatePacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_ARTILLERY_COORDINATE;
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //자주포정보
    pSendPacket->strtArtilleryInfo.nID = pRecvPacket->strtArtilleryInfo.nID;
    pSendPacket->strtArtilleryInfo.nTotalMissile = pRecvPacket->strtArtilleryInfo.nTotalMissile;
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;
}

// ********************************************************************
// * 함 수 명: CreateSyncMissileCoordinatePacket
// * 설    명: 미사일위치를 시뮬레이션 엔진과 동기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
void PacketUtil::CreateSyncMissileCoordinatePacket(int32_t nClientFd, const StSyncMissileCoordinatePacket *pRecvPacket, StSyncMissileCoordinatePacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_MISSILE_COORDINATE;
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //missile정보
    pSendPacket->strtMissileInfo.nID = pRecvPacket->strtMissileInfo.nID;
    pSendPacket->strtMissileInfo.nXpos = pRecvPacket->strtMissileInfo.nXpos;
    pSendPacket->strtMissileInfo.nYpos = pRecvPacket->strtMissileInfo.nYpos;
}

// ********************************************************************
// * 함 수 명: CreateSyncArtilleryAttackedPacket
// * 설    명: 자주포 피격 이벤트를 시뮬레이션 엔진과 동기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
void PacketUtil::CreateSyncArtilleryAttackedPacket(int32_t nClientFd, const StSyncArtilleryAttackedPacket *pRecvPacket, StSyncArtilleryAttackedPacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_ARTILLERY_ATTACKED;
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //자주포정보
    pSendPacket->strtArtilleryInfo.nID = pRecvPacket->strtArtilleryInfo.nID;
    pSendPacket->strtArtilleryInfo.nTotalMissile = pRecvPacket->strtArtilleryInfo.nTotalMissile;
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;

}

// ********************************************************************
// * 함 수 명: CreateSyncMilitaryUnitAttackedPacket
// * 설    명: 부대 피격이벤트를 시뮬레이션 엔진과 동기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
void PacketUtil::CreateSyncMilitaryUnitAttackedPacket(int32_t nClientFd, const StSyncMilitaryUnitAttackedPacket *pRecvPacket, StSyncMilitaryUnitAttackedPacket *pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_MILITARY_UNIT_ATTACKED;
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //자주포정보
    pSendPacket->strtMilitaryInfo.nID = pRecvPacket->strtMilitaryInfo.nID;
    pSendPacket->strtMilitaryInfo.nTotalArtillery = pRecvPacket->strtMilitaryInfo.nTotalArtillery;
    pSendPacket->strtMilitaryInfo.nUnitHealth = pRecvPacket->strtMilitaryInfo.nUnitHealth;
}
    

// ********************************************************************
// * 함 수 명: CreateSyncSimulationTimePacket
// * 설    명: broadcast 로 모든 클라이언트에게 현재 시뮬레이션 시각을 알린다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 11
// ********************************************************************
void PacketUtil::CreateSyncSimulationTimePacket(StSyncSimulationTimePacket *pSendPacket, const StSyncSimulationTimePacket *pRecvPacket)
{
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_SIM_TIME;
    pSendPacket->nCurrentSimulationTime = pRecvPacket->nCurrentSimulationTime;
}

// ********************************************************************
// * 함 수 명: CreateRequestLoadMilitaryUnitFromDB
// * 설    명: 데이터베이스 서버로부터 부대정보를 요청한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
void PacketUtil::CreateRequestLoadMilitaryUnitFromDB(StRequestLoadMilitaryUnitInfoFromDBPacket *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_LOAD_MILITARY_UNIT_FROM_DB;
}

// ********************************************************************
// * 함 수 명: CreateResponseLoadMilitaryUnitFromDB
// * 설    명: 읽어들인 부대정보를 엔진서버에 보내는 패킷을 구성한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
void PacketUtil::CreateResponseLoadMilitaryUnitFromDB(int8_t nIsFinFlag, StMilitaryUnitInfo *pStrtMilitaryUnit ,StResponseLoadMilitaryUnitInfoFromDBPacket *pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_LOAD_MILITARY_UNIT_FROM_DB;
    pStrtPacket->strtMilitaryInfo.nID = htons(pStrtMilitaryUnit->m_nID);
    pStrtPacket->strtMilitaryInfo.nTotalArtillery = htonl(pStrtMilitaryUnit->m_nTotalArtillery);
    pStrtPacket->strtMilitaryInfo.nXpos = htonl(pStrtMilitaryUnit->m_nXPos);
    pStrtPacket->strtMilitaryInfo.nYpos = htonl(pStrtMilitaryUnit->m_nYPos);
    pStrtPacket->nIsFin = nIsFinFlag;
}