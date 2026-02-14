// ********************************************************************
// * 소스정의: PacketUtil.h
// * 설    명: packet protocol 과 관련한 유틸함수들을 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
#include "pch.h"
#include "PacketUtil.h"


using namespace DATATRSFER;

// ********************************************************************
// * 함 수 명: ParsePacketInfo
// * 설    명: 패킷을 파싱한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
int_t PacketUtil::ParsePacketInfo(const char_t* pPacket)
{
    return -1;
}

// ********************************************************************
// * 함 수 명: CreateResponseLoginpacket
// * 설    명: accept후에 클라이언트가 보낸 로그인 요청응답
// *           1.   로그인 성공 실패 여부 반환
// *           2.   성공시 clientFd(clientID)전송후 항상 포함하도록...
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
void PacketUtil::CreateResponseLoginPacket(int32_t nClientFd, struct _Packet* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::RESPONSE_LOGIN;
    pStrtPacket->strtHeader.nID = htonl(nClientFd);
}

// ********************************************************************
// * 함 수 명: CreateRequestLoginPacket
// * 설    명: accept후에 user에게 로그인 정보를 요청
// *           clientFd(clientID)전송후 항상 포함하도록...
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
void PacketUtil::CreateRequestLoginPacket(const char_t* strUserId,const char_t* strUserPW,StLoginRequestPacketData* pStrtPacket)
{
    // 전체 초기화
    memset(pStrtPacket, 0, sizeof(StLoginRequestPacketData));


    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_LOGIN;
    pStrtPacket->strtHeader.nID = htonl(CLIENT_TYPE::INTEGRATE_LOGIN_SYSTEM);
    
    // User ID 채우기 (최대 63바이트 + 널 문자)
    strncpy_s(pStrtPacket->nUserId, strUserId, sizeof(pStrtPacket->nUserId) - 1);
    pStrtPacket->nUserId[sizeof(pStrtPacket->nUserId) - 1] = '\0';

    // User PW 채우기 (최대 63바이트 + 널 문자)
    strncpy_s(pStrtPacket->nUserPw, strUserPW, sizeof(pStrtPacket->nUserPw) - 1);
    pStrtPacket->nUserPw[sizeof(pStrtPacket->nUserPw) - 1] = '\0';

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
    pStrtPacket->strtHeader.nID = htonl(nClientFd);

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
    pStrtPacket->strtHeader.nID = htonl(nClientFd);

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
    pStrtPacket->strtHeader.nID = htonl(nClientFd);

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
void PacketUtil::CreateRequestLoadMilitaryUnitPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, StRequestLoadMilitaryUnitInfoPacket* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_MILITARY_UNIT_INFO;
    pStrtPacket->strtHeader.nID = htonl(nThisFd);
    pStrtPacket->nUserClientFd = htonl(nClientFd);
    pStrtPacket->nUserMilitarySection = htons(nMilitaryId);
    DEBUG("원본 militaryId : " + std::to_string(nMilitaryId));
    DEBUG("send militaryId : " + std::to_string(pStrtPacket->nUserMilitarySection));
}

// ********************************************************************
// * 함 수 명: CreateResponseLoadMilitaryUnitPacket
// * 설    명: 부대id에 맞게 필요한 객체정보를 요청한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
void PacketUtil::CreateResponseLoadMilitaryUnitPacket(int32_t nClientFd, const StResponseLoadMilitaryUnitInfoPacket* pRecvPacket, StResponseLoadMilitaryUnitInfoPacket* pSendPacket)
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
void PacketUtil::CreateRequestOrderInSallyPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, int32_t nDstX, int32_t nDstY, StRequestOrderInSallyPacket* pStrtPacket)
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
void PacketUtil::CreateResponseOrderInSallyPacket(int32_t nClientFd, const StResponseOrderInSallyPacket* pRecvPacket, StResponseOrderInSallyPacket* pSendPacket)
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
    pSendPacket->strtArtilleryInfo.nTotalMissile = htonl(pRecvPacket->strtArtilleryInfo.nTotalMissile);
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;
}

// ********************************************************************
// * 함 수 명: CreateRequestOrderInAttackPacket
// * 설    명: 부대에 missile attack명령 요청 packet 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
void PacketUtil::CreateRequestOrderInAttackPacket(int32_t nThisFd, int32_t nClientFd, int16_t nArtilleryId, int32_t nDstX, int32_t nDstY, StRequestOrderInAttackPacket* pStrtPacket)
{
    pStrtPacket->strtHeader.nType = PACKET_TYPE::REQUEST_ORDER_IN_ATTACK;
    pStrtPacket->strtHeader.nID = htonl(nThisFd);
    pStrtPacket->nUserClientFd = htonl(nClientFd);
    pStrtPacket->nArtilleryId = htons(nArtilleryId);
    pStrtPacket->nDstXPos = htonl(nDstX);
    pStrtPacket->nDstYPos = htonl(nDstY);

}

// ********************************************************************
// * 함 수 명: CreateResponseOrderInAttackPacket
// * 설    명: 부대에 missile attack명령 요청에 대한 응답 패킷 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
void PacketUtil::CreateResponseOrderInAttackPacket(int32_t nClientFd, const StResponseOrderInAttackPacket* pRecvPacket, StResponseOrderInAttackPacket* pSendPacket)
{
}

// ********************************************************************
// * 함 수 명: CreateSyncArtilleryCoordinatePacket
// * 설    명: 자주포위치를 시뮬레이션 엔진과 동기화한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 11
// ********************************************************************
void PacketUtil::CreateSyncArtilleryCoordinatePacket(int32_t nClientFd, const StSyncArtilleryCoordinatePacket* pRecvPacket, StSyncArtilleryCoordinatePacket* pSendPacket)
{
    pSendPacket->strtHeader.nID = htonl(nClientFd);
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_ARTILLERY_COORDINATE;
    pSendPacket->strtHeader.nFlagField = pRecvPacket->strtHeader.nFlagField;
    pSendPacket->nUserClientFd = pRecvPacket->nUserClientFd;

    //자주포정보
    pSendPacket->strtArtilleryInfo.nID = pRecvPacket->strtArtilleryInfo.nID;
    pSendPacket->strtArtilleryInfo.nTotalMissile = htonl(pRecvPacket->strtArtilleryInfo.nTotalMissile);
    pSendPacket->strtArtilleryInfo.nUnitHealth = pRecvPacket->strtArtilleryInfo.nUnitHealth;
    pSendPacket->strtArtilleryInfo.nXpos = pRecvPacket->strtArtilleryInfo.nXpos;
    pSendPacket->strtArtilleryInfo.nYpos = pRecvPacket->strtArtilleryInfo.nYpos;
}
// ********************************************************************
// * 함 수 명: CreateSyncMissileCoordinatePacket
// * 설    명: missile객체 위치값 동기화 패킷 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
void PacketUtil::CreateSyncMissileCoordinatePacket(int32_t nClientFd, const StSyncMissileCoordinatePacket* pRecvPacket, StSyncMissileCoordinatePacket* pSendPacket)
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
// * 함 수 명: CreateSyncSimulationTimePacket
// * 설    명: broadcast 로 모든 클라이언트에게 현재 시뮬레이션 시각을 알린다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 11
// ********************************************************************
void PacketUtil::CreateSyncSimulationTimePacket(StSyncSimulationTimePacket* pSendPacket, const StSyncSimulationTimePacket* pRecvPacket)
{
    pSendPacket->strtHeader.nType = PACKET_TYPE::SYNC_SIM_TIME;
    pSendPacket->nCurrentSimulationTime = pRecvPacket->nCurrentSimulationTime;
}