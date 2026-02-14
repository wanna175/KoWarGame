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
#include "LogManager.h"
#include "DataTrsfModel.h"
#include "Config.h"
#include "ThreadPool.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

// ***************************************************************
// * 함 수 명: SignalHandler
// * 설    명: 시그널을 다룬다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 25
// ****************************************************************
void SignalHandler(const int_t nSignum)
{
        if (SIGINT == nSignum)
        {
                DEBUG("SIGINT");
                DATATRSFER::DataTrsfModel::GetInstance().Finish();
                std::quick_exit(nSignum);
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
        try {
                //시그널 핸들
                signal(SIGINT, SignalHandler);
                signal(SIGALRM, SIG_IGN);


                cout << "///////////////////////////////////////////////////////////////" << endl;
                cout << "/////                Start DataTransfer                   /////" << endl;
                cout << "///////////////////////////////////////////////////////////////" << endl;

                const int32_t nStartMode = std::stoi(argv[1], nullptr);
                auto& pDataTrsfModel = DATATRSFER::DataTrsfModel::GetInstance();
                auto& pConfigManager = CONFIG::ConfigManager::GetInstance();
                auto& pThreadPool    = ThreadPool::GetInstance();

                DEBUG("argc : " + to_string(argc));
                DEBUG("argv[0] : " + to_string(*argv[0]));
                DEBUG("argv[1] : " + to_string(nStartMode));

                pConfigManager.LoadConfig();
                pDataTrsfModel.Start(nStartMode);
        }
        catch (...) 
        {
                ERROR("try~catch");
        }
        return 0;
}

