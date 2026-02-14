// ********************************************************************
// * 헤더정의: Coupling.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#include "Coupling.h"
#include <algorithm>

using namespace DEVSIM;

// ****************************************************************
// * 함 수 명: ~Coupling
// * 설    명: Coupling class를 닫는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
Coupling::~Coupling() 
{
    clear();
}

// ****************************************************************
// * 함 수 명: AddCoupling
// * 설    명: Coupling 을 추가한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::AddCoupling(String srcport, pModel dstmodel, String dstport) 
{
    m_CouplingList.insert(std::make_pair(srcport, std::make_unique<CouplingElem>(dstmodel,dstport)));
}

// ****************************************************************
// * 함 수 명: GetInfluence
// * 설    명: out 모델의 어떤 port와 연결되는 모든 커플링 정보를 
// *           read-only iterator형태로 반환한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
Coupling::CouplingMapIterPair Coupling::GetInfluence(String port) const 
{
    return m_CouplingList.equal_range(port);
}

// ****************************************************************
// * 함 수 명: RemoveCoupling
// * 설    명: Coupling 을 끊는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::RemoveCoupling(pModel destination) 
{
    for (auto it = m_CouplingList.begin(); it != m_CouplingList.end(); ) 
    {
        if (it->second->m_pToModel == destination) 
        {
            it = m_CouplingList.erase(it);
        }
        else 
        {
            ++it;
        }
    }
}

// ****************************************************************
// * 함 수 명: RemoveCoupling
// * 설    명: Coupling 을 끊는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::RemoveCoupling(String srcport, pModel dstmodel, String dstport) 
{
    auto range = m_CouplingList.equal_range(string(srcport));
    for (auto it = range.first; it != range.second; ) {
        if (it->second->m_pToModel == dstmodel && it->second->m_strToPort == dstport) {
            it = m_CouplingList.erase(it);
        } else {
            ++it;
        }
    }
}

// ****************************************************************
// * 함 수 명: RemoveCoupling
// * 설    명: Coupling 을 끊는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::RemoveCoupling(String srcport) 
{
    m_CouplingList.erase(srcport);
}

// ****************************************************************
// * 함 수 명: ~Coupling
// * 설    명: Coupling class를 닫는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::GetAllCouplingInfo(CouplingInfoVector& vec) 
{
    for (const auto& pair : m_CouplingList) 
    {
        vec.push_back(std::make_unique<CouplingInfo>(m_pModel, pair.first, pair.second->m_pToModel, pair.second->m_strToPort));
    }
}

// ****************************************************************
// * 함 수 명: ~Coupling
// * 설    명: Coupling class를 닫는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
void Coupling::clear() 
{
    m_CouplingList.clear();
}
