// ********************************************************************
// * 소스정의: MilitaryCoupled.h
// * 설    명: 객체 아토믹 모델을 연결하는 커플드모델을 정의하고 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
#pragma once

#include "Coupled.h"
#include "MilitaryUnitModel.h"
#include "ArtilleryModel.h"
#include "MissileModel.h"
#include "UnitMapModel.h"
#include "TypeDef.h"
#include "ScenarioControl.h"
#include "ModelStruct.h"
#include "Global.h"

#include <map>

using namespace DEVSIM;
using namespace ModelConfig::Content;
using namespace ModelConfig::Port;
using namespace ModelConfig::State;

// ********************************************************************
// * 클래스명: MilitaryCoupledModel
// * 설    명: 객체 아토믹 모델을 연결하는 커플드모델을 정의하고 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
class MilitaryCoupledModel : public Coupled {
private:
    std::map<int16_t, std::shared_ptr<CMilitaryUnit>> m_mMilitaryUnitInfo;
    std::map<int_t, std::shared_ptr<CArtillery>> m_mArtilleryInfo;
    std::map<int_t, std::shared_ptr<CMissile>> m_mMissileInfo; 

    std::shared_ptr<UnitMapModel> m_pUnitMapModel;
public:

// ********************************************************************
// * 함 수 명: MilitaryCoupledModel
// * 설    명: 생성자. 연관 모델들의 포트연결을 담당한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    MilitaryCoupledModel(const string& name) : Coupled(name) {

        //auto& ScenarioControl = ScenarioControl::GetInstance();

        //const auto& mapArtillery = ScenarioControl.GetArtilleryMap();
        //const auto& mapMilitaryUnit = ScenarioControl.GetMilitaryUnitMap();
        
        //시나리오 파일로 부터 객체의 정보를 저장해야한다.
        auto pUnitMapModel = std::make_shared<UnitMapModel>("UnitMapModel");

        m_pUnitMapModel = pUnitMapModel;
        AddComponent(pUnitMapModel);

        //외부 포트 연결 external → pUnitMapModel IN
        AddCoupling(nullptr, P_mic_IN, pUnitMapModel, P_map_IN);

        //아군
        /*
        for (int_t i=1;i<50;++i)
        {
            SetMilitaryUnitPortConnect((int16_t)i,127073+10*i,37526,99);

        }
        //SetMilitaryUnitPortConnect((int16_t)1,127073,37526,99,pUnitMapModel);

        //적군
        for (int_t i=50;i<100;++i)
        {
            SetMilitaryUnitPortConnect((int16_t)i,125762+i,39039,99);

        }
        */
        //SetMilitaryUnitPortConnect((int16_t)50,125762,39039,99,pUnitMapModel);
    }

// ********************************************************************
// * 함 수 명: GetAllMilitaryUnit
// * 설    명: 부대정보를 들고있는 모든 militaryUnit Id을 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    std::vector<int16_t> GetAllMilitaryUnit()
    {
        std::vector<int16_t> vecMilitaryId;
        for(auto e : m_mMilitaryUnitInfo)
        {
            vecMilitaryId.push_back(e.first);
        }
        return vecMilitaryId;
    }
// ********************************************************************
// * 함 수 명: GetMilitaryUnit
// * 설    명: 부대정보를 들고있는 militaryUnit model의 쉐어드 포인터를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    bool_t GetMilitaryUnit(int16_t nId, CMilitaryUnit* pMilitaryUnit)
    {
        if (m_mMilitaryUnitInfo.find(nId) == m_mMilitaryUnitInfo.end()) return false;
        auto cMilitaryUnitInfo = m_mMilitaryUnitInfo[nId];
        pMilitaryUnit->m_nID = cMilitaryUnitInfo->m_nID;
        pMilitaryUnit->m_nTotalArtillery = cMilitaryUnitInfo->m_nTotalArtillery;
        pMilitaryUnit->m_nXPos = cMilitaryUnitInfo->m_nXPos;
        pMilitaryUnit->m_nYPos = cMilitaryUnitInfo->m_nYPos;
        pMilitaryUnit->m_nUnitHealth = cMilitaryUnitInfo->m_nUnitHealth;
        pMilitaryUnit->m_vecArtilleryInfo = cMilitaryUnitInfo->m_vecArtilleryInfo;

        pMilitaryUnit->m_nUserIdentification = cMilitaryUnitInfo->m_nUserIdentification;
        pMilitaryUnit->m_nCommunicationIdentification = cMilitaryUnitInfo->m_nCommunicationIdentification;
        return true;
    }
// ********************************************************************
// * 함 수 명: IsDeadFromMilitaryUnit
// * 설    명: military unit id를 받아 부대가 destroy했는지 여부를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
    bool_t IsDeadFromMilitaryUnit(int_t nId)
    {
        if (m_mMilitaryUnitInfo.find(nId) == m_mMilitaryUnitInfo.end()) return false;
        return m_mMilitaryUnitInfo[nId]->m_nUnitHealth <= 0;
    }

// ********************************************************************
// * 함 수 명: GetNextSallyArtillery
// * 설    명: 다음 출격 자주포의 아이디를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    int_t GetNextSallyArtillery(int16_t nId)
    {
        if (m_mMilitaryUnitInfo.find(nId) == m_mMilitaryUnitInfo.end()) return -1;
        return m_mMilitaryUnitInfo[nId]->GetNextSallyArtilleryId();
    }

// ********************************************************************
// * 함 수 명: GetArtillery
// * 설    명: 자주포정보를 들고있는 artillery model의 쉐어드 포인터를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    bool_t GetArtillery(int_t nId, CArtillery* pArtillery)
    {
        if (m_mArtilleryInfo.find(nId) == m_mArtilleryInfo.end()) return false;
        auto cArtilleryInfo = m_mArtilleryInfo[nId];
        pArtillery->m_nID = cArtilleryInfo->m_nID;
        pArtillery->m_nXPos = cArtilleryInfo->m_nXPos;
        pArtillery->m_nYPos = cArtilleryInfo->m_nYPos;
        pArtillery->m_nUnitHealth = cArtilleryInfo->m_nUnitHealth;
        pArtillery->m_nTotalMissile = cArtilleryInfo->m_nTotalMissile;

        pArtillery->m_nUserIdentification = cArtilleryInfo->m_nUserIdentification;
        pArtillery->m_nCommunicationIdentification = cArtilleryInfo->m_nCommunicationIdentification;
        return true;
    }
// ********************************************************************
// * 함 수 명: IsArrivedFromArtillery
// * 설    명: 자주포 id를 받아 자주포가 현재 목적지에 도착했는지 여부를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
    bool_t IsArrivedFromArtillery(int_t nId)
    {
        if (m_mArtilleryInfo.find(nId) == m_mArtilleryInfo.end()) return false;
        return m_mArtilleryInfo[nId]->IsArriveAtDestination();
    }
// ********************************************************************
// * 함 수 명: IsDeadFromArtillery
// * 설    명: 자주포 id를 받아 자주포가 destroy했는지 여부를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
    bool_t IsDeadFromArtillery(int_t nId)
    {
        if (m_mArtilleryInfo.find(nId) == m_mArtilleryInfo.end()) return false;
        return m_mArtilleryInfo[nId]->m_nUnitHealth <= 0;
    }

// ********************************************************************
// * 함 수 명: GetNextMissile
// * 설    명: 다음 발사 미사일의 아이디를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
    int_t GetNextMissile(int16_t nId)
    {
        if (m_mArtilleryInfo.find(nId) == m_mArtilleryInfo.end()) return -1;
        return m_mArtilleryInfo[nId]->GetNextMissileId();
    }

// ********************************************************************
// * 함 수 명: GetMissile
// * 설    명: 미사일정보를 들고있는 missile model의 쉐어드 포인터를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
    bool_t GetMissile(int_t nId, CMissile* pMissile)
    {
        if (m_mMissileInfo.find(nId) == m_mMissileInfo.end()) return false;
        auto cMissileInfo = m_mMissileInfo[nId];
        pMissile->m_nID = cMissileInfo->m_nID;
        pMissile->m_nXPos = cMissileInfo->m_nXPos;
        pMissile->m_nYPos = cMissileInfo->m_nYPos;

        pMissile->m_nUserIdentification = cMissileInfo->m_nUserIdentification;
        pMissile->m_nCommunicationIdentification = cMissileInfo->m_nCommunicationIdentification;
        return true;
    }
// ********************************************************************
// * 함 수 명: InitMissilePos
// * 설    명: 미사일정보의 위치정보를 업데이트 한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
    bool_t InitMissilePos(int_t nId, int32_t xpos, int32_t ypos)
    {
        if (m_mMissileInfo.find(nId) == m_mMissileInfo.end()) return false;
        auto cMissileInfo = m_mMissileInfo[nId];
        cMissileInfo->m_nXPos = xpos;
        cMissileInfo->m_nYPos = ypos;
        cMissileInfo->m_dXPos = static_cast<double_t>(xpos);
        cMissileInfo->m_dYPos = static_cast<double_t>(ypos);

        return true;
    }

// ********************************************************************
// * 함 수 명: IsArrivedFromMissile
// * 설    명: missile id를 받아 미사일이 현재 목적지에 도착했는지 여부를 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
    bool_t IsArrivedFromMissile(int_t nId)
    {
        if (m_mMissileInfo.find(nId) == m_mMissileInfo.end()) return false;
        return m_mMissileInfo[nId]->IsArriveAtDestination();
    }

// ********************************************************************
// * 함 수 명: SetUserIdentificationToMilitaryUnit
// * 설    명: 전장상황도의 클라이언트 식별자를 저장한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    bool_t SetUserIdentificationToMilitaryUnit(int32_t nClientFd, int32_t nCommunicationFd, int16_t nMilitaryUnitId)
    {
        if (m_mMilitaryUnitInfo.find(nMilitaryUnitId) == m_mMilitaryUnitInfo.end()) return false;
        m_mMilitaryUnitInfo[nMilitaryUnitId]->SetUserIdentification(nClientFd,nCommunicationFd);
        return true;
    }

// ********************************************************************
// * 함 수 명: SetUserIdentificationToArtillery
// * 설    명: 전장상황도의 클라이언트 식별자를 저장한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 03
// ********************************************************************
    bool_t SetUserIdentificationToArtillery(int32_t nClientFd, int32_t nCommunicationFd, int16_t nArtilleryId)
    {
        if (m_mArtilleryInfo.find(nArtilleryId) == m_mArtilleryInfo.end()) return false;
        m_mArtilleryInfo[nArtilleryId]->SetUserIdentification(nClientFd,nCommunicationFd);
        return true;
    }

// ********************************************************************
// * 함 수 명: SetUserIdentificationToMissile
// * 설    명: 전장상황도의 클라이언트 식별자를 저장한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 08
// ********************************************************************
    bool_t SetUserIdentificationToMissile(int32_t nClientFd, int32_t nCommunicationFd, int_t nMissileId)
    {
        if (m_mMissileInfo.find(nMissileId) == m_mMissileInfo.end()) return false;
        m_mMissileInfo[nMissileId]->SetUserIdentification(nClientFd,nCommunicationFd);
        return true;
    }

// ********************************************************************
// * 함 수 명: FindIncludeAttackedObject
// * 설    명: 주어진 좌표값에 포함되는 객체들을 찾고 Attacked이벤트를
// *           보낸다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
    std::vector<Message> FindIncludeAttackedObject(const int32_t nXPos, const int32_t nYPos, const int32_t scope = 200)
    {
        std::vector<Message> vecMessage;
        //TODO : 지금은 모든 객체를 탐색하지만, 나중에는 격자안의 객체만 탐색하도록 바꾸자.
        for (auto artillery : m_mArtilleryInfo)
        {
            int32_t xpos = artillery.second->m_nXPos;
            int32_t ypos = artillery.second->m_nYPos;
            
            //피격범위 판단
            if (xpos <nXPos-scope || xpos > nXPos+scope) continue;
            if (ypos <nYPos-scope || ypos > nYPos+scope) continue;

            Message msgAttackedByMissile;
            msgAttackedByMissile.SetPort(P_main_IN);
            msgAttackedByMissile.SetContent(C_art_ATTACKED);
            msgAttackedByMissile.SetDetail(ObjectID::ARTILLERY,artillery.first);
            vecMessage.push_back(msgAttackedByMissile);
        }

        for (auto militaryUnit : m_mMilitaryUnitInfo)
        {
            int32_t xpos = militaryUnit.second->m_nXPos;
            int32_t ypos = militaryUnit.second->m_nYPos;
            
            //피격범위 판단
            if (xpos <nXPos-scope || xpos > nXPos+scope) continue;
            if (ypos <nYPos-scope || ypos > nYPos+scope) continue;

            Message msgAttackedByMissile;
            msgAttackedByMissile.SetPort(P_main_IN);
            msgAttackedByMissile.SetContent(C_mil_ATTACKED);
            msgAttackedByMissile.SetDetail(ObjectID::MILITARY_UNIT,militaryUnit.first);
            vecMessage.push_back(msgAttackedByMissile);
        }

        return vecMessage;
    }

// ********************************************************************
// * 함 수 명: RemoveMilitaryUnitInfo
// * 설    명: map에 저장된 부대정보를 지운다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
    void RemoveMilitaryUnitInfo(int_t nMilitaryUnitId)
    {
        if (m_mMilitaryUnitInfo.find(nMilitaryUnitId) == m_mMilitaryUnitInfo.end()) return;
        auto militaryUnit = m_mMilitaryUnitInfo[nMilitaryUnitId];
        auto vecArtillery = militaryUnit->GetAllIdForRemove();

        for (auto artillery : vecArtillery)
        {
            RemoveArtilleryInfo(artillery);
        }

    }
// ********************************************************************
// * 함 수 명: RemoveArtilleryInfo
// * 설    명: map에 저장된 부대정보를 지운다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 11
// ********************************************************************
    void RemoveArtilleryInfo(int_t nArtilleryId)
    {
        if (m_mArtilleryInfo.find(nArtilleryId) == m_mArtilleryInfo.end()) return;
        m_mArtilleryInfo.erase(nArtilleryId);
    }
public:
// ********************************************************************
// * 함 수 명: SetMilitaryUnitPortConnect
// * 설    명: military unit의 포트연결을 설정한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
    void SetMilitaryUnitPortConnect(int16_t nMilitaryUnitId,int_t xpos,int_t ypos,int_t nTotalArtillery/*, shared_ptr<UnitMapModel> pUnitMapModel*/)
    {
        auto pMilitaryUnitInfo = std::make_shared<CMilitaryUnit>();
        pMilitaryUnitInfo->m_nID = nMilitaryUnitId;
        pMilitaryUnitInfo->m_nXPos = xpos;
        pMilitaryUnitInfo->m_nYPos = ypos;
        pMilitaryUnitInfo->m_nUnitHealth = 100;
        pMilitaryUnitInfo->m_nTotalArtillery = nTotalArtillery;
        //pMilitaryUnitInfo->PushArtilleryToQueue(pArtilleryInfo);

        m_mMilitaryUnitInfo[pMilitaryUnitInfo->m_nID] = pMilitaryUnitInfo;
        auto pMilitaryUnit = std::make_shared<MilitaryUnitModel>("MilitaryUnit0"+to_string(nMilitaryUnitId),pMilitaryUnitInfo);
        AddComponent(pMilitaryUnit);

        // Connect pUnitMapModel OUT → Unit
        AddCoupling(m_pUnitMapModel, P_map_OUT+to_string(pMilitaryUnitInfo->m_nID), pMilitaryUnit, P_mil_IN);
        // 외부 아웃포트와 연결
        AddCoupling(pMilitaryUnit, P_mil_ATTACKED_OUTPUT, nullptr, P_mic_OUT);
        AddCoupling(pMilitaryUnit, P_mil_DESTROY_OUTPUT, nullptr, P_mic_OUT);

        //총 자주포 수는 99개가 최대값이라고 가정
        if (nTotalArtillery > 99) nTotalArtillery = 99;

        for (int_t i = 1; i <= nTotalArtillery; ++i)
        {
            auto pArtilleryInfo = std::make_shared<CArtillery>();
            pArtilleryInfo->m_nID = nMilitaryUnitId*100+i;
            pArtilleryInfo->m_nVelocity = 200;
            pArtilleryInfo->m_nXPos = xpos;
            pArtilleryInfo->m_nYPos = ypos;
            pArtilleryInfo->m_dXPos = static_cast<double_t>(pArtilleryInfo->m_nXPos);
            pArtilleryInfo->m_dYPos = static_cast<double_t>(pArtilleryInfo->m_nYPos);
            pArtilleryInfo->m_nUnitHealth = 100;
            pArtilleryInfo->m_nTotalMissile = 10;

            m_mArtilleryInfo[pArtilleryInfo->m_nID] = pArtilleryInfo;
            pMilitaryUnitInfo->PushArtilleryToQueue(pArtilleryInfo);
            auto pArtillery = std::make_shared<ArtilleryModel>("Artillery0"+to_string(pArtilleryInfo->m_nID),pArtilleryInfo);
            AddComponent(pArtillery);
            // Connect pUnitMapModel OUT → Unit
            AddCoupling(m_pUnitMapModel, P_map_OUT+to_string(pArtilleryInfo->m_nID),pArtillery, P_art_IN);
            // Connect pMillitaryUnit OUT → pArtillery IN
            AddCoupling(pMilitaryUnit, P_mil_OUT+to_string(pArtilleryInfo->m_nID), pArtillery, P_art_IN);
            // 외부 아웃포트와 연결 (유저에게 결과를 전송하기 위한 이벤트)
            AddCoupling(pArtillery, P_art_ATTACKED_OUTPUT, nullptr, P_mic_OUT);
            AddCoupling(pArtillery, P_art_DESTROY_OUTPUT, nullptr, P_mic_OUT);
            AddCoupling(pArtillery, P_art_MOVED_OUTPUT,nullptr, P_mic_OUT);

            for (int_t j = 0; j < 10; ++j)
            {
                auto pMissileInfo = std::make_shared<CMissile>();
                pMissileInfo->m_nID = (int)(pArtilleryInfo->m_nID)*10+j;
                pMissileInfo->m_nVelocity = 1000;
                pMissileInfo->m_nXPos = xpos;
                pMissileInfo->m_nYPos = ypos;
                pMissileInfo->m_dXPos = static_cast<double_t>(pMissileInfo->m_nXPos);
                pMissileInfo->m_dYPos = static_cast<double_t>(pMissileInfo->m_nYPos);

                m_mMissileInfo[pMissileInfo->m_nID] = pMissileInfo;
                pArtilleryInfo->PushMissileToQueue(pMissileInfo);
                auto pMissile = std::make_shared<MissileModel>("Missile"+to_string(pMissileInfo->m_nID),pMissileInfo);
                AddComponent(pMissile);
                // Connect pUnitMapModel OUT → Unit
                AddCoupling(m_pUnitMapModel, P_map_OUT+to_string(pMissileInfo->m_nID),pMissile, P_mis_IN);
                // Connect pArtillery OUT → pMissile IN
                AddCoupling(pArtillery, P_art_OUT+to_string(pMissileInfo->m_nID), pMissile, P_mis_IN);
                // 외부 아웃포트와 연결 (유저에게 결과를 전송하기 위한 이벤트)
                AddCoupling(pMissile, P_mis_DESTROY_OUTPUT, nullptr, P_mic_OUT);
                AddCoupling(pMissile, P_mis_MOVED_OUTPUT, nullptr, P_mic_OUT);
            }
        } 
    }
};
