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
// * 클래스명: MissileModel
// * 설    명: 미사일의 행동을 나타내는 아토믹 모델정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
class MissileModel : public Atomic
{
private:
    std::shared_ptr<CMissile> m_pMissileInfo;
public:
// ********************************************************************
// * 함 수 명: MissileModel
// * 설    명: MissileModel 생성자. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
    MissileModel(const string& name,std::shared_ptr<CMissile> pMissile) 
                : Atomic(name, StateVar({S_mis_WAIT, S_mis_MOVE, S_mis_DESTROY }))
    {
        SetTimeA(Infinity);
        m_pMissileInfo = pMissile;
    }

// ****************************************************************
// * 함 수 명: ExtTransFn
// * 설    명: External State Transition Function (delta_{ext})
// *           외부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ****************************************************************   
    bool_t ExtTransFn(const Message& msg) override {

        if (GetCurrentState() == S_mis_DESTROY)
        {
            return false;
        }
        
        //발사명령도착
        if (msg.GetContent() == C_mis_ORDER_IN_ATTACK)
        {
            DEBUG("GET ORDER IN Attack" + std::to_string(msg.GetTime())+" : " + GetName());
            bool_t ret_s = SetCurrentState(S_mis_MOVE);
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
// * 작성날짜: 2025. 09. 04
// ****************************************************************
    bool_t IntTransFn() override {

        if(GetCurrentState() == S_mis_MOVE)
        {
            bool_t bIsArrive = m_pMissileInfo->IsArriveAtDestination();
            if (bIsArrive)
            {
                //DEBUG("Bomb!!!");
                SetCurrentState(S_mis_DESTROY);
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
        else if(GetCurrentState() == S_mis_DESTROY)
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
// * 작성날짜: 2025. 09. 04
// ****************************************************************
    bool_t OutputFn(Message& msg) override {
        if(GetCurrentState() == S_mis_MOVE)
        {
            m_pMissileInfo->SetCurrentPos(1.0);
            msg.SetPort(P_mis_MOVED_OUTPUT);
            msg.SetContent(C_mis_MOVED);
            msg.SetDetail(ObjectID::MISSILE, m_pMissileInfo->m_nID,m_pMissileInfo->m_nXPos,m_pMissileInfo->m_nYPos);
        }
        else if(GetCurrentState() == S_mis_DESTROY)
        {
            DEBUG("destroy missile... but is it call?");
            msg.SetPort(P_mis_DESTROY_OUTPUT);
            msg.SetContent(C_mis_DESTROY);
            msg.SetDetail(ObjectID::MISSILE, m_pMissileInfo->m_nID,m_pMissileInfo->m_nXPos,m_pMissileInfo->m_nYPos);
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
// * 작성날짜: 2025. 09. 04
// ****************************************************************
    TimeType TimeAdvanceFn() override {
        return GetTimeA();
    }
};