// ********************************************************************
// * 헤더정의: Processor.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include <string>
#include <list>
#include <memory>
#include <set>
#include <map>

#include "TypeDef.h"
#include "TimeType.h"
#include "Model.h"
#include "Message.h"

using std::string;
using std::list;
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;

namespace DEVSIM{

// ****************************************************************
// * 클래스명: Processor
// * 설    명: 모델 실행기 Processor class 를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class Processor : public std::enable_shared_from_this<Processor>{
protected:
    shared_ptr<Model> 			m_Model;  //관리하는 모델
    int_t        				m_type;   //관리하는 모델의 타입 (atomic, coupled)
    weak_ptr<Processor> 		m_UpperProcessor;     //상위 프로세서
	std::map<string, shared_ptr<Processor>>			m_SubProcessor;		//하위 프로세서들	

    TimeType 					m_tN;	        // Next Event Time
	TimeType 					m_tL;	        // Last Event Time
	TimeType 					m_tInit;		// Initial Time
	TimeType 					m_tCurrent;	// Current Time

    list<string> 				m_InPorts;
    list<string> 				m_OutPorts;

    
public:
    Processor() = default;
public:
    virtual ~Processor() = default;

// ********************************************************************
// * 함수정의: GetType
// * 설    명: 현재 모델의 타입을 반환한다. (Atomic, Coupled)
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	int_t GetType() const {return m_type;}

// ********************************************************************
// * 함수정의: GetTN / GetTL
// * 설    명: 현재 모델의 다음 이벤트 시간, 이전 이벤트 시간을 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
    TimeType GetTN() const {return m_tN;}
	TimeType GetTL() const {return m_tL;}
	void SetTN(TimeType tN) { m_tN = tN; }

// ********************************************************************
// * 함수정의: Get/SetInitialTime
// * 설    명: 시뮬레이션 초기 시각을 설정하고 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	virtual TimeType GetInitialTime() { return m_tInit; }
	virtual void SetInitialTime(TimeType tInit) { m_tInit = tInit; }

// ********************************************************************
// * 함수정의: Get/SetTime
// * 설    명: 시뮬레이션 현재 시각을 설정하고 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	virtual TimeType GetTime() { return m_tCurrent; }
	virtual void SetTime(TimeType current) { m_tCurrent = current; }
	
// ********************************************************************
// * 함수정의: Initial
// * 설    명: 시뮬레이션 초기설정. 시작시 호출되어야 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	virtual void Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value = 0)
    {
        m_UpperProcessor = upperProc;
        m_Model = model;
        m_tInit = value; m_tCurrent = value; 
		m_type  = model->GetType();
	}

// ********************************************************************
// * 함수정의: WhenRcvX
// * 설    명: (x,t) Message 핸들러, 즉 t 시간에 들어온 X 인풋을 처리한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************		 
	virtual bool_t WhenRcvX(Message &)  {return false;}

// ********************************************************************
// * 함수정의: WhenRcvStar
// * 설    명: (*,t) Message 핸들러. 타임아웃 이벤트 메세지를 처리한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	 
	virtual bool_t WhenRcvStar(Message &)  {return false;}

// ********************************************************************
// * 함수정의: WhenRcvY
// * 설    명: (y,t) Message 핸들러. 출력 메세지를 처리한다.	
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	 
	virtual bool_t WhenRcvY(Message &)  {return false;}

// ********************************************************************
// * 함수정의: WhenRcvDone
// * 설    명: (done,t_N) Message. 처리완료 메세지를 핸들. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	virtual bool_t WhenRcvDone(Message &)  {return false;}

// ********************************************************************
// * 함수정의: CleanUp
// * 설    명: 시뮬레이션 종료 후 호출되어야 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	virtual void CleanUp() {}
};

}