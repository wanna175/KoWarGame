// ********************************************************************
// * 소스정의: main 
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 25
// ********************************************************************

#include <iostream>
#include <cstdlib>
#include <csignal>
#include <fstream>
#include <string>
#include "TypeDef.h"
#include "MACRO.h"
#include "Config.h"
#include "ThreadPool.h"
#include "PacketUtil.h"

#include "SimEngineDataTrsfModel.h"
#include "SharedMemControl.h"

#include "Coordinator.h"
#include "DEVSim.h"
#include "MilitaryCoupled.h"
#include "TimeType.h"
#include <cstring>
#include <netinet/in.h>


using std::cout;
using std::endl;
using std::string;
using std::to_string;

using namespace IPCSHAREDMEM;
using namespace DATATRSFER;
using namespace DEVSIM;

// ***************************************************************
// * 함 수 명: SignalHandler
// * 설    명: 시그널을 다룬다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 25
// ****************************************************************
void SignalHandler(const int_t signum)
{
        if (SIGINT == signum)
        {
                DEBUG("SIGINT");
                DATATRSFER::SimEngineDataTrsfModel::GetInstance().Finish();
                std::quick_exit(signum);
                //todo : at_quick_exit() 나중에 필요한 종료해야할 것들 등록
        }
}

// ***************************************************************
// * 함 수 명: main
// * 설    명: main 함수를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 25
// ****************************************************************
int32_t main(const int32_t argc, const char_t** argv) {
        try 
        {
                //시그널 핸들
                signal(SIGINT, SignalHandler);
                signal(SIGALRM, SIG_IGN);


                cout << "///////////////////////////////////////////////////////////////" << endl;
                cout << "/////                 Start SimEngine                     /////" << endl;
                cout << "///////////////////////////////////////////////////////////////" << endl;

                auto& pConfigManager = CONFIG::ConfigManager::GetInstance();
                //auto& pThreadpool    = ThreadPool::GetInstance();
                auto& pDataTrsfModel = DATATRSFER::SimEngineDataTrsfModel::GetInstance();
                pConfigManager.LoadConfig();
                
                pDataTrsfModel.Start();
        }
        catch (...) 
        {
                ERROR("try~catch");
        }
        return 0;
}

