// ********************************************************************
// * 소스정의: ThreadPool.cpp
// * 설    명: 쓰레드 풀을 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
#include "ThreadPool.h"
#include "TypeDef.h"
#include "MACRO.h"
#include "Singleton.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

using std::vector;
using std::queue;
using std::condition_variable;
using std::thread;
using std::mutex;
using std::function;

// ********************************************************************
// * 함 수 명: ThreadPool
// * 설    명: 쓰레드 풀 클래스 생성자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
ThreadPool::ThreadPool()
{
    size_t threadSize = std::thread::hardware_concurrency();
    InitThreadPool(threadSize);
}

// ********************************************************************
// * 함 수 명: ~ThreadPool
// * 설    명: 쓰레드 풀 클래스 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<mutex> lock(m_mutex);
        m_bExit = true;
    }
    m_ConditionVariable.notify_all();

    for (auto& thread : m_vWorkerThreads)
    {
        thread.join();
    }
}

// ********************************************************************
// * 함 수 명: InitThreadPool
// * 설    명: 쓰레드 풀을 초기화 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
void ThreadPool::InitThreadPool(size_t threadSize) 
{
    m_bExit = false;
    
    for (size_t i = 0; i < threadSize; ++i) 
    {
        //threadSize만큼 쓰레드 생성후, task가 올때까지 기다린다.
        //TODO:람다식을 안쓰고 깔끔하게 실행될까??
        m_vWorkerThreads.emplace_back(&ThreadPool::ExecuteThreadFunction, this);
    }
}

// ********************************************************************
// * 함 수 명: ExecuteThreadFunction
// * 설    명: task를 queue에서 가져와서 실행하는 쓰레드함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
void ThreadPool::ExecuteThreadFunction() {
    while(true) 
    {
        function<void()> task;
        {
            std::unique_lock<mutex> lock(m_mutex);
            m_ConditionVariable.wait(lock,[this]() {
                return m_bExit || !m_qTasks.empty();
            });

            if (m_bExit && m_qTasks.empty()) return;

            task = std::move(m_qTasks.front());
            m_qTasks.pop();
        }
        task();
    }

}

// ********************************************************************
// * 함 수 명: Enqueue
// * 설    명: 쓰레드 풀에 작업을 하나 할당한다. 
// *           성공시 true, 실패시 false 반환  
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
bool_t ThreadPool::Enqueue(const function<void()> task)
{
    {
        std::unique_lock<mutex> lock(m_mutex);
        if (m_bExit) {
            ERROR(" - Thread pool Enqueue on Stopped ");
            return false;
        }
        m_qTasks.emplace(std::move(task));
    }
    m_ConditionVariable.notify_one();
    return true;
}

// ********************************************************************
// * 함 수 명: ShutDown
// * 설    명: 쓰레드 풀 작업쓰레드 멈추기
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 15
// ********************************************************************
void ThreadPool::ShutDown()
{
    {
        std::unique_lock<mutex> lock(m_mutex);
        m_bExit = true;
    }
    m_ConditionVariable.notify_all();

    for (auto& thread : m_vWorkerThreads)
    {
        thread.join();
    }
}