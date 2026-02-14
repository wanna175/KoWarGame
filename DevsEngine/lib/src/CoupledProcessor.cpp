// ********************************************************************
// * 소스정의: CoupledProcessor.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20 
// ********************************************************************

#include "Processor.h"
#include "AtomicProcessor.h"
#include "CoupledProcessor.h"
#include "Coupled.h"

#include "TypeDef.h"
#include "TimeType.h"
#include "MACRO.h"
#include <memory>
#include <algorithm>
#include <vector>


using namespace DEVSIM;

// ****************************************************************
// * 함 수 명: Initial
// * 설    명: 시뮬레이션 내에서 CoupledModel 를 관리할 실행기를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
void CoupledProcessor::Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value = 0)
{
    INFO("Start " + model->GetName() + " Processor Initialize - ");
    m_type = COUPLED;
    if (model->GetType() != m_type && upperProc->GetType() != COUPLED)
    {
        ERROR("Coupled Processor : Type Error");
        return;
    }
    Processor::Initial(upperProc,model,value); //m_tInit, m_tCurrent 설정
    auto pModel = std::static_pointer_cast<Coupled>(m_Model);
    
    m_tL = m_tInit;         //이전 이벤트 시간 초기값 설정
    m_tN = Infinity;        //다음 이벤트 시간 초기값 설정
    //상위 코디네이터에서 processor생성 후 Start신호로 초기화 해주도록 하자.
    Coupled::ModelVector vecModels;
    pModel->GetAllComponents(vecModels);

    for(const auto& subModel : vecModels)
    {
        if (subModel->GetType() == ATOMIC)
        {
            //생성자가 없구만...
            auto subProcessor = std::make_shared<AtomicProcessor>();
            m_SubProcessor[subModel->GetName()] = subProcessor;
            subProcessor->Initial(shared_from_this(),subModel, value);
        }
        else
        {
            auto subProcessor = std::make_shared<CoupledProcessor>();
            m_SubProcessor[subModel->GetName()] = subProcessor;
            subProcessor->Initial(shared_from_this(),subModel, value);
        }

    }

    //다음 이벤트 발생시각 초기화
    for (auto subProcessor : m_SubProcessor)
    {
        m_tN = std::min(subProcessor.second->GetTN(), m_tN);
    }

}

// ****************************************************************
// * 함 수 명: WhenRcvX
// * 설    명: CoupledProccessor용 (x,t)외부 이벤트 수신처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t CoupledProcessor::WhenRcvX(Message &msg) 
{
    m_tCurrent = msg.GetTime();     //메세지의 시점으로 현재 시각 갱신
    m_tL = m_tCurrent;              //마지막 이벤트 시간 설정
    
    shared_ptr<Coupled> pModel = std::static_pointer_cast<Coupled>(m_Model);
    
    m_nExpectedDoneChildCnt = 0;
    m_nReceiveDoneChildCnt  = 0;

    auto& coupling = pModel->GetThisCoupling();
    
    auto influenceList = coupling.GetInfluence(msg.GetPort());
    for (auto it = influenceList.first; it != influenceList.second; ++it) {
        auto& elem = it->second;
        auto targetProc = m_SubProcessor[elem->m_pToModel->GetName()];
        DEBUG(elem->m_pToModel->GetName());
        ///////////////////////////external message를 정의하고 보내야함
        m_nExpectedDoneChildCnt++;
        Message externalMessage(m_Model->GetName(), elem->m_strToPort, msg.GetTime(), MSG_TYPE::X_MSG);
        externalMessage.SetContent(msg.GetContent());
        externalMessage.SetDetail(msg.GetObjType(),msg.GetObjId(),msg.GetDestinationXpos(),msg.GetDestinationYpos());
        targetProc->WhenRcvX(externalMessage);
    }
    return true;

}

// ****************************************************************
// * 함 수 명: WhenRcvStar
// * 설    명: CoupledProccessor용 (*,t) 타임아웃 처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t CoupledProcessor::WhenRcvStar(Message &msg)
{
    m_tCurrent = msg.GetTime();     //메세지의 시점으로 현재 시각 갱신
    m_tL = m_tCurrent;

    shared_ptr<Coupled> pModel = std::static_pointer_cast<Coupled>(m_Model);
    
    //지금은 그냥 모든 프로세서를 순회하지만,, 이벤트 발생시간 순으로 처리한다면..
    //더 나을 듯. 아니면 다음 타임아웃이 TN인 서브들만 모아놓자.
    //TODO : for문 한번 도는 동안 tN이 되버린 애는 어떻게 처리???
    m_nExpectedDoneChildCnt=0;
    for (auto& subProcessor : m_SubProcessor) {
        //DEBUG("for문 돈다이~~~~~~`");
        if (subProcessor.second->GetTN() <= m_tN && subProcessor.second->GetTN()!=Infinity) {
            m_nExpectedDoneChildCnt++;              //done msg를 보내야 하는 child의 갯수증가.
            //DEBUG(m_Model->GetName() + " for문안에서 티앤 : "+std::to_string(subProcessor.second->GetTN()));
            Message msgStar(m_Model->GetName(), "", msg.GetTime(), MSG_TYPE::STAR_MSG);
            //TODO : star msg는 port가 굳이 필요없을까?
            subProcessor.second->WhenRcvStar(msgStar);
            break;
        }
    }
    return true;
} 

// ****************************************************************
// * 함 수 명: WhenRcvY
// * 설    명: CoupledProccessor용 (y,t) 외부 이벤트 수신처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t CoupledProcessor::WhenRcvY(Message &msg) 
{
    m_tCurrent = msg.GetTime();     //메세지의 시점으로 현재 시각 갱신
    m_tL = m_tCurrent;              //마지막 이벤트 시간 설정
    
    shared_ptr<Coupled> pModel = std::static_pointer_cast<Coupled>(m_Model);
    //DEBUG(m_Model->GetName() + " recived (y,t) from " + msg.GetSourceName()+" "+msg.GetPort());
    //잘못된 메세지 처리 : (y,t)는 내부에서 보내는 메세지여야함.
    string fromModelName = msg.GetSourceName();
    auto srcModel = pModel->GetComponent(fromModelName);
    if (!srcModel) return false;
    // 하위 모델이 다른 하위 모델, 또는 상위 모델에 보낼 coupling 정보
    // 
    auto it = pModel->GetChildCoupling().find(srcModel);
    if (it != pModel->GetChildCoupling().end()) {
        auto& coupling = it->second;
        auto influenceList = coupling->GetInfluence(msg.GetPort());
        for (auto iter = influenceList.first; iter != influenceList.second; ++iter) {
            auto& elem = iter->second;
            Message message;
            //Message outMessage(msg); TODO!!!!
            message.SetSourceName(pModel->GetName());
            message.SetContent(msg.GetContent());
            message.SetPort(elem->m_strToPort);
            message.SetTime(msg.GetTime());
            message.SetDetail(msg.GetObjType(),msg.GetObjId(),msg.GetDestinationXpos(),msg.GetDestinationYpos());

            //nullptr일 경우 IOC라고 볼수 있음.
            if (elem->m_pToModel == nullptr)
            {
                //DEBUG("외부로 (y,t) 전달" );
                message.SetType(MSG_TYPE::Y_MSG);
                m_vecOutMessages.push_back(message);
            }
            else
            {
                m_nExpectedDoneChildCnt++;
                //DEBUG(elem->m_pToModel->GetName() + "로 전달" );
                message.SetType(MSG_TYPE::X_MSG);
                auto dstProc = m_SubProcessor[elem->m_pToModel->GetName()];
                dstProc->WhenRcvX(message);
            }
        }
    }
    
    return true;

}

// ****************************************************************
// * 함 수 명: WhenRcvDone
// * 설    명: CoupledProccessor용 (Done,tN) 처리 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t CoupledProcessor::WhenRcvDone(Message &msg)
{
    m_nReceiveDoneChildCnt++;
    //DEBUG(m_Model->GetName()+" Recev : "+std::to_string(m_nReceiveDoneChildCnt)+ " Expect : "+std::to_string(m_nExpectedDoneChildCnt));

    shared_ptr<Coupled> pModel = std::static_pointer_cast<Coupled>(m_Model);

    if (m_nExpectedDoneChildCnt == m_nReceiveDoneChildCnt) 
    {
        m_tL = m_tN;    //이전 이벤트 발생시각
        m_tN = Infinity;
        //다음 이벤트 발생시각 구하기
        //m_tN = std::min(m_tN, msg.GetTime());
        for (auto subProcessor : m_SubProcessor)
        {
            m_tN = std::min(subProcessor.second->GetTN(), m_tN);
        }

        //DEBUG("모든 y 보낸다");
        SendUpperProcY();
        //DEBUG(m_Model->GetName() + " 모든 done 도착 업데이트 된 tN :"+ std::to_string(m_tN));
        //recived done이 모든 모델에서 도착한다면 상위에게 tN 을 보내야한다.
        bool_t ret_s = SendUpperProcDone();
        if (!ret_s)
        {
            ERROR("SendUpperProcDone() fail");
            return false;
        }
        m_nExpectedDoneChildCnt = 0;
        m_nReceiveDoneChildCnt = 0;
    }
    return true;
}

// ****************************************************************
// * 함 수 명: SendUpperProcDone
// * 설    명: UpperProcessor에게 (Done,tN) 메세지를 보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
bool_t CoupledProcessor::SendUpperProcDone()
{
    Message msgDone;
    msgDone.SetType(MSG_TYPE::DONE_MSG);
    msgDone.SetTime(m_tN);
    msgDone.SetSourceName(m_Model->GetName());

    //모든 Done msg를 받고선 다음 tN을 구한후에 보내야한다.... 어떻게 보낼지 고민...
    if (auto upperProcessor = m_UpperProcessor.lock()){
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
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t CoupledProcessor::SendUpperProcY()
{
    //한꺼번에 처리후 Done을 보낸 후에 upper에 전달하도록 하자.
    if (auto upperProcessor = m_UpperProcessor.lock()){
        for (auto& outMessage : m_vecOutMessages)
        {
            //DEBUG("CUR : "+m_Model->GetName()+" YMSG : "+ outMessage.GetContent() + " PORT : " + outMessage.GetPort());
            bool_t ret_w = upperProcessor->WhenRcvY(outMessage);
            if (!ret_w)
            {
                ERROR("SendUpperProcY() : WhenRcvY fail");
                return false;
            }
        }
    }
    m_vecOutMessages.clear();
    return true;
}

// ****************************************************************
// * 함 수 명: CleanUp
// * 설    명: 시뮬레이션이 끝나고 coupled모델 실행기 닫기
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
void CoupledProcessor::CleanUp()
{
    for (auto& [_, proc] : m_SubProcessor)
    {
        proc->CleanUp();
    }
    m_SubProcessor.clear();
    m_vecOutMessages.clear();
}

