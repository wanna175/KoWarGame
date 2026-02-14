// ********************************************************************
// * 소스정의: TimeControlModel.h
// * 설    명: 시뮬레이션 시간 동기화를 위한 시간제어모델을 정의,구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

#include "Atomic.h"
#include "StateVariable.h"
#include "TimeType.h"
#include "Message.h"
#include "Config.h"
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
// * 클래스명: TimeControlModel
// * 설    명: 시뮬레이션 시간 동기화를 위한 시간제어모델을 정의,구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class TimeControlModel : public Atomic
{
private:

public:
// ********************************************************************
// * 함 수 명: TimeControlModel
// * 설    명: MainCoupledModel 생성자. 연관 모델들의 포트연결을 담당한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    TimeControlModel(const string& name) : Atomic(name, StateVar({S_tic_WAIT }))
    {
        SetTimeA(1.0);
    }

// ****************************************************************
// * 함 수 명: ExtTransFn
// * 설    명: External State Transition Function (delta_{ext})
// *           외부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    bool_t ExtTransFn(const Message& msg) override 
    {
        //timecontrol model은 내부시간만 관리한다.
        return false;
    }

// ****************************************************************
// * 함 수 명: IntTransFn
// * 설    명: Internal State Transition Function (delta_{int})
// *           내부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    bool_t IntTransFn() override {
        if(GetCurrentState() == S_tic_WAIT)
        {
            SetTimeE(0);
            SetTimeA(1.0);
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
        if(GetCurrentState() == S_tic_WAIT)
        {
            msg.SetPort(P_tic_TIMEOUT);
            
            msg.SetContent(C_tic_TIMEOUT);
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