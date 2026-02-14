// ********************************************************************
// * 소스정의: MessageQueue.cpp 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ********************************************************************
#include "MessageQueue.h"
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
// * 함 수 명: MessageQueue
// * 설    명: MessageQueue 생성자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
MessageQueue::MessageQueue()
{

}

// ****************************************************************
// * 함 수 명: ~MessageQueue
// * 설    명: MessageQueue 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
MessageQueue::~MessageQueue()
{

}

// ****************************************************************
// * 함 수 명: Push
// * 설    명: 외부메세지를 넣고 waitpop하고 있는 친구에게 알려준다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
void MessageQueue::Push(const Message& message)
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_qExternalEventQueue.push(message);
    }
    m_CV.notify_one();
}

// ****************************************************************
// * 함 수 명: Pop
// * 설    명: 큐에 들어가 있는 메세지를 pop한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
bool_t MessageQueue::Pop(Message& out)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_qExternalEventQueue.empty()) return false;
    
    out = m_qExternalEventQueue.front();
    m_qExternalEventQueue.pop();
    return true;
}

// ****************************************************************
// * 함 수 명: WaitPop
// * 설    명: timeout이 걸릴때까지, 즉 다음 예정된 이벤트 시간이 되거나
// *           외부에서 사건이 발생할때까지 wait한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
bool_t MessageQueue::WaitPop(Message& out,TimeType timeout)
{
    if (timeout == Infinity) 
    {
        timeout = 60 * 60;  //1시간 기다림.
    }
    std::unique_lock<std::mutex> lock(m_Mutex);
    if (!m_CV.wait_for(lock, std::chrono::duration<double_t>(timeout), [&]() {
         return !m_qExternalEventQueue.empty(); 
        }))
        return false;

    out = m_qExternalEventQueue.front();
    m_qExternalEventQueue.pop();
    return true;
}

// ****************************************************************
// * 함 수 명: IsEmpty
// * 설    명: MessageQueue가 비었는지 확인한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 25
// ****************************************************************
bool_t MessageQueue::IsEmpty()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_qExternalEventQueue.empty();
}


}


