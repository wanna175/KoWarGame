// ********************************************************************
// * 헤더정의: DEVSim.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ********************************************************************
#pragma once

#include "Coordinator.h"
#include "Coupled.h"
#include "MessageQueue.h"
#include "TimeType.h"
#include "MACRO.h"
#include "TypeDef.h"

#include <condition_variable>
#include <thread>
#include <functional>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <atomic>


namespace DEVSIM{

// ****************************************************************
// * 클래스명: DEVSim
// * 설    명: DEVS엔진 실행하고 옵션을 관리하는 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
class DEVSim {
private:
    std::shared_ptr<Coordinator> m_pCoordinator;
    TimeType m_tStartSimulationTime;                    //시뮬레이션 시작시간
    TimeType m_tEndSimulationTime;
    TimeType m_tCurrent;
    TimeType m_tNextEvent;

    std::thread m_thrDevsimMainLoop;
    std::atomic<bool_t> m_bIsExit;

    std::condition_variable m_cvStartSignal;
    std::mutex m_Mutex;
    /////////////////////////////////////////////외부입력을 위한 변수
    std::unique_ptr<MessageQueue> m_pMessageQueue;

    std::function<void(const Message&)> m_outputCallback;


    
public:
    DEVSim();
    virtual ~DEVSim();

    void StartEngine(std::shared_ptr<Coupled> root);

    void SendStartSignalToEngine();
    bool_t IsRunningSimEngine();
    void StartDevsimMainLoop();
    void InjectExternalEvent(const Message& message);
    void StopEngine();

    TimeType GetNowTime();
    void JoinEngineThread();
    void SetOutputCallback(std::function<void(const Message&)> callback);
    void SendOutputToExternal(const Message& out);

};

}