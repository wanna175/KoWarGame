// ********************************************************************
// * 소스정의: LogManager.h
// * 설    명: log를 파일로 저장한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************

#pragma once

#include <string>
#include "TypeDef.h"

using std::string;

// ********************************************************************
// * 클래스명: LogManager
// * 설    명: log를 다룬다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 26
// ********************************************************************
class LogManager {
private:
	string m_fileName;
public:
	explicit LogManager();
	virtual ~LogManager();	

	void InitializeLog(void) const;
	void WriteLogData(const string strLogData);
};
