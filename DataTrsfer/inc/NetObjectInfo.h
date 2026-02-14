// ********************************************************************
// * 소스정의: NetObjectInfo.h
// * 설    명: 세션관리에 필요한 세션클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 09
// ********************************************************************
#pragma once

#include <ctime>
#include <string>
#include <deque>
#include <vector>
#include <set>
#include <netinet/in.h>
#include <mutex>

#include "TypeDef.h"
#include "MACRO.h"


using std::string;
using std::deque;
using std::vector;
using std::set;

namespace DATATRSFER {

// 클라이언트 상태
enum CLIENT_STATE
{
	CONNECT_WAIT = 0,				// 접속대기중
	CONNECT      = 1,			    // 접속중
	
};
// ********************************************************************
// * 클래스명: NetObjectInfo
// * 설    명: 네트워크에 연결된 객체(세션 정보)를 저장하는 클래스를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 09
// ********************************************************************
class NetObjectInfo {
public:
    string                  m_strIpAddress;             //tcp클라이언트 소켓
    int32_t                 m_nTcpSocket;               // 사용자 식별 ID
    sockaddr_in             m_strtUdpAddr;              //udp 연결용 구조체
    int_t                   m_nProgramType;             //연결된 클라이언트의 종류              
    int_t                   m_nConnStatus;              //client state
	time_t                  m_LoginTIME;	            // 로그인 시간

public:
    // upper server 용 routing info
    set<int32_t>            m_setRoutingInfo;
    std::mutex              m_mutexSet;       //mutex routing info
    //lock free 한 다른 라이브러리로 업그레이드 고려!! 현재는 그냥 lock걸기

    deque<vector<char_t>>   m_deqSendQueue;  //전송 큐 
    size_t                  m_nSentBytes;    //전송큐 가장 앞 데이터 보낸 데이터 수
    std::mutex              m_mutexQueue;    //mutex 전송 큐
    
public:
    NetObjectInfo() = default;
    ~NetObjectInfo() { DEBUG(" NetObjectInfo destructor call : "+std::to_string(m_nTcpSocket));}

    void EnqueueSendQueue(const char_t* cSendData, size_t nDataLen) { m_deqSendQueue.emplace_back(cSendData, cSendData + nDataLen); }
    bool_t HasPendingData() const { return !m_deqSendQueue.empty();}
    void PopSendQueue() { m_deqSendQueue.pop_front(); }
    vector<char_t>& PeekFrontQueue() { return m_deqSendQueue.front();}
    
    void InsertRoutingSet(const int32_t clientFd)
    {
        std::lock_guard<std::mutex> lock(m_mutexSet);
        if (m_setRoutingInfo.find(clientFd) != m_setRoutingInfo.end()) 
        {
            m_setRoutingInfo.insert(clientFd);
        }
    }
    bool_t HasRoutingInfo(const int32_t clientFd)
    {
        std::lock_guard<std::mutex> lock(m_mutexSet);
        if (m_setRoutingInfo.find(clientFd) != m_setRoutingInfo.end()) 
        {
            return true;
        }
        return false;
    }
    void RemoveRoutingSet(const int32_t clientFd)
    {
        std::lock_guard<std::mutex> lock(m_mutexSet);
        if (m_setRoutingInfo.find(clientFd) != m_setRoutingInfo.end()) 
        {
            m_setRoutingInfo.erase(clientFd);
        }
    }

};

}