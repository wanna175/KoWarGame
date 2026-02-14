// ********************************************************************
// * 소스정의: Atomic.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************

#include "Atomic.h"
#include "Model.h"
#include "TypeDef.h"
#include "TimeType.h"
#include "StateVariable.h"

#include <string>

using std::string;

using namespace DEVSIM;
// ****************************************************************
// * 함 수 명: Atomic
// * 설    명: AtomicModel 생성자. 모델의 이름, 타입, 상태집합을 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
Atomic::Atomic(const string& name, const StateVar& stateSet) :Model(name)
{
    m_Type = ATOMIC;
    //TODO : 효율적으로 복사하기 위한 복사생성자 나중에 정의하자.
    m_StateVar = stateSet;
}

// ****************************************************************
// * 함 수 명: ~Atomic
// * 설    명: AtomicModel 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
Atomic::~Atomic()
{}


