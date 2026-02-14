// ********************************************************************
// * 헤더정의: Model.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ********************************************************************
#pragma once


#include <string>
#include <list>

#include "TypeDef.h"
#include "TimeType.h"

using std::string;

namespace DEVSIM{

enum ModelType {
	ATOMIC,		/* Atomic Model */
	COUPLED		/* Coupled Model */
};

// ***************************************************************
// * 클래스명: Model
// * 설    명: BaseModel을 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ****************************************************************
class Model {
protected:
    string 		m_strName;
	ModelType   m_Type;

	int_t m_nPriority;                //동일시간내 발생시 우선순위
public:

// ********************************************************************
// * 함수정의: Model
// * 설    명: 모델의 이름을 받는 생성자.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	Model(const string& name) : m_strName(name) {};

    virtual ~Model() {};

// ********************************************************************
// * 함수정의: get/set
// * 설    명: 멤버변수에 접근할 수 있는 getter, setter 함수들
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	void SetName(const string& name) { m_strName = name;}
    string GetName() const { return m_strName; }

	int_t GetType() const { return m_Type; }

	void SetPriority(int_t priority) {m_nPriority = priority;}
};

}