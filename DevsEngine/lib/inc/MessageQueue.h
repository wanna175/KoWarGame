// ********************************************************************
// * 헤더정의: MessageQueue.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ********************************************************************
#pragma once

#include "TypeDef.h"
#include "Message.h"
#include "MACRO.h"
#include "TimeType.h"

#include <queue>
#include <string>
#include <condition_variable>
#include <mutex>

using std::queue;
using std::condition_variable;

namespace DEVSIM{

// ****************************************************************
// * 클래스명: MessageQueue
// * 설    명: 외부 Event를 관리할 수 있는 MessageQueue를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
class MessageQueue
{
private:
    queue<Message> m_qExternalEventQueue;
    condition_variable m_CV;
    std::mutex m_Mutex;
    
public:
    MessageQueue();
    ~MessageQueue();

    void Push(const Message& message);
    bool_t Pop(Message& out);
    bool_t WaitPop(Message& out,TimeType timeout);
    bool_t IsEmpty();
    void NotifyAll(){ m_CV.notify_all();}
};

}


