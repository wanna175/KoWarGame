using System;
using System.Runtime.InteropServices;

namespace NetworkPackets
{
    enum PACKET_TYPE : byte
    {
        ///////////////////////////////Client와의 통신
        REQUEST_LOGIN = 0,     //login request 
        RESPONSE_LOGIN = 1,     //login connect 
        DISCONNECTED_LOGIN_SYSTEM = 2,     //logout  
        HELLO_UDP = 3,     //helloUdp
        HELLO_UDP_RESPONSE = 4,     //helloUdp response

        ///////////////////////////////UpperServer와 통신
        REQUEST_CONNECT = 5,     //upperserver와 연결요청
        RESPONSE_CONNECT = 6,     //upperserver의 응답
        UPDATE_ROUTING_INFO = 7,  //upperserver에게 routing path info 전달

        ///////////////////////////////모의통제도구 와 통신
        REQUEST_SIM_START = 10,    //시뮬레이션 시작 요청
        REQUEST_SIM_STOP = 11,    //시뮬레이션 중단 요청


        RESPONSE_SIM_START = 12,    //시뮬레이션 시작 응답(broadcast)


        ///////////////////////////////전장상황도 와 통신
        REQUEST_MILITARY_UNIT_INFO = 20,   //부대정보 요청
        RESPONSE_MILITARY_UNIT_INFO = 21,   //부대정보 응답

        REQUEST_ORDER_IN_SALLY = 31,   //출격명령 요청
        RESPONSE_ORDER_IN_SALLY = 32,   //출격명령 응답
        REQUEST_ORDER_IN_ATTACK = 33,   //발사명령 요청
        RESPONSE_ORDER_IN_ATTACK = 34,   //발사명령 응답


        SYNC_ARTILLERY_COORDINATE = 45,   //자주포객체의 좌표 동기화(udp 통신)
        SYNC_MISSILE_COORDINATE = 46,   //미사일객체의 좌표 동기화(udp 통신)
        SYNC_ARTILLERY_ATTACKED = 47,
        SYNC_MILITARY_UNIT_ATTACKED = 48,

        ///////////////////////////////시뮬레이션 시간 키핑관련 통신
        SYNC_SIM_TIME = 50,   //시뮬레이션 엔진과 시간 동기화(broadcast)
    };

    enum USER_AUTHORITY : byte
    {
        USER = 0,     //user 
        ADMIN = 1,     //admin 
    };
    enum MILITARY_ID : ushort
    {
        ADMIN_ID  = 0,     //모든 부대아이디 .
        DEFAULT   = 1,     //default 부대 id
    };
    enum NETWORK_FLAG : byte
    {
        UDP = 0,     //udp
        TCP = 1,     //tcp
        BROADCAST_UDP = 2,
        BROADCAST_TCP = 3,


        ///////////////////////////load obj 관련 flag
        NONE          = 10,
        FIN           = 11,

        ///////////////////////////login 관련 flag
        LOGIN_SUCCESS     = 20,
        LOGIN_NO_FOUND_ID = 21,
        LOGIN_NO_FOUND_PW = 22,

        SERVER_ERROR      = 30,
    };



    // 공통 헤더
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StPacketHeader
    {
        public byte nType;          // uint8_t
        public int nID;             // int32_t
        public byte nFlagField;     // uint8_t
    }

    // 군부대 정보
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StMilitaryPacketInfo
    {
        public ushort nID;          // uint16_t
        public uint nTotalArtillery;// uint32_t
        public uint nXpos;          // uint32_t
        public uint nYpos;          // uint32_t
        public uint nUnitHealth;    // uint32_t
    }

    // 자주포 정보
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StArtilleryPacketInfo
    {
        public ushort nID;          // uint16_t
        public uint nTotalMissile;   // uint32_t
        public uint nXpos;          // uint32_t
        public uint nYpos;          // uint32_t
        public uint nUnitHealth;    // uint32_t
    }

    // 미사일 정보
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StMissilePacketInfo
    {
        public uint nID;            // uint32_t
        public uint nXpos;          // uint32_t
        public uint nYpos;          // uint32_t
    }


    // 로그인 요청
    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct StLoginRequestPacketData
    {
        public StPacketHeader strtHeader;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string nUserId;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string nUserPw;
    }

    // 로그인 응답
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StLoginResponsePacketData
    {
        public StPacketHeader strtHeader;
        public int nUserClientFD;
        public sbyte nUserAuthority;        // int8_t
        public short nUserMilitarySection;  // int16_t
    }

    // 엔진 시작 요청
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StStartRequestEnginePacket
    {
        public StPacketHeader strtHeader;
    }

    // 엔진 시작 응답
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StStartResponseEnginePacket
    {
        public StPacketHeader strtHeader;
    }

    // 군부대 정보 요청
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StRequestLoadMilitaryUnitInfoPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;          // int32_t
        public short nUserMilitarySection; // int16_t
    }

    // 군부대 정보 응답
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StResponseLoadMilitaryUnitInfoPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;                // int32_t
        public StMilitaryPacketInfo strtMilitaryInfo;
        public byte nIsFin;
    }

    // 출격 요청
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StRequestOrderInSallyPacket
    {
        public StPacketHeader strtHeader;
        public short nUserMilitarySection;   // int16_t
        public int nUserClientFd;            // int32_t
        public int nDstXPos;                 // int32_t
        public int nDstYPos;                 // int32_t
    }

    // 출격 응답
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StResponseOrderInSallyPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;                  // int32_t
        public StMilitaryPacketInfo strtMilitaryInfo;
        public StArtilleryPacketInfo strtArtilleryInfo;
    }

    // 미사일 발사 요청
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StRequestOrderInAttackPacket
    {
        public StPacketHeader strtHeader;
        public short nArtilleryId;           // int16_t
        public int nUserClientFd;            // int32_t
        public int nDstXPos;                 // int32_t
        public int nDstYPos;                 // int32_t
    }

    // 미사일 발사 응답
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StResponseOrderInAttackPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;                  // int32_t
        public StArtilleryPacketInfo strtArtilleryInfo;
        public StMissilePacketInfo strtMissileInfo;
    }


    // 자주포 좌표 동기화
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StSyncArtilleryCoordinatePacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;            // int32_t
        public StArtilleryPacketInfo strtArtilleryInfo;
    }

    // 미사일 좌표 동기화
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StSyncMissileCoordinatePacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;            // int32_t
        public StMissilePacketInfo strtMissileInfo;
    }

    // 자주포   attacked 동기화
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StSyncArtilleryAttackedPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;            // int32_t
        public StArtilleryPacketInfo strtArtilleryInfo;
    }

    // military unit attacked 동기화
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StSyncMilitaryUnitAttackedPacket
    {
        public StPacketHeader strtHeader;
        public int nUserClientFd;            // int32_t
        public StMilitaryPacketInfo strtMilitaryInfo;
    }

    // 시뮬레이션 시간 동기화
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StSyncSimulationTimePacket
    {
        public StPacketHeader strtHeader;
        public int nCurrentSimulationTime;   // int32_t
    }

    // 기본 패킷 (헤더만)
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct StPacket
    {
        public StPacketHeader strtHeader;
    }
}
