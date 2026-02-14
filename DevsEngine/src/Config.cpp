// **************************************************************
//  소 스 명: Config.cpp						
//  설    명: ConfigManager를 구현한다.
//  작 성 자: KJH							
//  작성일자: 2025. 07. 04					
// **************************************************************
#include "Config.h"
#include "TypeDef.h"
#include "MACRO.h"
#include "CommonUtil.h"


#include <set>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using std::string;
using namespace CONFIG;

// ********************************************************************
// * 함 수 명: ~ConfigManager
// * 설    명: ConfigManager class 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
ConfigManager::~ConfigManager() 
{

}

// ********************************************************************
// * 함 수 명: GetValue
// * 설    명: Records에 작성된 요소를 읽어온다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
string ConfigManager::GetValue(const string& strKeyName, const string& strSectionName, const string& strFileName) const
{
    Record stRecord{strSectionName,strKeyName, ""};
    auto it = m_sRecords.find(stRecord);
    if (m_sRecords.end() != it)
    {
        return it->m_strValue;
    }
    return "";
}

// ********************************************************************
// * 함 수 명: LoadConfig 
// * 설    명: config file을 읽어서 메모리에 로드한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
void ConfigManager::LoadConfig() 
{
    Load(IPCSHAREDMEM::CONFIG_PATH);
    
}

// ********************************************************************
// * 함 수 명: Load
// * 설    명: strfileName file을 읽어서 Records에 기록한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
bool_t ConfigManager::Load(const string& strFileName) 
{
    std::ifstream fsInFile(strFileName);
    if (false == fsInFile.is_open()) 
    {
        ERROR("- Fail to open file : " + strFileName);
        return false;
    }

    string strLine, strCurrentSection;
    while(std::getline(fsInFile, strLine)) 
    {
        CommonUtil::Trim(strLine);
        if(strLine.empty() || strLine[0] == '#') continue;

        if(strLine.front() == '[' && strLine.back() == ']') 
        {
            strCurrentSection = strLine.substr(1, strLine.length() -2);
        }
        else
        {
            size_t pos = strLine.find('=');
            if (std::string::npos == pos) continue;

            string strKeyName     = strLine.substr(0, pos);
            string strValueName   = strLine.substr(pos + 1);

            Record stRecord{strCurrentSection, strKeyName, strValueName};

            m_sRecords.insert(stRecord); 

        }
    }
    fsInFile.close();
    return true;
}