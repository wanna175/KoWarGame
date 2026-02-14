// ********************************************************************
// * 소스정의: DataTrsfModel.h
// * 설    명: 데이터 전달 처리 모델 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************

#pragma once

#include <string>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <atomic>

#include "TypeDef.h"
#include "MACRO.h"
#include "Singleton.h"
#include "CommonUtil.h"
#include "LogManager.h"
#include "PacketUtil.h"
#include "NetObjectInfo.h"
#include "SharedMemControl.h"
#include "Database.h"

using std::string;
using std::vector;
using std::map;
using std::set;
using std::unique_ptr;
using namespace IPCSHAREDMEM;
using namespace DATABASE;

namespace DATATRSFER {

    enum DATATRSF_START_MODE
	{
		STOP_MODE = -1,
		INTERNAL_MODE = 0,		// 데이터전달처리 서버 프로세스
		EXTERNAL_MODE = 1,		// 데이터전달처리 클라이언트 프로세스
	};

    // 클라이언트 타입
	enum CLIENT_TYPE
	{
		NONE_SYSTEM = 0,						// 초기값
		DATA_COMMUNICATION_SYSTEM = 11,			// 데이터전달처리
		BATTLEFIELD_DISPLAY_SYSTEM = 12,		// 전장상황도
		INTEGRATE_LOGIN_SYSTEM = 20,			// 통합로그인
        ENGINE_CONTROL_SYSTEM = 21,             // 모의엔진통제도구

        //////////////////////////////////////하위데이터전달처리일 경우
        UPPER_ENGINE_SERVER = 100,              // 시뮬레이션 엔진 서버
	};

// ********************************************************************
// * 클래스명: DataTrsfModel
// * 설    명: 데이터 전달 처리 모델 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 01
// ********************************************************************
    class DataTrsfModel : public Singleton<DataTrsfModel>{
    private:
        unique_ptr<LogManager>          m_pLogManager;              //로그매니져
        unique_ptr<SharedMemControl>    m_pShmManager;              //공유메모리 매니져
        unique_ptr<CDatabase>           m_pDatabaseManager;         //데이터베이스 매니져
    
        string                          m_IPADDRESS;                //ip addr

        int32_t                         m_nDataTrsfStartMode;       //mode number
        int_t                           m_nExternalServerSocket;    //외부와 통신할 TCP서버소켓
        int_t                           m_nExternalUdpSocket;       //외부와 통신할 UDP서버소켓
        int_t                           m_nUpperServerSocket;       //상위 서버 TCP
        int_t                           m_nUpperUdpSocket;          //상위 서버 UDP

        //////////////////////////////////////////////하위데이터전달처리일 경우
        int_t                           m_nUpperClientFd;           //상위 서버에서 처리되는 fd
        struct sockaddr_in              m_strtUpperUdpAddr;         //상위 서버 udp 정보
        //////////////////////////////////////////////////////////////////////

        NetObjectInfo                   m_clServerInfo;             //데이터 전달 server의 info(자기의 정보)

        int_t                           m_nEpollFD;                 //epoll fd

        //datatrsfer과 연결된 세션info
        //TODO : 밑에 두친구는 lock guard를 해야할 것 같은데... 좀더 생각해 볼 문제

        std::mutex                                       m_NetMapMutex;
        //std::mutex                                       m_NetRoutingMutex;
        map<int32_t, std::shared_ptr<NetObjectInfo>>     m_mNetInfo;   
        //map<int32_t, std::set<int32_t>>                  m_mNetRoutingInfo;         // ( 하위 전달처리 fd , 하위 전달처리가 들고 있는 클라이언트 fd set )
                                                                                    //불필요한 패킷을 전송하는 것을 막기위해 라우팅 정보를 저장한다.

        std::thread m_thrInternalRecvThread;
        std::atomic<bool_t> m_bIsExit;    
    private:
        void Initialize(const int32_t stnStartMode);
        void InternalProcessStart();
        ////////////////////// internal communication function
        void InitInternalProcess();
        void ConnectUpperServer();
        void InternalProcessSendRoutine(const char_t *pSendingData, size_t nSendDataLen);
        void InternalProcessReceiveRoutine();

        //////////////////////  external commuication function
        void InitExternalServerSocket(const int32_t stnStartMode);
        void InitExternalUdpSocket(const int32_t stnStartMode);
        void ExternalSocketProcessAcceptRoutine();
        void ExternalSocketProcessSendRoutine(int32_t nClientId);
        void ExternalSocketProcessSendtoRoutine(int32_t nClientFd, const char_t *pSendingData, size_t nSendDataLen);
        void ProcessSendtoRoutineToUpperServer(const char_t *pSendingData, size_t nSendDataLen);
        void ExternalSocketProcessRecvRoutine(const int_t nClientFd);
        void ExternalSocketProcessRecvfromRoutine(const int_t nUdpSocketFd);
        void StockPacketToClientSendingQueue(int32_t nClientFd, const char_t *pSendingData, size_t nSendDataLen);
        void ProcessTcpPacket(const char_t *pPacketBuf, const ssize_t nPacketSize, const int32_t nClientId);
        void ProcessUdpPacket(const char_t *pPacketBuf, const ssize_t nPacketSize, const sockaddr_in &strtClientAddr);
        void ProcessUpperTcpPacket(const char_t *pPacketBuf, const ssize_t nPacketSize);
        void ProcessUpperUdpPacket(const char_t *pPacketBuf, const ssize_t nPacketSize, const sockaddr_in &strtClientAddr);
        void SetSockOptValue(const int_t server_sock);

        ////////////////////////////////////////////////////////////network session control function 
        void RegisterNetObjectInfo(const std::shared_ptr<NetObjectInfo> clNetObjInfo);

        void ModifyNetObjectInfo(const int32_t &nNetObjectFd);
        void RemoveNetObjectInfo(const int_t &nNetObjectFd);

        void RegisterNetRoutingInfo(const int32_t nNetObjectFd, const int32_t nRoutingObjectFd);
        void RemoveNetRoutingInfo(const int_t &nNetObjectFd, const int32_t &nRoutingObjectFd);
        bool_t IsThereNetRoutingInfo(const int_t &nNetObjectFd, const int32_t &nRoutingObjectFd);

        ////////////////////////////////////////////////////////////Util   


        void PushFunThreadQueue(const std::function<void()> task);
        void PrintInfoLog(const string& info_log) const;
        void PrintErrorLog(const string& error_log) const;
        void PrintDebugLog(const string& debug_log) const;

    public:
        DataTrsfModel() = default;
        ~DataTrsfModel(){}
        void Start(const int32_t stnStartMode);
        void Finish(void);
    };
}