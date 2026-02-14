// ********************************************************************
// * 소스정의: LogManager.cpp
// * 설    명: LogManager class 를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <cstring>
#include "LogManager.h"
#include "MACRO.h"

using std::cerr;
using std::string;
using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;
using std::stringstream;
using std::to_string;
using std::stoi;
using std::ios;

// ********************************************************************
// * 함 수 명: LogManager 생성자
// * 설    명: 생성자
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************
LogManager::LogManager() {
	m_fileName = "";
}

// ********************************************************************
// * 함 수 명: LogManager
// * 설    명: 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************
LogManager::~LogManager() {
	
}

// ********************************************************************
// * 함 수 명: InitializeLog
// * 설    명: Log Manager을 초기화 한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************
void LogManager::InitializeLog(void) const {
	//현재 작업중인 디렉토리의 정보를 얻는다.
	char_t szDirPath[100];
	std::memset(szDirPath, 0, sizeof(szDirPath));

	char_t* cirDirName = getcwd(szDirPath, 100);
	if (cirDirName ==nullptr) {
		ERROR("getcwd() fail");
		return; }

	//LOG 디렉터리가 없을 때 생성
	string strLogDirPath = szDirPath;
	strLogDirPath.append("/LOG");

	INFO("LOG dir : " + strLogDirPath);

	int32_t nResult = access(strLogDirPath.c_str(), F_OK);
	if (nResult != 0) {	//Log 디렉터리가 없음
		int_t log_dir = mkdir("LOG",0755);
		if (log_dir != 0) {
			ERROR("mkdir() fail");
			return;
		}
		INFO("Success Create Log Directory!! : " + strLogDirPath);
	}
}

// ********************************************************************
// * 함 수 명: WriteLogData
// * 설    명: Log를 쓴다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
void LogManager::WriteLogData(const string strLogData) {
	const time_t tmtSystemTime = time(nullptr);
	const struct tm* strtTm = localtime(&tmtSystemTime);	//현재 시간을 가지고 옴

	const int_t nCurrentYear = strtTm->tm_year + 1900;
	const int_t nCurrentMonth = strtTm->tm_mon + 1;			// month range : 0 ~ 11
	const int_t nCurrentDay = strtTm->tm_mday;
	const int_t nCurrentHour = strtTm->tm_hour;
	int_t nCurrentMin = strtTm->tm_min;

	
}

