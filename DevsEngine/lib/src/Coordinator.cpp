// ********************************************************************
// * 소스정의: Coordinator.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 21
// ********************************************************************


#include "Coordinator.h"
#include "AtomicProcessor.h"
#include "CoupledProcessor.h"
#include "Coupled.h"

#include "TypeDef.h"
#include "TimeType.h"
#include "MACRO.h"
#include <memory>
#include <algorithm>
#include <string>


using namespace DEVSIM;

// ****************************************************************
// * 함 수 명: Initial
// * 설    명: Coordinator 초기화
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 21
// ****************************************************************
void Coordinator::Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value = 0)
{
    INFO("Coordinator Initialize...");
    //upperProcessor는 항상 nullptr
    if (upperProc !=nullptr)
    {
        ERROR("Coordinator Initial() fail");
        return;
    }
    Processor::Initial(upperProc,model,value); //m_tInit, m_tCurrent 설정
    auto pModel = std::static_pointer_cast<Coupled>(m_Model);
    
    m_type = COUPLED;
    m_tL = m_tInit;         //이전 이벤트 시간 초기값 설정
    m_tN = Infinity;        //다음 이벤트 시간 초기값 설정
    m_rootProcessor = std::make_shared<CoupledProcessor>();
    //Coordinator은 쉐어드 포인터로 선언되어야 한다.
    m_rootProcessor->Initial(shared_from_this(), model, value);
    m_tN = m_rootProcessor->GetTN();
    //다음 이벤트 발생시각 초기화
    INFO("NextEventTime : " + std::to_string(m_tN));
    INFO("Coordinator Initialize end");
}

// ****************************************************************
// * 함 수 명: WhenRcvX
// * 설    명: (x,t)외부 이벤트 수신처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 21
// ****************************************************************
bool_t Coordinator::WhenRcvX(Message &msg) 
{
    m_tCurrent = msg.GetTime();  //현재시간 갱신
    m_tL = m_tCurrent;        // 이전 이벤트 시간 갱신

    Message externalMessage(msg);
    //DEBUG("msgX Send" + std::to_string(externalMessage.GetTime())+" "+externalMessage.GetContent()+" "+std::to_string(externalMessage.GetObjId()));
    
    bool_t ret_w = m_rootProcessor->WhenRcvX(externalMessage);
    if (!ret_w)
    {
        ERROR("WhenRcvX fail");
        return false;
    };

    m_tN = m_rootProcessor->GetTN();  
    return true;
}

// ****************************************************************
// * 함 수 명: WhenRcvStar
// * 설    명: CoordinatorProccessor용 (*,t) 타임아웃 처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coordinator::WhenRcvStar(Message &msg)
{
    m_tCurrent = msg.GetTime();
    m_tL = m_tN;        // 이전 이벤트 시간 갱신

    Message timeoutMessage("", "", m_tCurrent, MSG_TYPE::STAR_MSG);
    //DEBUG("Event timeout occur : " + std::to_string(m_tCurrent));
    
    bool_t ret_w = m_rootProcessor->WhenRcvStar(timeoutMessage);
    if (!ret_w)
    {
        ERROR("WhenRcvStar fail");
        return false;
    };

    m_tN = m_rootProcessor->GetTN();  
    return true;
} 

// ****************************************************************
// * 함 수 명: WhenRcvY
// * 설    명: CoordinatorProccessor용 (y,t) 외부 이벤트 수신처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coordinator::WhenRcvY(Message &msg) 
{
    //DEBUG("coordinator에 out msg도착 : "+msg.GetContent());
    m_vecOutMessages.push(msg);
    return true;
}

// ****************************************************************
// * 함 수 명: WhenRcvDone
// * 설    명: CoordinatorProccessor용 (Done,tN) 처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coordinator::WhenRcvDone(Message &msg)
{
    m_tN = m_rootProcessor->GetTN();
    return true;
}

// ****************************************************************
// * 함 수 명: CleanUp
// * 설    명: 시뮬레이션이 끝나고 root모델 실행기 닫기
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 21
// ****************************************************************
void Coordinator::CleanUp()
{
    m_rootProcessor->CleanUp();
}

// ****************************************************************
// * 함 수 명: GetOutMessageFromVector
// * 설    명: 도착한 OutMessage를 queue가 빌때까지 실행한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 21
// ****************************************************************
bool_t Coordinator::GetOutMessageFromVector(Message &outMessage)
{
    if (m_vecOutMessages.size() == 0) return false;

    outMessage = m_vecOutMessages.front();
    m_vecOutMessages.pop();
    return true;
}