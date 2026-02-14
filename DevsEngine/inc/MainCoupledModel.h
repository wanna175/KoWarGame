// ********************************************************************
// * 소스정의: MainCoupledModel.h
// * 설    명: 최상위 커플드모델 Main을 정의하고 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

#include "Coupled.h"
#include "MilitaryCoupled.h"
#include "TimeControlModel.h"
#include "TypeDef.h"
#include "Global.h"

using namespace DEVSIM;
using namespace ModelConfig::Content;
using namespace ModelConfig::Port;
using namespace ModelConfig::State;

// ********************************************************************
// * 클래스명: MainCoupledModel
// * 설    명: 최상위 커플드모델 Main을 정의하고 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class MainCoupledModel : public Coupled{
private:
    shared_ptr<MilitaryCoupledModel> m_pMilitaryModel;
public:

// ********************************************************************
// * 함 수 명: MainCoupledModel
// * 설    명: MainCoupledModel 생성자. 연관 모델들의 포트연결을 담당한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    MainCoupledModel(const string& name) : Coupled(name)
    {
        auto pMilitaryCoupledModel = std::make_shared<MilitaryCoupledModel>("MilitaryCoupled");
        auto pTimeControlModel = std::make_shared<TimeControlModel>("TimeControl");

        m_pMilitaryModel = pMilitaryCoupledModel;

        AddComponent(pMilitaryCoupledModel);
        AddComponent(pTimeControlModel);

        //external input connect → pMilitaryCoupled
        AddCoupling(nullptr,P_main_IN,pMilitaryCoupledModel,P_mic_IN);

        //connect pMillitaryCoupled OUT → external
        AddCoupling(pMilitaryCoupledModel, P_mic_OUT, nullptr, P_main_OUT);

        //connect pTimeControl OUT → external
        AddCoupling(pTimeControlModel, P_tic_TIMEOUT, nullptr, P_main_OUT);
    }
// ********************************************************************
// * 함 수 명: GetMilitaryModel
// * 설    명: war game객체정보를 들고있는 militaryCoupled model의 쉐어드 포인터를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    shared_ptr<MilitaryCoupledModel> GetMilitaryModel() { return m_pMilitaryModel;}
};