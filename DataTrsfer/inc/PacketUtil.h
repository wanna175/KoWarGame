// ********************************************************************
// * 소스정의: PacketUtil.h
// * 설    명: 프로토콜및 필요한 구조체를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
#pragma once

#include <ctime>
#include <string>
#include <deque>
#include <vector>
#include <cstdint>
#include "TypeDef.h"
#include "Database.h"

using std::vector;
using std::deque;
using namespace DATABASE;

namespace DATATRSFER {

// ********************************************************************
// * 구조체명: _NetObjectInfo
// * 설    명: 네트워크에 연결된 객체(세션 정보)를 저장하는 자료구조
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 06
// ********************************************************************
/*
typedef struct _NetObjectInfo
{
    std::string             strRoutingPath;
    std::string             strIpAddress;
    int_t                   nTcpSocket;
    sockaddr_in             strtUdpAddr;
    int32_t                 nID;                      // 사용자 식별 ID
    int_t                   nProgramType;                     
    int_t                   nPID;
    std::string             strConnStatus;
	time_t                  LoginTIME;	// 로그인 시간

    deque<vector<char_t>>   deqSendQueue;         //전송 큐
}StNetObjectInfo;
*/

enum PACKET_TYPE : uint8_t
{
    ///////////////////////////////Client와의 통신
    REQUEST_LOGIN               = 0,     //login request 
	RESPONSE_LOGIN              = 1,     //login connect 
    DISCONNECTED_LOGIN_SYSTEM   = 2,     //logout  
    HELLO_UDP                   = 3,     //helloUdp
    HELLO_UDP_RESPONSE          = 4,     //helloUdp response

    ///////////////////////////////UpperServer와 통신
    REQUEST_CONNECT             = 5,     //upperserver와 연결요청
    RESPONSE_CONNECT            = 6,     //upperserver의 응답
    UPDATE_ROUTING_INFO         = 7,     //upperserver에게 routing path info를 전달

    ///////////////////////////////모의통제도구 와 통신
    REQUEST_SIM_START           = 10,    //시뮬레이션 시작 요청
    REQUEST_SIM_STOP            = 11,    //시뮬레이션 중단 요청


    RESPONSE_SIM_START          = 12,    //시뮬레이션 시작 응답(broadcast)


    ///////////////////////////////전장상황도 와 통신
    REQUEST_MILITARY_UNIT_INFO            = 20,   //부대정보 요청
    RESPONSE_MILITARY_UNIT_INFO           = 21,   //부대정보 응답

    REQUEST_ORDER_IN_SALLY                = 31,   //출격명령 요청
    RESPONSE_ORDER_IN_SALLY               = 32,   //출격명령 응답
    REQUEST_ORDER_IN_ATTACK               = 33,   //미사일 발사 요청
    RESPONSE_ORDER_IN_ATTACK              = 34,   //미사일 발사 응답

    SYNC_ARTILLERY_COORDINATE             = 45,   //자주포객체의 좌표 동기화(udp 통신)
    SYNC_MISSILE_COORDINATE               = 46,   //미사일객체의 좌표 동기화(udp 통신)
    SYNC_ARTILLERY_ATTACKED               = 47,   //자주포객체의 피격 이벤트 동기화
    SYNC_MILITARY_UNIT_ATTACKED           = 48,   //부대객체의 피격 이벤트 동기화


    ///////////////////////////////시뮬레이션 시간 키핑관련 통신
    SYNC_SIM_TIME                         = 50,   //시뮬레이션 엔진과 시간 동기화(broadcast)

    ///////////////////////////////database server 와 통신
    REQUEST_LOAD_MILITARY_UNIT_FROM_DB    = 60,   //db server에 부대정보 요청
    RESPONSE_LOAD_MILITARY_UNIT_FROM_DB   = 61,   //db server에 부대정보 응답
};

enum USER_AUTHORITY : uint8_t
{
    USER               = 0,     //user 
	ADMIN              = 1,     //admin 
};
enum MILITARY_ID : uint16_t
{
    ADMIN_ID           = 0,     //admin 계정. 모든 시나리오의 부대정보를 전송해줘야함
    DEFAULT            = 1,     //default 부대 id
};
enum NETWORK_FLAG : uint8_t
{
    UDP               = 0,     //udp
	TCP               = 1,     //tcp
    BROADCAST_UDP     = 2,
    BROADCAST_TCP     = 3,


    /////////////////////////////load obj 관련 flag
    NONE              = 10,
    FIN               = 11,

    /////////////////////////////login 관련 flag
    LOGIN_SUCCESS       = 20,
    LOGIN_NO_FOUND_ID   = 21,
    LOGIN_NO_FOUND_PW   = 22,

    SERVER_ERROR        = 30,
};
#pragma pack(push, 1)
//////////////////////////////////////////////////////////pragma pack 1

// ********************************************************************
// * 구조체명: _MilitaryUnitPacketInfo
// * 설    명: militaryUnit정보 패킷 전달용 구조체
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
typedef struct _MilitaryUnitPacketInfo{
    uint16_t        nID;
    uint32_t        nTotalArtillery;
    uint32_t        nXpos;
    uint32_t        nYpos;
    uint32_t        nUnitHealth; 
}StMilitaryPacketInfo;

// ********************************************************************
// * 구조체명: _ArtilleryPacketInfo
// * 설    명: Artillery정보 패킷 전달용 구조체
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 01
// ********************************************************************
typedef struct _ArtilleryPacketInfo{
    uint16_t        nID;
    uint32_t        nTotalMissile;
    uint32_t        nXpos;
    uint32_t        nYpos;
    uint32_t        nUnitHealth; 
}StArtilleryPacketInfo;

// ********************************************************************
// * 구조체명: _MissilePacketInfo
// * 설    명: Missile정보 패킷 전달용 구조체
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
typedef struct _MissilePacketInfo{
    uint32_t        nID;
    uint32_t        nXpos;
    uint32_t        nYpos;
}StMissilePacketInfo;

// ********************************************************************
// * 구조체명: _PacketHeader
// * 설    명: packet header 를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 09
// ********************************************************************
typedef struct _PacketHeader{
    uint8_t         nType;             //packet type
    int32_t         nID;               //사용자 식별 ID (서버의 clientsocketFD)
                                       //처음 요청 보낼시 프로그램 타입을 전송해준다.
    uint8_t         nFlagField;        // 현재는 tcp, udp 로 보낼지 확인하는 용도로 사용한다.
}StPacketHeader;

// ********************************************************************
// * 구조체명: _LoginRequestPacketData
// * 설    명: 로그인 요청 packet payload 를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 15
// ********************************************************************
typedef struct _LoginRequestPacketData{
    StPacketHeader   strtHeader;
    int32_t          nUserClientFd;
    char_t           nUserId[64];
    char_t           nUserPw[64];
}StLoginRequestPacketData;

// ********************************************************************
// * 구조체명: _LoginResponsePacketData
// * 설    명: 로그인 응답 packet payload 를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 15
// ********************************************************************
typedef struct _LoginResponsePacketData{
    StPacketHeader  strtHeader;
    int32_t         nUserClientFd;
    int8_t          nUserAuthority;         //사용자의 권한
    int16_t         nUserMilitarySection;   //사용자가 관리하는 부대ID 디비에서 읽어서 보내주자.
}StLoginResponsePacketData;

// ********************************************************************
// * 구조체명: _UpdateRoutingPath
// * 설    명: 상위 데이터 전달처리에게 하위서버의 라우팅 정보를 삭제한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
typedef struct _UpdateRoutingPath{
    StPacketHeader  strtHeader;
    int32_t         nUserClientFd;          //remove 할 client fd정보
}StUpdateRoutingPath;

// ********************************************************************
// * 구조체명: _StartRequestEnginePacket
// * 설    명: 모의통제도구의 엔진 시작요청 패킷
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 29
// ********************************************************************
typedef struct _StartRequestEnginePacket{
    StPacketHeader  strtHeader;
}StStartRequestEnginePacket;

// ********************************************************************
// * 구조체명: _StartResponseEnginePacket
// * 설    명: simengine의 엔진 시작응답 패킷
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 29
// ********************************************************************
typedef struct _StartResponseEnginePacket{
    StPacketHeader  strtHeader;
}StStartResponseEnginePacket;

// ********************************************************************
// * 구조체명: _RequestLoadMilitaryUnitInfoPacket
// * 설    명: 클라이언트의 필요한 객체정보 요청
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
typedef struct _RequestLoadMilitaryUnitInfoPacket{
    StPacketHeader  strtHeader;
    int32_t         nUserClientFd;          //클라이언트를 식별하기 위함.
    int16_t         nUserMilitarySection;   //요청하는 부대ID
}StRequestLoadMilitaryUnitInfoPacket;

// ********************************************************************
// * 구조체명: _ResponseLoadMilitaryUnitInfoPacket
// * 설    명: 클라이언트의 필요한 객체정보 응답
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 31
// ********************************************************************
typedef struct _ResponseLoadMilitaryUnitInfoPacket{
    StPacketHeader       strtHeader;
    int32_t              nUserClientFd;          //클라이언트를 식별하기 위함.
    StMilitaryPacketInfo strtMilitaryInfo;       //요청한 부대정보 
    int8_t               nIsFin;                 //마지막 부대정보인지 여부
}StResponseLoadMilitaryUnitInfoPacket;

// ********************************************************************
// * 구조체명: _RequestOrderInSallyPacket
// * 설    명: 해당 부대에 자주포 출격명령 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 01
// ********************************************************************
typedef struct _RequestOrderInSallyPacket{
    StPacketHeader  strtHeader;
    int16_t         nUserMilitarySection;   //요청하는 부대ID
    int32_t         nUserClientFd;          //클라이언트를 식별하기 위함.
    int32_t         nDstXPos;               //자주포 목적 x값 
    int32_t         nDstYPos;               //자주포 목적 y값
}StRequestOrderInSallyPacket;

// ********************************************************************
// * 구조체명: _ResponseOrderInSallyPacket
// * 설    명: 해당 부대에 자주포 출격명령 응답 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 01
// ********************************************************************
typedef struct _ResponseOrderInSallyPacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StMilitaryPacketInfo  strtMilitaryInfo;       //명령을 내린 부대정보
    StArtilleryPacketInfo strtArtilleryInfo;      //출격중인 자주포정보
}StResponseOrderInSallyPacket;

// ********************************************************************
// * 구조체명: _RequestOrderInAttackPacket
// * 설    명: 해당 자주포에 미사일 발사명령 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
typedef struct _RequestOrderInAttackPacket{
    StPacketHeader  strtHeader;
    int16_t         nArtilleryId;           //요청하는 자주포ID
    int32_t         nUserClientFd;          //클라이언트를 식별하기 위함.
    int32_t         nDstXPos;               //미사일 목적 x값 
    int32_t         nDstYPos;               //미사일 목적 y값
}StRequestOrderInAttackPacket;

// ********************************************************************
// * 구조체명: _ResponseOrderInAttackPacket
// * 설    명: 해당 자주포에 미사일 발사명령 응답 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
typedef struct _ResponseOrderInAttackPacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StArtilleryPacketInfo strtArtilleryInfo;      //update된 자주포정보
    StMissilePacketInfo   strtMissileInfo;        //발사된 미사일 정보
}StResponseOrderInAttackPacket;

// ********************************************************************
// * 구조체명: _SyncArtilleryCoordinatePacket
// * 설    명: 자주포 객체의 위치값 동기화 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 08
// ********************************************************************
typedef struct _SyncArtilleryCoordinatePacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StArtilleryPacketInfo strtArtilleryInfo;      //자주포정보
}StSyncArtilleryCoordinatePacket;

// ********************************************************************
// * 구조체명: _SyncMissileCoordinatePacket
// * 설    명: 미사일 객체의 위치값 동기화 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 05
// ********************************************************************
typedef struct _SyncMissileCoordinatePacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StMissilePacketInfo   strtMissileInfo;        //미사일정보
}StSyncMissileCoordinatePacket;

// ********************************************************************
// * 구조체명: _SyncArtilleryAttackedPacket
// * 설    명: 자주포 객체의 피격이벤트 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
typedef struct _SyncArtilleryAttackedPacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StArtilleryPacketInfo strtArtilleryInfo;      //자주포정보
}StSyncArtilleryAttackedPacket;

// ********************************************************************
// * 구조체명: _SyncMilitaryUnitAttackedPacket
// * 설    명: 부대 객체의 피격이벤트 패킷을 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
typedef struct _SyncMilitaryUnitAttackedPacket{
    StPacketHeader        strtHeader;
    int32_t               nUserClientFd;          //클라이언트를 식별하기 위함.
    StMilitaryPacketInfo  strtMilitaryInfo;       //부대 정보
}StSyncMilitaryUnitAttackedPacket;


// ********************************************************************
// * 구조체명: _SyncSimulationTimePacket
// * 설    명: simulation engine에서 보내주는 시간으로 클라이언트 동기화
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 29
// ********************************************************************
typedef struct _SyncSimulationTimePacket{
    StPacketHeader  strtHeader;
    int32_t         nCurrentSimulationTime;
}StSyncSimulationTimePacket;

// ********************************************************************
// * 구조체명: _RequestLoadMilitaryUnitInfoFromDBPacket
// * 설    명: database server에 부대정보를 요청하는 패킷
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
typedef struct _RequestLoadMilitaryUnitInfoFromDBPacket{
    StPacketHeader       strtHeader;
}StRequestLoadMilitaryUnitInfoFromDBPacket;

// ********************************************************************
// * 구조체명: _ResponseLoadMilitaryUnitInfoFromDBPacket
// * 설    명: database server가 부대정보를 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
typedef struct _ResponseLoadMilitaryUnitInfoFromDBPacket{
    StPacketHeader       strtHeader;
    StMilitaryPacketInfo strtMilitaryInfo;       //요청한 부대정보 
    int8_t               nIsFin;                 //마지막 부대정보인지 여부
}StResponseLoadMilitaryUnitInfoFromDBPacket;

// ********************************************************************
// * 구조체명: _Packet
// * 설    명: packet 구조를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 09
// ********************************************************************
typedef struct _Packet{
    struct _PacketHeader   strtHeader;
}StPacket;
//////////////////////////////////////////////////////////pragma pop
#pragma pack(pop)

// ********************************************************************
// * 클래스명: PacketUtil
// * 설    명: packet 파싱 및 생성을 담당하는 다양한 util함수를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 10
// ********************************************************************
class PacketUtil {
private:
    PacketUtil()  = delete;
    ~PacketUtil() = delete; 

private:
	static std::vector<std::string> ParsePacketHeaderInfo(const char_t* pPacket);
	static std::vector<std::string> ParsePacketBodyInfo(const char_t* pSBIPacket);

public:
    static int_t ParsePacketInfo(const char_t *pPacket);

    //로그인 및 초기 연결 관련 패킷
    static void CreateRequestLoginPacket(int32_t nClientFd, int32_t nUserClientFd, const StLoginRequestPacketData* pRecvPacket, StLoginRequestPacketData* pSendPacket);
    static void CreateResponseLoginPacket(int32_t nClinetFd,int32_t nUserClientFd, int8_t nUserAuth, int16_t nUserMilitaryId, int8_t nLoginState, StLoginResponsePacketData *pStrtPacket);
    static void CreateResponseLoginToClientPacket(int32_t nClientFd,const StLoginResponsePacketData* pRecvPacket,StLoginResponsePacketData* pSendPacket);
    static void CreateRequestHelloUdp(int32_t nClinetFd, struct _Packet* pStrtPacket);
    static void CreateResponseHelloUdp(int32_t nClinetFd, struct _Packet *pStrtPacket);

    //UpperServer와 통신 관련 패킷
    static void CreateRequestConnectPacket(int32_t nKey, struct _Packet *pStrtPacket);
    static void CreateResponseConnectPacket(int32_t nClinetFd, struct _Packet *pStrtPacket);
    static void CreateUpdateRoutingPathPacket(int32_t nUpperFd, int32_t nClientFd, StUpdateRoutingPath *pStrtPacket);
    
    //모의통제도구와 통신 관련 패킷
    static void CreateRequestSimulationStartPacket(int32_t nClinetFd, StStartRequestEnginePacket *pStrtPacket);
    static void CreateResponseSimulationStartPacket(StStartResponseEnginePacket *pStrtPacket);

    //전장상황도와 통신 관련 패킷
    static void CreateRequestLoadMilitaryUnitPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, StRequestLoadMilitaryUnitInfoPacket *pStrtPacket);
    static void CreateResponseLoadMilitaryUnitPacket(int32_t nClientFd, const StResponseLoadMilitaryUnitInfoPacket *pRecvPacket, StResponseLoadMilitaryUnitInfoPacket *pSendPacket);
    
    static void CreateRequestOrderInSallyPacket(int32_t nThisFd, int32_t nClientFd, int16_t nMilitaryId, int32_t nDstX, int32_t nDstY, StRequestOrderInSallyPacket *pStrtPacket);
    static void CreateResponseOrderInSallyPacket(int32_t nClientFd, const StResponseOrderInSallyPacket *pRecvPacket, StResponseOrderInSallyPacket *pSendPacket);
    static void CreateRequestOrderInAttackPacket(int32_t nThisFd, int32_t nClientFd, int16_t nArtilleyId, int32_t nDstX, int32_t nDstY, StRequestOrderInAttackPacket *pStrtPacket);
    static void CreateResponseOrderInAttackPacket(int32_t nClientFd, const StResponseOrderInAttackPacket *pRecvPacket, StResponseOrderInAttackPacket *pSendPacket);

    static void CreateSyncArtilleryCoordinatePacket(int32_t nClientFd, const StSyncArtilleryCoordinatePacket *pRecvPacket, StSyncArtilleryCoordinatePacket *pSendPacket);
    static void CreateSyncMissileCoordinatePacket(int32_t nClientFd, const StSyncMissileCoordinatePacket *pRecvPacket, StSyncMissileCoordinatePacket *pSendPacket);
    static void CreateSyncArtilleryAttackedPacket(int32_t nClientFd, const StSyncArtilleryAttackedPacket *pRecvPacket, StSyncArtilleryAttackedPacket *pSendPacket);
    static void CreateSyncMilitaryUnitAttackedPacket(int32_t nClientFd, const StSyncMilitaryUnitAttackedPacket *pRecvPacket, StSyncMilitaryUnitAttackedPacket *pSendPacket);
    static void CreateSyncSimulationTimePacket(StSyncSimulationTimePacket *pSendPacket, const StSyncSimulationTimePacket *pRecvPacket);

    //Database server와 통신
    static void CreateRequestLoadMilitaryUnitFromDB(StRequestLoadMilitaryUnitInfoFromDBPacket *pPacket);
    static void CreateResponseLoadMilitaryUnitFromDB(int8_t nIsFinFlag, StMilitaryUnitInfo *pStrtMilitaryUnit ,StResponseLoadMilitaryUnitInfoFromDBPacket *pPacket);
};
}