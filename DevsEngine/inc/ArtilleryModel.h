// ********************************************************************
// * 소스정의: ArtilleryModel.h
// * 설    명: 자주포의 행동을 나타내는 아토믹 모델정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

#include "Atomic.h"
#include "StateVariable.h"
#include "TimeType.h"
#include "Message.h"
#include "ModelStruct.h"
#include "Global.h"

#include <string>
#include "MACRO.h"

using std::to_string;

using namespace DEVSIM;
using namespace ModelConfig::Content;
using namespace ModelConfig::Port;
using namespace ModelConfig::State;

// ********************************************************************
// * 클래스명: ArtilleryModel
// * 설    명: 자주포의 행동을 나타내는 아토믹 모델정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class ArtilleryModel : public Atomic
{
private:
    std::shared_ptr<CArtillery> m_pArtilleryInfo;
public:
// ********************************************************************
// * 함 수 명: ArtilleyModel
// * 설    명: ArtilleyModel 생성자. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    ArtilleryModel(const string& name,std::shared_ptr<CArtillery> pArtillery) 
                : Atomic(name, StateVar({S_art_WAIT_IN,S_art_WAIT_OUT,S_art_MOVE,S_art_ATTACK,S_art_ATTACKED,S_art_DESTROY }))
    {
        SetTimeA(Infinity);
        m_pArtilleryInfo = pArtillery;
    }

 // ****************************************************************
// * 함 수 명: ExtTransFn
// * 설    명: External State Transition Function (delta_{ext})
// *           외부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************   
    bool_t ExtTransFn(const Message& msg) override {

        if (GetCurrentState() == S_art_DESTROY)
        {
            return false;
        }
        
        //출격명령도착
        if (msg.GetContent() == C_art_ORDER_IN_SALLY)
        {
            DEBUG("GET ORDER IN SALLY" + std::to_string(msg.GetTime())+" : " + GetName());
            bool_t ret_s = SetCurrentState(S_art_MOVE);
            if (!ret_s)
            {
                ERROR("SetCurrentState() fail");
                return false;
            }
            SetTimeE(0);
            SetTimeA(1.0);
        }
        //피격시
        else if (msg.GetContent() == C_art_ATTACKED) 
        {
            DEBUG("GET ATTACKED "+ std::to_string(msg.GetTime())+" : " + GetName());
            //현재는 그냥 간단히 20 데미지라고 하자..
            m_pArtilleryInfo->m_nUnitHealth-=50;
            bool_t ret_s = m_pArtilleryInfo->m_nUnitHealth > 0 ? SetCurrentState(S_art_ATTACKED) : SetCurrentState(S_art_DESTROY);
            if (!ret_s)
            {
                ERROR("SetCurrentState() fail");
                return false;
            }

            SetTimeE(0);
            SetTimeA(0.0);  
        }
        //미사일 발사 명령시
        else if (msg.GetContent() == C_art_ORDER_IN_ATTACK)
        {
            DEBUG("GET ATTACK "+ std::to_string(msg.GetTime())+" : " + GetName());

            //destination setting...
            m_pArtilleryInfo->SetDestinationMissile(msg.GetDestinationXpos(),msg.GetDestinationYpos());
            bool_t ret_s = SetCurrentState(S_art_ATTACK);
            if (!ret_s)
            {
                ERROR("SetCurrentState() fail");
                return false;
            }
            SetTimeE(0);
            SetTimeA(1.0);
        }
        return true;
    }

// ****************************************************************
// * 함 수 명: IntTransFn
// * 설    명: Internal State Transition Function (delta_{int})
// *           내부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    bool_t IntTransFn() override {
        if(GetCurrentState() == S_art_ATTACK)
        {
            SetCurrentState(S_art_WAIT_OUT);
            SetTimeE(0);
            SetTimeA(Infinity);
        }
        else if(GetCurrentState() == S_art_MOVE)
        {
            DEBUG("Has moving.... : " + GetName());
            bool_t bIsArrive = m_pArtilleryInfo->IsArriveAtDestination();
            if (bIsArrive)
            {
                //DEBUG("WAIT 아웃 상태로....");
                SetCurrentState(S_art_WAIT_OUT);
                SetTimeE(0);
                SetTimeA(Infinity);
            }
            else
            {
                //DEBUG("계속 움직인다.....");
                SetTimeE(0);
                SetTimeA(1.0);
            }
        }
        else if(GetCurrentState() == S_art_ATTACKED)
        {
            SetCurrentState(S_art_WAIT_OUT);
            SetTimeE(0);
            SetTimeA(Infinity);
        }
        else if(GetCurrentState() == S_art_DESTROY)
        {
            //SetCurrentState("WAIT");
            SetTimeE(0);
            SetTimeA(Infinity);
        }
        else
        {
            return false;
        }

        return true;
    }

// ****************************************************************
// * 함 수 명: OutputFn
// * 설    명: Output Function (lambda)
// *           출력함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    bool_t OutputFn(Message& msg) override {
        if(GetCurrentState() == S_art_MOVE)
        {
            m_pArtilleryInfo->SetCurrentPos(1.0);
            msg.SetPort(P_art_MOVED_OUTPUT);
            msg.SetContent(C_art_MOVED);
            msg.SetDetail(ObjectID::ARTILLERY,m_pArtilleryInfo->m_nID,m_pArtilleryInfo->m_nXPos,m_pArtilleryInfo->m_nYPos);
        }
        else if(GetCurrentState() == S_art_ATTACK)
        {
            shared_ptr<CMissile> pMissile = m_pArtilleryInfo->GetMissileFromQueue();
            if (pMissile != nullptr)
            {
                //연결된 미사일에게 명령을 내려야 한다.
                msg.SetPort(P_art_OUT+to_string(pMissile->m_nID));
                msg.SetContent(C_art_ORDER_IN_ATTACK);

                (m_pArtilleryInfo->m_nTotalMissile)--;
            }
            else
            {
                //TODO : 사용자에게 잘못된 명령이라고 보내줘야함.
            }
        
        }
        else if(GetCurrentState() == S_art_ATTACKED)
        {
            msg.SetPort(P_art_ATTACKED_OUTPUT);
            msg.SetContent(C_art_ATTACKED);
            msg.SetDetail(ObjectID::ARTILLERY,m_pArtilleryInfo->m_nID);
        }
        else if(GetCurrentState() == S_art_DESTROY)
        {
            msg.SetPort(P_art_DESTROY_OUTPUT);
            msg.SetContent(C_art_DESTROY);
            msg.SetDetail(ObjectID::ARTILLERY,m_pArtilleryInfo->m_nID);
        }
        else
        {
            return false;
        }
        return true;
    }

// ****************************************************************
// * 함 수 명: TimeAdvanceFn
// * 설    명: Time Advance Function ( ta )
// *           시간 진행함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    TimeType TimeAdvanceFn() override {
        return GetTimeA();
    }
};