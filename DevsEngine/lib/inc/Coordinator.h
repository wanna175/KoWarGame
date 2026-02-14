// ********************************************************************
// * 헤더정의: Coordinator.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ********************************************************************
#pragma once

#include "Processor.h"
#include "CoupledProcessor.h"
#include "Coupled.h"

#include "TypeDef.h"
#include "TimeType.h"

#include <map>
#include <vector>
#include <queue>

namespace DEVSIM{

// ****************************************************************
// * 클래스명: Coordinator
// * 설    명: 모든 프로세서의 최상위 루트
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
class Coordinator : public Processor{
private:
    std::queue<Message> m_vecOutMessages;
    std::shared_ptr<CoupledProcessor> m_rootProcessor;

    //동기화를 위한 변수 추가.
    int_t m_nExpectedDoneChildCnt;
    int_t m_nReceiveDoneChildCnt; 
public:
    Coordinator() : Processor() {}
    void Initial(shared_ptr<Processor> upperProc, shared_ptr<Model> model, TimeType value) override;

    bool_t WhenRcvX(Message &) override;
    bool_t WhenRcvStar(Message &) override;
    bool_t WhenRcvY(Message &) override;
    bool_t WhenRcvDone(Message &) override;
    void CleanUp() override;

    bool_t GetOutMessageFromVector(Message&);
};

}