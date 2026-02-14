#pragma once

#include <string>
#include <queue>
#include <map>
#include <vector>
#include <memory>

#include "TypeDef.h"
#include "TimeType.h"
#include "MACRO.h"
#include <mutex>
#include <cmath>

using std::shared_ptr;
using std::to_string;

//model 오브젝트 타입
enum ObjectID : int8_t
{
    MILITARY_UNIT,
    ARTILLERY,
    MISSILE,
};
// ********************************************************************
// * 클래스명: CMissile
// * 설    명: missile객체의 정보를 담는 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
class CMissile
{
public:
    int_t     m_nID;
    int_t     m_nXPos;
    int_t     m_nYPos;
    int_t     m_nVelocity;

    int32_t   m_nDestinationXPos;
    int32_t   m_nDestinationYPos;

    double_t  m_dXPos;           // 정밀한 좌표값
    double_t  m_dYPos;           // 정밀한 좌표값 

    //Routing path 관련 변수
    int32_t   m_nUserIdentification;         // 이 객체를 소유한 상황도 클라이언트 Fd // TODO : 로그인 ID로 할까?
    int32_t   m_nCommunicationIdentification;// 이 객체를 소유한 하위 데이터전달처리 Fd 
public:
    void SetDestinationPos(int32_t xpos, int32_t ypos)
    {
        m_nDestinationXPos = xpos;
        m_nDestinationYPos = ypos;
    }
//시간의 따른 변화량 적용
    void SetCurrentPos(TimeType time) 
    {
        
        int32_t dx = m_nDestinationXPos - m_nXPos;
        int32_t dy = m_nDestinationYPos - m_nYPos;

        double_t dist = sqrt(dx * dx + dy * dy);
        if (dist == 0.0) return;

        double_t moveMeter = static_cast<double_t>(m_nVelocity) * time;
        double_t moveTile = moveMeter / 10.0;

        if (moveTile >= dist) {
            m_dXPos = m_nDestinationXPos;
            m_dYPos = m_nDestinationYPos;
        } else {
            double ratio = moveTile / dist;
            m_dXPos += dx * ratio;
            m_dYPos += dy * ratio;
        }

        // 정수 좌표로 갱신
        m_nXPos = static_cast<int32_t>(round(m_dXPos));
        m_nYPos = static_cast<int32_t>(round(m_dYPos));
    }
    //도착여부 판단.
    bool_t IsArriveAtDestination()
    {
        return (m_nXPos == m_nDestinationXPos) && (m_nYPos == m_nDestinationYPos);
    }
    void SetUserIdentification(int32_t nClientFd, int32_t nCommunicationFd)
    {
        m_nUserIdentification = nClientFd;
        m_nCommunicationIdentification = nCommunicationFd;
    }
};
// ********************************************************************
// * 클래스명: CArtillery
// * 설    명: 자주포의 정보를 담는 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class CArtillery
{
public:
    int16_t   m_nID;
    int_t     m_nXPos;
    int_t     m_nYPos;
    int_t     m_nVelocity;

    int32_t   m_nDestinationXPos;
    int32_t   m_nDestinationYPos;

    double_t  m_dXPos;           // 정밀한 좌표값
    double_t  m_dYPos;           // 정밀한 좌표값 

    int_t     m_nUnitHealth;
    int_t     m_nTotalMissile;   // 가지고 있는 미사일의 갯수

    std::mutex m_mutexQueue;
    std::queue<shared_ptr<CMissile>> m_vecMissileInfo;   //가지고있는 미사일

    //Routing path 관련 변수
    int32_t   m_nUserIdentification;         // 이 객체를 소유한 상황도 클라이언트 Fd // TODO : 로그인 ID로 할까?
    int32_t   m_nCommunicationIdentification;// 이 객체를 소유한 하위 데이터전달처리 Fd 
private:
    bool_t IsEmpty(){
        return m_vecMissileInfo.empty();
    }
public:
    bool_t SetDestinationMissile(int32_t xpos, int32_t ypos)
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return false;
        auto missile = m_vecMissileInfo.front();
        missile->SetDestinationPos(xpos,ypos);
        return true;
    }
    shared_ptr<CMissile> GetMissileFromQueue()
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return nullptr;
        auto missile = m_vecMissileInfo.front();
        m_vecMissileInfo.pop();
        return missile;
    }
    void PushMissileToQueue(shared_ptr<CMissile> strtMissile)
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        m_vecMissileInfo.push(strtMissile);
    }
    int_t GetNextMissileId()
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return -1;
        return m_vecMissileInfo.front()->m_nID;
    }
    void SetDestinationPos(int32_t xpos, int32_t ypos)
    {
        m_nDestinationXPos = xpos;
        m_nDestinationYPos = ypos;
    }
//시간의 따른 변화량 적용
    void SetCurrentPos(TimeType time) 
    {
        
        int32_t dx = m_nDestinationXPos - m_nXPos;
        int32_t dy = m_nDestinationYPos - m_nYPos;

        double_t dist = sqrt(dx * dx + dy * dy);
        if (dist == 0.0) return;

        double_t moveMeter = static_cast<double_t>(m_nVelocity) * time;
        double_t moveTile = moveMeter / 10.0;

        if (moveTile >= dist) {
            m_dXPos = m_nDestinationXPos;
            m_dYPos = m_nDestinationYPos;
        } else {
            double ratio = moveTile / dist;
            m_dXPos += dx * ratio;
            m_dYPos += dy * ratio;
        }

        // 정수 좌표로 갱신
        m_nXPos = static_cast<int32_t>(round(m_dXPos));
        m_nYPos = static_cast<int32_t>(round(m_dYPos));
    }
    //도착여부 판단.
    bool_t IsArriveAtDestination()
    {
        return (m_nXPos == m_nDestinationXPos) && (m_nYPos == m_nDestinationYPos);
    }
    void SetUserIdentification(int32_t nClientFd, int32_t nCommunicationFd)
    {
        m_nUserIdentification = nClientFd;
        m_nCommunicationIdentification = nCommunicationFd;
    }
};
// ********************************************************************
// * 클래스명: CMilitaryUnit
// * 설    명: 부대정보를 저장할 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class CMilitaryUnit
{
public:
    int16_t m_nID;
    int_t   m_nXPos;
    int_t   m_nYPos;

    int_t m_nUnitHealth;
    int_t m_nTotalArtillery;

    std::mutex m_mutexQueue;
    std::queue<shared_ptr<CArtillery>> m_vecArtilleryInfo;   //출격중인지여부

    //Routing path 관련 변수
    int32_t   m_nUserIdentification;         // 이 객체를 소유한 상황도 클라이언트 Fd // TODO : 로그인 ID로 할까?
    int32_t   m_nCommunicationIdentification;// 이 객체를 소유한 하위 데이터전달처리 Fd 
private:
    bool_t IsEmpty(){
        return m_vecArtilleryInfo.empty();
    }
public:
    bool_t SetDestinationArtillery(int32_t xpos, int32_t ypos)
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return false;
        auto artillery = m_vecArtilleryInfo.front();
        artillery->SetDestinationPos(xpos,ypos);
        return true;
    }
    shared_ptr<CArtillery> GetArtilleryFromQueue()
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return nullptr;
        auto artillery = m_vecArtilleryInfo.front();
        m_vecArtilleryInfo.pop();
        return artillery;
    }
    std::vector<int_t> GetAllIdForRemove()
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        std::vector<int_t> vecArtilleryId;
        while(!IsEmpty())
        {
            auto artillery = m_vecArtilleryInfo.front();
            m_vecArtilleryInfo.pop();
            vecArtilleryId.push_back(artillery->m_nID);
        }
        return vecArtilleryId;
    }
    void PushArtilleryToQueue(shared_ptr<CArtillery> strtArtillery)
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        m_vecArtilleryInfo.push(strtArtillery);
    }
    int16_t GetNextSallyArtilleryId()
    {
        std::lock_guard<std::mutex> lockQueue(m_mutexQueue);
        //////////////////////////////////////////////////////////////////critical section
        if (IsEmpty()) return -1;
        return m_vecArtilleryInfo.front()->m_nID;
    }
    void SetUserIdentification(int32_t nClientFd, int32_t nCommunicationFd)
    {
        m_nUserIdentification = nClientFd;
        m_nCommunicationIdentification = nCommunicationFd;
    }
};
