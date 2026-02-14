// ********************************************************************
// * 헤더정의: AtomicProcessor.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include "Model.h"
#include "Atomic.h"
#include "Processor.h"

#include "TypeDef.h"
#include "TimeType.h"


namespace DEVSIM{

// ****************************************************************
// * 클래스명: AtomicProcessor
// * 설    명: 시뮬레이션 내에서 AtomicModel 를 관리할 실행기를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class AtomicProcessor : public Processor{
public:
    void Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value) override;

    bool_t WhenRcvX(Message &) override;
    bool_t WhenRcvStar(Message &) override;
    bool_t SendUpperProcDone();
    bool_t SendUpperProcY();
    void CleanUp() override;
};

}