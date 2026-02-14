// ********************************************************************
// * 헤더정의: SimEngineDataTrsfModel.h
// * 설    명: engine 파트에서 데이터 전달처리 모델을 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
#pragma once

#include "SharedMemControl.h"
#include "MACRO.h"
#include "PacketUtil.h"
#include "TypeDef.h"
#include "Singleton.h"

#include "DEVSim.h"
#include "MilitaryCoupled.h"
#include "MainCoupledModel.h"

#include <thread>

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::to_string;

using namespace IPCSHAREDMEM;
using namespace DATATRSFER;
using namespace DEVSIM;

namespace DATATRSFER{
// ********************************************************************
// * 클래스명: SimEngineDataTrsfModel
// * 설    명: engine 파트에서 데이터 전달처리 모델을 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 30
// ********************************************************************
class SimEngineDataTrsfModel : public Singleton<SimEngineDataTrsfModel>{
private:
    unique_ptr<DEVSim> m_pSimulator;
    unique_ptr<SharedMemControl> m_pShmManager;
    unique_ptr<ScenarioControl> m_pScenarioManager;

    shared_ptr<MilitaryCoupledModel> m_pMilitaryModel;
    shared_ptr<MainCoupledModel> m_pMainModel;
    
    //////////////////////////////////////////////internal process를 위한 변수들
    std::atomic<bool_t> m_bIsLoadScenarioData;      //database에서 info를 다 불러왔는지 여부
    std::atomic<bool_t> m_bIsExit;
    std::thread m_thrInternalRecvThread;
private:
    void Initialize();
    void InternalProcessStart();
    void InternalProcessSendRoutine(const char_t *pSendingData, size_t nSendDataLen);
    void InternalProcessReceiveRoutine();

    void OutputSendMessageParse(const Message &pOutputMessage);

public:
    SimEngineDataTrsfModel() = default;
    ~SimEngineDataTrsfModel(){}
    void Start();
    void Finish();
};

}
