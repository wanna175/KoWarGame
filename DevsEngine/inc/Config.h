// **************************************************************
//  헤 더 명: Config.h						
//  설    명: config 관련 전역상수및 ConfigManager를 정의한다.
//  작 성 자: KJH							
//  작성일자: 2025. 07. 01					
// **************************************************************
#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include "TypeDef.h"
#include "Singleton.h"

using std::string;
using std::set;
using std::map;
using std::vector;

//SimEngineDataTrsfModel 관련 상수들
namespace DATATRSFER 
{
    const uint32_t BUFSIZE     = 1024 * 10;
}

//ipc shared memory 관련 상수들
namespace IPCSHAREDMEM
{
    ///TODO: 나중에 패킷사이즈로 바꾸자.
    const int_t SHM_DATA_SIZE           = 100;             
    const int_t SHM_QUEUE_SIZE          = 1024;

    const string READ_SECTION_NAME      = "READ_SHARED_INFO";
    const string WRITE_SECTION_NAME     = "WRITE_SHARED_INFO";
    const string CONFIG_PATH            = "./cfg/ipcconfig.cfg";

    const string READ_SHM_NAME          = "READ_SHM_NAME";			
	const string WRITE_SHM_NAME         = "WRITE_SHM_NAME";			
	//const int_t READ_SHM_SIZE
	//const int_t WRITE_SHM_SIZE						
	const string READ_SEMAPHORE_FULL    = "READ_SEM_FULL";	
	const string READ_SEMAPHORE_EMPTY   = "READ_SEM_EMPTY";
	const string WRITE_SEMAPHORE_FULL   = "WRITE_SEM_FULL";
	const string WRITE_SEMAPHORE_EMPTY  = "WRITE_SEM_EMPTY";
}


namespace CONFIG {
// ********************************************************************
// * 구조체명: Record
// * 설    명: file에 들어있는 요소를 나타내는 구조체
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
struct Record
{
	string  		m_strSection;
	string  		m_strKey;
	string  		m_strValue;

    bool_t operator<(const Record& other) const
    {
        if (m_strSection != other.m_strSection) return m_strSection < other.m_strSection;
        return m_strKey < other.m_strKey;
    } 
};
// ********************************************************************
// * 클래스명: ConfigManager
// * 설    명: file config등을 읽어서 관리하는 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 04
// ********************************************************************
class ConfigManager : public Singleton<ConfigManager> {
private:
    set<Record> m_sRecords;


private:
    bool_t Load(const string& strFileName);
public:
    ~ConfigManager();
    string GetValue(const string& strKeyName, const string& strSectionName, const string& strFileName) const;
    void LoadConfig();
    
};
}