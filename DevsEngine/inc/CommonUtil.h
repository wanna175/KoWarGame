// ********************************************************************
// * 헤더정의: CommonUtil.h
// * 설    명: 다양한 유틸클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************

#pragma once

#include <string>
#include <vector>
#include "TypeDef.h"
#include "MACRO.h"



// ********************************************************************
// * 클래스명: CommonUtil
// * 설    명: 다양한 유틸클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
class CommonUtil {
private:
    CommonUtil()  = delete;
    ~CommonUtil() = delete; 

public:
	static void Trim(std::string& strSource);

};

