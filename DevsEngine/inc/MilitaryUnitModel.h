// ********************************************************************
// * 소스정의: MilitaryUnitModel.h
// * 설    명: 부대의 행동을 나타내는 모델을 정의/구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

#include "Coupled.h"
#include "Atomic.h"
#include "StateVariable.h"
#include "TimeType.h"
#include "Message.h"
#include "Config.h"
#include "ModelStruct.h"
#include "Global.h"

#include <string>
#include <queue>
#include <map>
#include <vector>

using std::to_string;
using std::shared_ptr;

using namespace DEVSIM;
using namespace ModelConfig::Content;
using namespace ModelConfig::Port;
using namespace ModelConfig::State;

// ********************************************************************
// * 클래스명: MilitaryUnitModel
// * 설    명: 부대의 행동을 나타내는 모델을 정의/구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class MilitaryUnitModel : public Atomic
{
private:
    std::shared_ptr<CMilitaryUnit> m_pMilitaryUnitInfo;
public:
// ********************************************************************
// * 함 수 명: MilitaryUnitModel
// * 설    명: MainCoupledModel 생성자.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    MilitaryUnitModel(const string& name, std::shared_ptr<CMilitaryUnit> pMilitaryUnit) : Atomic(name, StateVar({S_mil_WAIT,S_mil_ORDER,S_mil_ATTACKED,S_mil_DESTROY }))
    {
        SetTimeA(Infinity);
        m_pMilitaryUnitInfo = pMilitaryUnit;
    }

// ****************************************************************
// * 함 수 명: ExtTransFn
// * 설    명: External State Transition Function (delta_{ext})
// *           외부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    bool_t ExtTransFn(const Message& msg) override {

        if (GetCurrentState() == S_mil_DESTROY)
        {
            return false;
        }

        //출격명령도착
        if (msg.GetContent() == C_mil_ORDER_IN_SALLY)
        {
            DEBUG("GET ORDER IN SALLY " + std::to_string(msg.GetTime())+" : " + GetName());
            //목적지 설정...
            m_pMilitaryUnitInfo->SetDestinationArtillery(msg.GetDestinationXpos(),msg.GetDestinationYpos());

            bool_t ret_s = SetCurrentState(S_mil_ORDER);
            if (!ret_s)
            {
                ERROR("SetCurrentState() fail");
                return false;
            }
            SetTimeE(0);
            SetTimeA(1.0);
        }
        //피격시
        else if (msg.GetContent() == C_mil_ATTACKED) {
            DEBUG("GET ATTACKED "+ std::to_string(msg.GetTime())+" : " + GetName());
            m_pMilitaryUnitInfo->m_nUnitHealth-=50;
            bool_t ret_s = m_pMilitaryUnitInfo->m_nUnitHealth > 0 ? SetCurrentState(S_mil_ATTACKED) : SetCurrentState(S_mil_DESTROY);
            if (!ret_s)
            {
                ERROR("SetCurrentState() fail");
                return false;
            }

            SetTimeE(0);
            SetTimeA(0.0);  
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
        if(GetCurrentState() == S_mil_ORDER)
        {
            SetCurrentState(S_mil_WAIT);
            SetTimeE(0);
            SetTimeA(Infinity);
        }
        else if(GetCurrentState() == S_mil_ATTACKED)
        {
            SetCurrentState(S_mil_WAIT);
            SetTimeE(0);
            SetTimeA(Infinity);
        }
        else if(GetCurrentState() == S_mil_DESTROY)
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
        DEBUG("miliytsryModel Outputfn");
        if(GetCurrentState() == S_mil_ORDER)
        {
            shared_ptr<CArtillery> pArtillery = m_pMilitaryUnitInfo->GetArtilleryFromQueue();
            if (pArtillery != nullptr) 
            {
                //연결된 자주포에게 명령을 내려야 한다.
                msg.SetPort(P_mil_OUT + std::to_string(pArtillery->m_nID));
                msg.SetContent(C_mil_ORDER_IN_SALLY);
                //msg.SetDetail(ENGINE::ARTILLERY, artilleryId, msg);
                //TODO : total 자주포 감소 : atomic으로 해야할지 고민...
                (m_pMilitaryUnitInfo->m_nTotalArtillery)--;
            }
            else 
            {
                //사용자에게 잘못된 명령이라고 보내줘야한다.
            }
        }
        else if(GetCurrentState() == S_mil_ATTACKED)
        {
            msg.SetPort(P_mil_ATTACKED_OUTPUT);
            msg.SetContent(C_mil_ATTACKED);
            msg.SetDetail(ObjectID::MILITARY_UNIT, m_pMilitaryUnitInfo->m_nID);
        }
        else if(GetCurrentState() == S_mil_DESTROY)
        {
            msg.SetPort(P_mil_DESTROY_OUTPUT);
            msg.SetContent(C_mil_DESTROY);
            msg.SetDetail(ObjectID::MILITARY_UNIT, m_pMilitaryUnitInfo->m_nID);
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