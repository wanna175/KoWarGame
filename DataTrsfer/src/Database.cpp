// **************************************************************
//  소 스 명: Database.cpp						
//  설    명: database 연결을 위한 클래스를 정의한다.
//  작 성 자: KJH							
//  작성일자: 2025. 09. 15					
// **************************************************************
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <map>
#include "Database.h"
#include "CommonUtil.h"
#include "TypeDef.h"
#include "MACRO.h"
#include "Config.h"
#include <fstream>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
//#include "CAes.h"


using namespace DATABASE;
using std::string;
using std::vector;
using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;
using std::stringstream;
using std::to_string;
using std::stoi;
using std::map;


// ********************************************************************
// * 함 수 명: CDatabase
// * 설    명: CDatabase 의 생성자
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 16
// ********************************************************************
CDatabase::CDatabase()
{
}

// ********************************************************************
// * 함 수 명: CDatabase
// * 설    명: CDatabase 의 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 16
// ********************************************************************
CDatabase::~CDatabase()
{
	DEBUG("~CDatabase() is called");
	Disconnect();
}

// ********************************************************************
// * 함 수 명: Initialize
// * 설    명: Database connect 초기화를 담당한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 22
// ********************************************************************
bool_t CDatabase::InitializeDB()
{
	INFO("start connect Database server...");

	auto& pConfig = CONFIG::ConfigManager::GetInstance();

	string strSectionName = DATABASE::DATABASE_SECTION_NAME;
	string cfgPath = DATABASE::CONFIG_PATH;

	string strKeyName = DATABASE::DATABASE_ADMIN_ID;
	const string strTmpAdminId = pConfig.GetValue(strKeyName,strSectionName,cfgPath);

	strKeyName = DATABASE::DATABASE_ADMIN_PW;
	const string strTmpAdminPw = pConfig.GetValue(strKeyName,strSectionName,cfgPath);

	//////////////////////////////// TODO: DB에 id, pw 저장시 암호화 해야함

	/*string descId;
	string descPW;

	if( (true != strTmpAdminId.empty()) && (true != strTmpAdminPw.empty()))
	{
		//CAes theAes128(128){ 0, MODE::ECB };
		CAes theAes128(128, MODE::ECB);
		descId = theAes128.decryption(strTmpAdminId.c_str());
		descPW = theAes128.decryption(strTmpAdminPw.c_str());
	}
	*/

	const string strAdminId = strTmpAdminId;
	const string strAdminPw = strTmpAdminPw;

	strKeyName = DATABASE::DATABASE_IP_ADDR;
	const string strAdminIpAddress = pConfig.GetValue(strKeyName, strSectionName, cfgPath);

    strKeyName = DATABASE::DATABASE_PORT;
	const string strPort = pConfig.GetValue(strKeyName, strSectionName, cfgPath);

	strKeyName = DATABASE::DATABASE_NAME;
    const string strDbName = pConfig.GetValue(strKeyName, strSectionName, cfgPath);

    try 
	{
        // MySQL 드라이버 가져오기
        m_pDriver = sql::mysql::get_mysql_driver_instance();

        // 연결 문자열 생성
        string url = "tcp://" + strAdminIpAddress + ":" + strPort;

        // DB 연결
        m_pMySQLConnection.reset(m_pDriver->connect(url, strAdminId, strAdminPw));

        // 스키마(데이터베이스) 선택
        m_pMySQLConnection->setSchema(strDbName);

        INFO("success Connect Database Server");
        return true;
    }
    catch (sql::SQLException& e) 
	{
        ERROR("fail to Connect Database Server : " + (string)e.what());
        return false;
    }
}

// ********************************************************************
// * 함 수 명: Disconnect
// * 설    명: Database connect를 종료한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
void CDatabase::Disconnect() 
{
    if (m_pMySQLConnection) 
	{
        m_pMySQLConnection->close();
        m_pMySQLConnection.reset();
    }
	INFO("Disconnect Database server...");
}

// ********************************************************************
// * 함 수 명: Execute
// * 설    명: Database INSERT, UPDATE, DELETE query를 실행한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
bool_t CDatabase::Execute(const string& query) 
{
    try 
	{
        std::unique_ptr<sql::Statement> stmt(m_pMySQLConnection->createStatement());
        stmt->execute(query);
        return true;
    } 
	catch (sql::SQLException& e) 
	{
		ERROR("[DB] Execute error: " + (string)e.what());
        return false;
    }
}

// ********************************************************************
// * 함 수 명: Query
// * 설    명: Database SELECT 실행 후 결과를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
vector<vector<string>> CDatabase::Query(const string& query) 
{
    vector<vector<string>> results;
    try 
	{
        std::unique_ptr<sql::Statement> stmt(m_pMySQLConnection->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));
        int cols = res->getMetaData()->getColumnCount();
        while (res->next()) 
		{
            std::vector<std::string> row;
            for (int i = 1; i <= cols; ++i) 
			{
                row.push_back(res->getString(i));
            }
            results.push_back(row);
        }
    } 
	catch (sql::SQLException& e) 
	{
		ERROR("[DB] Query error: " + (string)e.what());
    }
    return results;
}

// ********************************************************************
// * 함 수 명: ExecutePrepared
// * 설    명: Database Prepared Statement 실행 (INSERT/UPDATE/DELETE)
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
bool_t CDatabase::ExecutePrepared(const string& query, const vector<string>& params)
{
    try 
	{
        std::unique_ptr<sql::PreparedStatement> pstmt(m_pMySQLConnection->prepareStatement(query));
        for (size_t i = 0; i < params.size(); ++i) 
		{
            pstmt->setString(static_cast<int>(i + 1), params[i]);
        }
        pstmt->execute();
        return true;
    } 
	catch (sql::SQLException& e) 
	{
		ERROR("[DB] ExecutePrepared error: " + (string)e.what());
        return false;
    }
}

// ********************************************************************
// * 함 수 명: AuthLoginInfo
// * 설    명: Login request 시 database server에서 확인한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
int_t CDatabase::AuthLoginInfo(const string strUsername, const string strPw)
{
	try 
	{
        std::unique_ptr<sql::PreparedStatement> pstmt(
            m_pMySQLConnection->prepareStatement("SELECT pw FROM user WHERE username = ?"));

        pstmt->setString(1, strUsername);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// Username 없음
        if (!res->next()) 
		{
            return -1;
        }

        std::string dbPw = res->getString("pw");
        // 로그인 성공
		if (dbPw == strPw) 
		{
            return 1;
        } 
		// PW 불일치
		else 
		{
            return 0;   
        }
    }
	// DB 오류
    catch (sql::SQLException& e) 
	{
		ERROR("[DB] AuthLoginInfo error: "+(string)e.what());
        return -2; // DB 오류
    }
}

// ********************************************************************
// * 함 수 명: DownloadMilitaryUnitInfo
// * 설    명: database에서 부대정보를 다운로드한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 23
// ********************************************************************
vector<StMilitaryUnitInfo> CDatabase::DownloadMilitaryUnitInfo()
{
    vector<StMilitaryUnitInfo> units;

    try 
	{
        std::unique_ptr<sql::PreparedStatement> pstmt(
            m_pMySQLConnection->prepareStatement(
                "SELECT id, name, longitude, latitude, total_artillery_cnt "
                "FROM military_unit ORDER BY id ASC"
            )
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) 
		{
            StMilitaryUnitInfo info;

            info.m_nID = static_cast<int16_t>(res->getInt("id"));
            info.m_strName = res->getString("name");

            info.m_nXPos = static_cast<int>(res->getDouble("longitude") * 1000); 
            info.m_nYPos = static_cast<int>(res->getDouble("latitude") * 1000);  

            info.m_nTotalArtillery = res->getInt("total_artillery_cnt");

            units.push_back(info);
        }
    }
    catch (sql::SQLException& e) 
	{
		ERROR("[DB] DownloadMilitaryUnitInfo error: " + (string)e.what());
    }

    return units;
}
