// ********************************************************************
// * 헤더정의: ScenarioControl.h
// * 설    명: 시나리오를 불러오는 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 29
// ********************************************************************
#include "TypeDef.h"
#include "MACRO.h"
#include "Singleton.h"
#include "ModelStruct.h"

#include <map>
#include <queue>

// ****************************************************************
// * 클래스명: ScenarioControl
// * 설    명: 시나리오를 불러오고 맞는 객체를 초기화하는 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 29
// ****************************************************************
class ScenarioControl : public Singleton<ScenarioControl>{
private:
    ///////////////////////////////시뮬레이션에 필요한 객체들
    std::map<int_t,int_t> m_mapUserInfo;    //객체id,userid로 이뤄짐.
    //객체들을 소유하고 있는 userid를 저장한다.

    std::map<int_t,std::shared_ptr<CArtillery>> m_mapArtillery;
    std::map<int_t,std::shared_ptr<CMilitaryUnit>> m_mapMilitaryUnit;  
public:
    ScenarioControl();
    ~ScenarioControl();

    void InitalizeScenarioControl();
    void LoadBasicScenario()
    {

        auto pArtilleryInfo = std::make_shared<CArtillery>();
        pArtilleryInfo->m_nID = 101;
        pArtilleryInfo->m_nXPos = 0;
        pArtilleryInfo->m_nYPos = 0;
        pArtilleryInfo->m_nUnitHealth = 100;

        auto pMilitaryUnitInfo = std::make_shared<CMilitaryUnit>();
        pMilitaryUnitInfo->m_nID = 1;
        pMilitaryUnitInfo->m_nXPos = 0;
        pMilitaryUnitInfo->m_nYPos = 0;
        pMilitaryUnitInfo->m_nUnitHealth = 100;
        pMilitaryUnitInfo->m_nTotalArtillery = 1;
        pMilitaryUnitInfo->PushArtilleryToQueue(pArtilleryInfo);

        m_mapArtillery[pArtilleryInfo->m_nID] = pArtilleryInfo;
        m_mapMilitaryUnit[pMilitaryUnitInfo->m_nID] = pMilitaryUnitInfo;
    }
    const std::map<int_t,std::shared_ptr<CArtillery>>& GetArtilleryMap() const
    {
        return m_mapArtillery;
    }
    const std::map<int_t,std::shared_ptr<CMilitaryUnit>>& GetMilitaryUnitMap() const
    {
        return m_mapMilitaryUnit;
    }

};