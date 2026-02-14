// ********************************************************************
// * 소스정의: AtomicProcessor.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************

#include "AtomicProcessor.h"
#include "Atomic.h"

#include "TypeDef.h"
#include "TimeType.h"
#include "MACRO.h"
#include <memory>


using namespace DEVSIM;

// ****************************************************************
// * 함 수 명: Initial
// * 설    명: 시뮬레이션 내에서 AtomicModel 를 관리할 실행기를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void AtomicProcessor::Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value = 0)
{
    INFO("Start " + model->GetName() + " Processor Initialize - ");
    m_type = ATOMIC;
    if (model->GetType() != m_type && upperProc->GetType() != COUPLED)
    {
        ERROR("Atomic Processor : Type Error");
        return;
    }
    Processor::Initial(upperProc,model,value); //m_tInit, m_tCurrent 설정
    shared_ptr<Atomic> pModel = std::static_pointer_cast<Atomic>(m_Model);
    pModel->SetTimeE(0);                        //모델의 현재 상태에 머무른 시간 초기화
    pModel->SetTimeA(pModel->TimeAdvanceFn());  //모델의 다음 타임아웃 시간 계산
    m_tN = m_tInit + pModel->GetTimeA();        //다음 이벤트 시간 계산
    m_tL = m_tInit;                             //마지막 이벤트 시간


}

// ****************************************************************
// * 함 수 명: WhenRcvX
// * 설    명: AtomicProccessor용 (x,t)외부 이벤트 수신처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
bool_t AtomicProcessor::WhenRcvX(Message &msg) 
{
    m_tCurrent = msg.GetTime();     //메세지의 시점으로 현재 시각 갱신
    m_tL = m_tCurrent;              //마지막 이벤트 시간 설정
    
    auto pModel = std::static_pointer_cast<Atomic>(m_Model);
    
    /* 외부함수 천이 함수 호출
       외부함수 천이에서 메세지 내용에 따라 상태값과 tE,tA 가 결정됨 
    */
    if (!pModel->ExtTransFn(msg))
    {
        ERROR("WhenRcvX() - ExtTransFn() fail ");
        return false;
    }

    pModel->SetTimeA(pModel->TimeAdvanceFn());  //모델의 머무를 수 있는 시간 설정
    m_tN = m_tL + pModel->GetTimeA();   //다음 타임아웃 시간 결정

    //DEBUG("send done");
    SendUpperProcDone();

    return true;

}

// ****************************************************************
// * 함 수 명: WhenRcvStar
// * 설    명: AtomicProccessor용 (*,t) 타임아웃 처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
bool_t AtomicProcessor::WhenRcvStar(Message &msg)
{
    auto pModel = std::static_pointer_cast<Atomic>(m_Model);
    //DEBUG(m_Model->GetName() + " recived (*,t) state : " + pModel->GetCurrentState());
    m_tCurrent = msg.GetTime();     //메세지의 시점으로 현재 시각 갱신
    m_tL = m_tCurrent;

    SendUpperProcY();
    bool_t ret_i = pModel->IntTransFn();
    if (!ret_i)
    {
        ERROR("WhenRcvSTAR() - ExtTransFn() fail ");
        return false;
    }

    pModel->SetTimeA(pModel->TimeAdvanceFn());  //모델의 머무를 수 있는 시간 설정
    m_tN = m_tL + pModel->GetTimeA();   //다음 타임아웃 시간 결정

    //DEBUG("send done");
    SendUpperProcDone();

    return true;
} 

// ****************************************************************
// * 함 수 명: SendUpperProcDone
// * 설    명: UpperProcessor에게 (Done,tN) 메세지를 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
bool_t AtomicProcessor::SendUpperProcDone()
{
    Message msgDone;
    msgDone.SetType(MSG_TYPE::DONE_MSG);
    msgDone.SetTime(m_tN);
    msgDone.SetSourceName(m_Model->GetName());

    if (auto upperProcessor = m_UpperProcessor.lock())
    {
        bool_t ret_w = upperProcessor->WhenRcvDone(msgDone);
        if (!ret_w)
        {
            ERROR("SendUpperProcDone() : WhenRcvDone fail");
            return false;
        }
    }
    return true;
}

// ****************************************************************
// * 함 수 명: SendUpperProcY
// * 설    명: UpperProcessor에게 (y,t) 메세지를 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
bool_t AtomicProcessor::SendUpperProcY()
{
    shared_ptr<Atomic> pModel = std::static_pointer_cast<Atomic>(m_Model);
    //TODO : outMsg를 보내야 하는데... 이 모델과 연결된 port를 검사 후
    //실제로 outMsg메세지를 날려야함. 지금 그러한 로직이 빠져있음.
    //모든 상태 변환 함수를 쓰레드 풀로 실행한다면...?
    Message outMessage;
    outMessage.SetType(MSG_TYPE::Y_MSG);
    outMessage.SetSourceName(m_Model->GetName());

    bool_t ret_o = pModel->OutputFn(outMessage);
    if (!ret_o)
    {
        return false;
    }

    outMessage.SetTime(m_tCurrent);
    //상위 프로세서에게 출력 메세지 (y,t) 전달
    //상위 커플드 프로세서는 이 메세지를 받고 파싱하여 도착지를 정해줌.
    if (auto upperProcessor = m_UpperProcessor.lock())
    {
        upperProcessor->WhenRcvY(outMessage);
    }

    return true;
}


// ****************************************************************
// * 함 수 명: CleanUp
// * 설    명: 시뮬레이션이 끝나고 원자모델 실행기 닫기
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void AtomicProcessor::CleanUp()
{

}

