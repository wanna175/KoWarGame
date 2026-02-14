// ********************************************************************
// * 소스정의: DataTrsfModel.h
// * 설    명: 윈도우용 데이터 전달 처리 모델 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 20
// ********************************************************************
#pragma once

#include "pch.h"
#include "PacketUtil.h"

//#pragma comment(lib, "ws2_32.lib")

namespace DATATRSFER {

// ********************************************************************
// * 클래스명: DataTrsfModel
// * 설    명: 윈도우버젼 데이터 전달 처리 모델 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 20
// ********************************************************************
	class DataTrsfModel {
	private:
		string                          m_strIpAddress;             // server ip addr,port ...
		string							m_strPortNo;
		string							m_strUdpPortNo;
		SOCKET                          m_nExternalServerSocket;    // 외부와 통신할 TCP서버소켓
		SOCKET                          m_nExternalUdpSocket;       // 외부와 통신할 UDP서버소켓

		SOCKET							m_nInternalLoopbackPipeS;	
		SOCKET							m_nInternalLoopbackSocket;	//안전한 select 종료처리를 위한 루프백 소켓
		SOCKET							m_nInternalLoopbackPipeC;

		int16_t                         m_nUserMilitarySection;       //사용자의 military unit id
		int32_t                         m_nIdentifcationFd;          // server에서 사용하는 clinet fd
																    // packet 전송에 필요.. 식별자.

		struct sockaddr_in				m_strtServerUdpAddr;		// 서버 udp정보
		
		struct fd_set					m_strtReadSet;				// select 용 read fd set

		WSADATA							m_WinSockInfo;				// winsock information
																	// TODO : 필요시 확인한다.
		
		std::atomic<bool_t>				m_bIsExit;					//network module 중지인지?
		std::thread                     m_thrExternalRecvThread;			//network module 실행 쓰레드
		std::thread						m_thrExternalSendThread;

		///////////////////////////////////////////////////////sending queue 관련
		deque<vector<char_t>>   m_deqSendQueue;  // 전송 큐 
		size_t                  m_nSentBytes;    // 전송큐 가장 앞 데이터 보낸 데이터 수
		std::mutex              m_mutexQueue;    // mutex 전송 큐


		//////////////////////////////////////////////////////// dll을 쓰는 측(유니티)과 통신할 큐
		deque<vector<char_t>>   m_deqRecvQueue;	  // 수신 큐
		std::mutex				m_mutexRecvQueue;

	private:
		void Initialize(const string strIpAddress, const string strPortNo, const string strUdpPortNo);

		bool_t InitializeLoopback();

		bool_t ConnectServer();

		void ExternalSocketProcessSendRoutine();
		void ExternalSocketProcessSendtoRoutine(const char_t* pSendingData, size_t nSendDataLen);
		void ExternalSocketProcessRecvRoutine();
		void ExternalSocketProcessRecvfromRoutine();
		void StockPacketToServerSendingQueue(const char_t* pSendingData, size_t nSendDataLen);
		void ProcessTcpPacket(const char_t* pPacketBuf, const int_t nPacketSize);
		void ProcessUdpPacket(const char_t* pPacketBuf, const int_t nPacketSize, const sockaddr_in& strtClientAddr);
		void SetSockOptValue(const SOCKET  server_sock, const bool_t isTcp);

		void ProcessSelectLoop();
		////////////////////////////////////////////////////////////Util  
		void EnqueueSendQueue(const char_t* cSendData, size_t nDataLen) { m_deqSendQueue.emplace_back(cSendData, cSendData + nDataLen); }
		bool_t HasPendingData() const { return !m_deqSendQueue.empty(); }
		void PopSendQueue() { m_deqSendQueue.pop_front(); }
		vector<char_t>& PeekFrontQueue() { return m_deqSendQueue.front(); }

		void PushFunThreadQueue(const std::function<void()> task);
		void PrintInfoLog(const string& info_log) const;
		void PrintErrorLog(const string& error_log) const;
		void PrintDebugLog(const string& debug_log) const;

		void PushPacketToRecvQueue(const char_t* cSendData, size_t nDataLen);
		void PushLoginMessageToServerProcess(const int8_t nPacketType, const char_t* strUserId, const char_t* strUserPW);
		void PushMessageToServerProcess(const int8_t nPacketType, const int32_t xpos, const int32_t ypos,const int16_t nRequestObjId);

	public:
		DataTrsfModel() = default;
		~DataTrsfModel() {}
		bool_t Start(const string strIpAddress, const string strPortNo, const string strUdpPortNo);
		void Finish(void);

	public:
		/////////////////////////////////////////////////////////////Unity program과 통신할 목적의 함수
		////////recv function
		int_t PopPacketFromRecvQueue(char_t* czOutBuffer, int_t nBufSize);
		////////send function
		void PushMessageToServer(const int8_t nPacketType,const int32_t xpos, const int32_t ypos,const int16_t nId);
		void PushLoginMessageToServer(const int8_t nPacketType, const char_t* strUserId, const char_t* strUserPW);
	};
}
