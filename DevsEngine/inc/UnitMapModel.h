// ********************************************************************
// * 소스정의: UnitMapModel.h
// * 설    명: 다른 아토믹모델에게 메세지를 파싱하여 전달하는 모델 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

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
// * 클래스명: UnitMapModel
// * 설    명: 다른 아토믹모델에게 메세지를 파싱하여 전달하는 모델 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class UnitMapModel : public Atomic
{
private:
    Message m_RecvMessage;
public:
// ****************************************************************
// * 함 수 명: UnitMapModel
// * 설    명: 생성자.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ****************************************************************
    UnitMapModel(const string& name) : Atomic(name, StateVar({S_map_WAIT }))
    {
        SetTimeA(Infinity);
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
        //이벤트가 발생할 obj model의 id, type, content를 저장한다.
        m_RecvMessage.SetContent(msg.GetContent());
        m_RecvMessage.SetDetail(msg.GetObjType(),msg.GetObjId(),msg.GetDestinationXpos(),msg.GetDestinationYpos());
        SetTimeE(0);
        SetTimeA(0);
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
        if(GetCurrentState() == S_map_WAIT)
        {
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
    bool_t OutputFn(Message& msg) override 
    {
        msg.SetPort(P_map_OUT + to_string(m_RecvMessage.GetObjId()));           
        msg.SetContent(m_RecvMessage.GetContent());
        msg.SetDetail(m_RecvMessage.GetObjType(),m_RecvMessage.GetObjId(),m_RecvMessage.GetDestinationXpos(),m_RecvMessage.GetDestinationYpos());
   
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