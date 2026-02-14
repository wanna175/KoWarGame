// ********************************************************************
// * 헤더정의: StateVariable.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include "TypeDef.h"

#include <map>
#include <iostream>
#include <string>

using std::string;

namespace DEVSIM{

// ****************************************************************
// * 클래스명: StateVar
// * 설    명: Atomic Model이 가지고 있을 수 있는 State를 저장할
// *           클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ****************************************************************
class StateVar  
{
public:
	typedef std::map<int_t, string> IntStateMap;

	IntStateMap		m_IntState;
	int_t			m_nCurrentState;
public:
	StateVar() {}
	StateVar(std::initializer_list<string> stateNames);  // 문자열 리스트 생성자
	~StateVar();

	// State Variable
	//bool_t AddStateVar(string);
	//string GetStateVar(int_t) const;
	string GetCurrentState() const;
    int_t ExistStateVar(const string &name) const;
    bool_t SetCurrentState(const string &name);
};

}