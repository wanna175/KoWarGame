// ********************************************************************
// * 헤더정의: CoupledProcessor.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 19
// ********************************************************************
#pragma once

#include "Processor.h"
#include "Coupled.h"

#include "TypeDef.h"
#include "TimeType.h"

#include <map>
#include <vector>

namespace DEVSIM{

// ****************************************************************
// * 클래스명: CoupledProcessor
// * 설    명: 시뮬레이션 내에서 CoupledModel 를 관리할 실행기를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 19
// ****************************************************************
class CoupledProcessor : public Processor{
private:
    std::vector<Message> m_vecOutMessages;
    std::vector<Message> m_vecDoneMessages;


    //동기화를 위한 변수 추가.
    int_t m_nExpectedDoneChildCnt;
    int_t m_nReceiveDoneChildCnt; 
public:
    void Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value) override;

    bool_t WhenRcvX(Message &) override;
    bool_t WhenRcvStar(Message &) override;
    bool_t WhenRcvY(Message &) override;
    bool_t WhenRcvDone(Message &) override;
    bool_t SendUpperProcDone();
    bool_t SendUpperProcY();
    void CleanUp() override;
};

}