// **************************************************************
//  헤 더 명: Database.h						
//  설    명: database 연결을 위한 클래스를 정의한다.
//  작 성 자: KJH							
//  작성일자: 2025. 09. 15					
// **************************************************************
#pragma once

#include <string>
#include <TypeDef.h>
#include <MACRO.h>
#include <map>
#include <vector>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

using std::vector;
using std::string;
using std::map;


namespace DATABASE {

    typedef struct USER_INFO
    {
        std::string strUserName;
        std::string strPwCtnt;
    }StUserInfo;

    typedef struct Military_Unit
    {
        int16_t     m_nID;
        std::string m_strName;
        int_t       m_nXPos;
        int_t       m_nYPos;
        int_t       m_nTotalArtillery;
    }StMilitaryUnitInfo;

// ********************************************************************
// * 클래스명: CDatabase
// * 설    명: database 연결및 관리 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 16
// ********************************************************************
    class CDatabase
    {
    private:
        sql::mysql::MySQL_Driver* m_pDriver;
        std::unique_ptr<sql::Connection> m_pMySQLConnection;
    
    private:
        void Disconnect();
    public:
        explicit CDatabase();
        virtual ~CDatabase();
        
        bool_t InitializeDB();

        bool_t Execute(const string& query);
        vector<vector<string>> Query(const string& query);
        bool_t ExecutePrepared(const string& query, const vector<string>& params);


        int_t AuthLoginInfo(const string strUsername, const string strPw);
        vector<StMilitaryUnitInfo> DownloadMilitaryUnitInfo();
        //map<int_t, vector<string>> DownloadUserInfo(void) const;
    };
}