// ********************************************************************
// * 소스정의: ThreadPool.h
// * 설    명: 쓰레드 풀을 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
#pragma once

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
// * 클래스명: ThreadPool
// * 설    명: 쓰레드 풀 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 07
// ********************************************************************
class ThreadPool : public Singleton<ThreadPool> {
private:
    vector<thread>                m_vWorkerThreads;
    queue<function<void()>>       m_qTasks;
    mutex                         m_mutex;
    condition_variable            m_ConditionVariable;
    bool_t                        m_bExit;

private:
    void ExecuteThreadFunction();
    void InitThreadPool(size_t threadSize);
public:
    ThreadPool();
    ~ThreadPool();

    bool_t Enqueue(const function<void()> task);
    void waitAll();
    void ShutDown();
};