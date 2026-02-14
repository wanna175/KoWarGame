// ********************************************************************
// * 소스정의: StateVariable.cpp
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ********************************************************************
#include "StateVariable.h"
#include "MACRO.h"

namespace DEVSIM {

// ************************************************************
// * 함 수 명: StateVar({...})
// * 설    명: StateVar의 생성자, 문자열 기반으로 초기화
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
StateVar::StateVar(std::initializer_list<string> stateNames)
    : m_nCurrentState(0)
{
    int_t id = 0;
    for (const auto& name : stateNames) 
    {
        m_IntState[id] = name;
        ++id;
    }
    if (!m_IntState.empty())
        m_nCurrentState = 0;
}

// ************************************************************
// * 함 수 명: ~StateVar
// * 설    명: StateVar의 소멸자
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
StateVar::~StateVar() 
{
	m_IntState.clear();
	m_nCurrentState = 0;
}

// ************************************************************
// * 함 수 명: AddStateVar
// * 설    명: 새로운 상태 변수 추가
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
// bool_t StateVar::AddStateVar(string state) {
// 	if (ExistStateVar(state))
// 		return false;
// 	m_IntState[state] = "";
// 	return true;
// }

// ************************************************************
// * 함 수 명: GetStateVar
// * 설    명: 특정 상태값의 문자열 반환
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
// string StateVar::GetStateVar(int_t state) const {
// 	auto it = m_IntState.find(state);
// 	if (it != m_IntState.end())
// 		return it->second;
// 	return "";
// }

// ************************************************************
// * 함 수 명: GetCurrentState
// * 설    명: 현재 상태 반환
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
string StateVar::GetCurrentState() const 
{
	return m_IntState.at(m_nCurrentState);
}

// ************************************************************
// * 함 수 명: ExistStateVar
// * 설    명: 해당 상태가 존재하는지 확인
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
int_t StateVar::ExistStateVar(const string& name) const 
{
    for (const auto& [id, val] : m_IntState) 
    {
        if (val == name)
            return id;
    }
    return -1;
}

// ************************************************************
// * 함 수 명: SetCurrentState
// * 설    명: 현재 상태를 셋팅한다.
// * 작 성 자: KJH
// * 작 성 일: 2025. 07. 23
// ************************************************************
bool_t StateVar::SetCurrentState(const string& name) 
{
    int_t ret_e = ExistStateVar(name);
    if (ret_e == -1) return false;
    
    m_nCurrentState = ret_e;
    
    return true; 
}

} // namespace DEVSIM
