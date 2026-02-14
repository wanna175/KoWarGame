// ********************************************************************
// * 소스정의: CommonUtil.cpp
// * 설    명: 다양한 유틸클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
#include "CommonUtil.h"
#include "TypeDef.h"
#include "MACRO.h"

#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;

// ********************************************************************
// * 함 수 명: Trim
// * 설    명: string 문자열에서 모든 " ", "\t", "\n", "\r"을 제거한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
void CommonUtil::Trim(string& strSource)
{
	const string strChrsToTrim = " \t\n\r";
	strSource.erase(std::remove_if(strSource.begin(), strSource.end(), [&](u_char chrs)
    {
        return strChrsToTrim.find(chrs) != std::string::npos;
    }
    ), strSource.end());
}
