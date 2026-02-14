// ********************************************************************
// * 소스정의: DEVSim.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ********************************************************************
#include "DEVSim.h"
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
// * 함 수 명: DEVSim
// * 설    명: DEVSim 생성자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
DEVSim::DEVSim()
{
    m_pCoordinator = std::make_shared<Coordinator>();
    m_tStartSimulationTime = 0;
    m_tEndSimulationTime = Infinity;
    m_tCurrent = m_tStartSimulationTime;
    m_tNextEvent = Infinity;

    m_bIsExit.store(true);
    m_pMessageQueue = std::make_unique<MessageQueue>();
}

// ****************************************************************
// * 함 수 명: ~DEVSim
// * 설    명: DEVSim 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
DEVSim::~DEVSim() 
{
    //TODO : 쓰레드 종료처리 해야댐
}

// ****************************************************************
// * 함 수 명: StartEngine
// * 설    명: 시뮬레이션 엔진을 시작한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::StartEngine(std::shared_ptr<Coupled> root)
{
    //m_pCoordinator->StartCoordinator(root, m_tStartSimulationTime);
    INFO("Start Simulation...");
    m_pCoordinator->Initial(nullptr, root, 0);
    m_tNextEvent = m_pCoordinator->GetTN();

    m_thrDevsimMainLoop = std::thread([this](){
        std::unique_lock<std::mutex> lk(m_Mutex); 
        m_cvStartSignal.wait(lk);
        this->StartDevsimMainLoop();
    });
    //외부에서 join 하도록 바꿔야하나..?
    //m_thrDevsimMainLoop.join();
    
}

// ****************************************************************
// * 함 수 명: SendStartSignalToEngine
// * 설    명: 시뮬레이션 시작신호를 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::SendStartSignalToEngine()
{
    std::unique_lock<std::mutex> lk(m_Mutex);
    m_bIsExit.store(false);
    m_cvStartSignal.notify_one();
}

// ****************************************************************
// * 함 수 명: IsRunningSimEngine
// * 설    명: 시뮬레이션 엔진이 running여부를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
bool_t DEVSim::IsRunningSimEngine(){ return !m_bIsExit.load();}

// ****************************************************************
// * 함 수 명: StartDevsimMainLoop
// * 설    명: DEVS의 메인 루프 process
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::StartDevsimMainLoop(){
    DEBUG("START SIMENGINE PACKET 받음");
    m_tStartSimulationTime = GetNowTime();
    while(!m_bIsExit.load())
    {
        m_tCurrent = GetNowTime();
        m_tNextEvent = m_pCoordinator->GetTN();
        TimeType waitTime = std::max(m_tNextEvent - m_tCurrent,(TimeType)0.0);

        Message extMessage;
        bool_t hasExternalEvent = m_pMessageQueue->WaitPop(extMessage,waitTime);
        if (hasExternalEvent)
        {
            extMessage.SetTime(GetNowTime());
            bool_t ret_w = m_pCoordinator->WhenRcvX(extMessage);
            if (!ret_w)
            {
                ERROR("WhenRcvX fail");
                break;
            }
            continue;
        }
        m_tCurrent = GetNowTime();
        if (m_tCurrent>=m_tNextEvent){
            Message starMessage("", "", m_tCurrent, MSG_TYPE::STAR_MSG);
            bool_t ret_w = m_pCoordinator->WhenRcvStar(starMessage);
            if (!ret_w)
            {
                ERROR("WhenRcvStar fail");
                break;
            }
        }
        Message outMessage;
        while (m_pCoordinator->GetOutMessageFromVector(outMessage))
        {
            DEBUG("external로 보냄");
            SendOutputToExternal(outMessage);
        }
    }
}

// ****************************************************************
// * 함 수 명: InjectExternalEvent
// * 설    명: 시뮬레이션 엔진 외부에서 이벤트를 주입한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::InjectExternalEvent(const Message& message)
{
    m_pMessageQueue->Push(message);
}

// ****************************************************************
// * 함 수 명: StopEngine
// * 설    명: 시뮬레이션 엔진을 중지한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::StopEngine()
{
    m_bIsExit.store(true);
    //TODO : queue에 있는 것 다 빼내야 하는지..?
    m_pMessageQueue->NotifyAll();
    //m_thrDevsimMainLoop.join();
    m_pCoordinator->CleanUp();
}

// ****************************************************************
// * 함 수 명: GetNowTime
// * 설    명: 시뮬레이션 엔진의 현재 실행 시간을 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
TimeType DEVSim::GetNowTime() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return duration<double_t>(now.time_since_epoch()).count() - m_tStartSimulationTime;
}

// ****************************************************************
// * 함 수 명: JoinEngineThread
// * 설    명: DevsimMainLoop Thread를 조인한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::JoinEngineThread(){
    //DEBUG("egineThread join ");
    m_thrDevsimMainLoop.join();
}

// ****************************************************************
// * 함 수 명: SetOutputCallback
// * 설    명: 시뮬레이션이 반환하는 출력메세지를 받고 행동할 콜백함수를 등록한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::SetOutputCallback(std::function<void(const Message&)> callback)
{
    m_outputCallback = callback;
}

// ****************************************************************
// * 함 수 명: GetNowTime
// * 설    명: 외부출력메세지를 받고 등록된 콜백함수를 호출한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 24
// ****************************************************************
void DEVSim::SendOutputToExternal(const Message& out)
{
    if (m_outputCallback)
    {
        m_outputCallback(out);
    }
}

}
