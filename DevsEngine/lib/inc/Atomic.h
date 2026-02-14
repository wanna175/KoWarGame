// ********************************************************************
// * 헤더정의: Atomic.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include "Model.h"
#include "MACRO.h"
#include "TypeDef.h"
#include "TimeType.h"
#include "StateVariable.h"
#include "Message.h"

#include <string>

using std::string;

namespace DEVSIM{
// ****************************************************************
// * 클래스명: Atomic
// * 설    명: AtomicModel을 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ****************************************************************
class Atomic : public Model{
private:
    StateVar m_StateVar;            //등록된 state및 현재 state정보 저장
    
    TimeType m_tE;                  //현재 상태에 머무른 시간
	TimeType m_tA;                  //현재 상태에 머무를 시간

public:
    Atomic(const string &name, const StateVar &stateSet);
    virtual ~Atomic();

//////////////////////////////////////////////////////////////
/* 아래의 함수들은 User가 반드시 override하여 사용하여야 한다. */

// ****************************************************************
// * 함 수 명: ExtTransFn
// * 설    명: External State Transition Function (delta_{ext})
// *           외부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
    virtual bool_t ExtTransFn(const Message &) { return false; }

// ****************************************************************
// * 함 수 명: IntTransFn
// * 설    명: Internal State Transition Function (delta_{int})
// *           내부 천이 함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
	virtual bool_t IntTransFn() { return false; }

// ****************************************************************
// * 함 수 명: OutputFn
// * 설    명: Output Function (lambda)
// *           출력함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
	virtual bool_t OutputFn(Message &) { return false; }

// ****************************************************************
// * 함 수 명: TimeAdvanceFn
// * 설    명: Time Advance Function ( ta )
// *           시간 진행함수
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
	virtual TimeType TimeAdvanceFn() { return -1; }


// ****************************************************************
// * 함 수 명: Set/Get TimeE,A
// * 설    명: 현재 상태에 머무를 시간 A, 머무른 시간 E 을 다루는 함수들 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
    void SetTimeE(TimeType time) { m_tE = time; }
    void SetTimeA(TimeType time) { m_tA = time; }
	TimeType GetTimeE() const { return m_tE; }
	TimeType GetTimeA() const { return m_tA; }

// ****************************************************************
// * 함 수 명: Set/Get StateVar
// * 설    명: 현재 state를 설정하고, 확인한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 23
// ****************************************************************
    string GetCurrentState() const { return m_StateVar.GetCurrentState();}
    bool_t SetCurrentState(const string& state) { return m_StateVar.SetCurrentState(state);}
};


}