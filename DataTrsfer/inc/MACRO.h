// ********************************************************************
// * 헤더정의: MACRO.h
// * 설    명: 매크로 함수들을 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
#pragma once

#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

#define DEBUG(msg) \
	cout << "[DEBUG] " << __FILE__ << " : " << __LINE__<< " - " << msg << endl;
#define ERROR(msg) \
	cerr << "[ERROR] " << __FILE__ << " : " << __LINE__<< " - " << msg << endl;
#define INFO(msg)  \
    cout << "[INFO] "  << __FILE__ << " : " << __LINE__<< " - " << msg << endl;

