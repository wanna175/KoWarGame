// ********************************************************************
// * 소스정의: Coupled.cpp
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ********************************************************************
#include "Coupled.h"


#include <string>
#include <set>
#include <memory>
#include <map>
#include <cstdarg>

using std::string;
using std::shared_ptr;
using std::unique_ptr;

using namespace DEVSIM;

typedef const string& String;
typedef shared_ptr<Model> pModel;
typedef std::set<shared_ptr<Model>> ModelVector;
typedef std::map<shared_ptr<Model>, unique_ptr<Coupling>> CouplingMap;


// ****************************************************************
// * 함 수 명: Coupled
// * 설    명: Coupled 클래스의 생성자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
Coupled::Coupled(String name) : Model(name)
{
    m_Type = COUPLED;
}
// ****************************************************************
// * 함 수 명: ~Coupled
// * 설    명: Coupled 클래스의 소멸자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
Coupled::~Coupled()
{
    m_ChildList.clear(); // shared_ptr이므로 자동 소멸
    m_ThisCoupling.clear();
    for (auto& pair : m_ChildCoupling) 
    {
        pair.second->clear();
    }
    m_ChildCoupling.clear();
}
   
// ****************************************************************
// * 함 수 명: AddComponent
// * 설    명: child모델을 등록한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::AddComponent(pModel model)
{
    if (m_ChildList.find(model) != m_ChildList.end()) 
    {
        ERROR("AddComponent() : Already exist model");
        return false;
    }
    m_ChildList.insert(model);
    return true;
}

// ****************************************************************
// * 함 수 명: GetComponent
// * 설    명: Model의 이름으로 등록된 Model을 가져온다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
pModel Coupled::GetComponent(String name)
{
    for (const auto& model : m_ChildList) 
    {
        if (model->GetName() == name) return model;
    }
    return nullptr;
}

// ****************************************************************
// * 함 수 명: ExistComponent
// * 설    명: 등록된 모델이 있는지 확인한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::ExistComponent(pModel model)
{
    return m_ChildList.find(model) != m_ChildList.end();
}

// ****************************************************************
// * 함 수 명: NumberComponent
// * 설    명: 하위모델들의 갯수를 반환한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
int_t Coupled::NumberComponent() 
{
    return m_ChildList.size(); 
}

// ****************************************************************
// * 함 수 명: RemoveComponet
// * 설    명: 하위 모델을 제거한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::RemoveComponent(pModel model)
{
    if (!model) return false;

    m_ThisCoupling.RemoveCoupling(model);

    auto it = m_ChildCoupling.find(model);
    if (it != m_ChildCoupling.end()) 
    {
        it->second->clear();
        m_ChildCoupling.erase(it);
    }

    for (auto& pair : m_ChildCoupling) 
    {
        pair.second->RemoveCoupling(model);
    }

    return m_ChildList.erase(model) > 0;
}


// ****************************************************************
// * 함 수 명: AddCoupling
// * 설    명: Coupling정보를 등록한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::AddCoupling(pModel src_model, String src_port, pModel dst_model, String dst_port)
{
    //Coupled모델의 coupling 정보라면
    if (src_model == nullptr) 
    {
        m_ThisCoupling.AddCoupling(src_port, dst_model, dst_port);
    } 
    else 
    {
        auto& coupling = m_ChildCoupling[src_model];
        if (!coupling) coupling = std::make_unique<Coupling>(src_model);
        coupling->AddCoupling(src_port, dst_model, dst_port);
    }
    return true;
}

// ****************************************************************
// * 함 수 명: RemoveCoupling
// * 설    명: Coupling정보를 삭제한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::RemoveCoupling(pModel src_model, String src_port, pModel dst_model, String dst_port)
{

    if (src_model == nullptr) {
        m_ThisCoupling.RemoveCoupling(src_port, dst_model, dst_port);
    } else {
        auto it = m_ChildCoupling.find(src_model);
        if (it != m_ChildCoupling.end())
            it->second->RemoveCoupling(src_port, dst_model, dst_port);
    }
    return true;
}

// ****************************************************************
// * 함 수 명: RemoveCoupling
// * 설    명: Coupling정보를 삭제한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
bool_t Coupled::RemoveCoupling(pModel src_model, String src_port)
{
    if (src_model == nullptr) {
        m_ThisCoupling.RemoveCoupling(src_port);
    } else {
        auto it = m_ChildCoupling.find(src_model);
        if (it != m_ChildCoupling.end())
            it->second->RemoveCoupling(src_port);
    }
    return true;
}

// ****************************************************************
// * 함 수 명: Coupled
// * 설    명: Coupled 클래스의 생성자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
// bool_t Coupled::SetPriority(int_t num, ...)
// {

// }

//void SetDeleteChildModel( bool flag ) { m_bDeleteChildModel = flag; }

// ****************************************************************
// * 함 수 명: Coupled
// * 설    명: Coupled 클래스의 생성자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
void Coupled::GetAllComponents(ModelVector& models)
{
    for (const auto& m : m_ChildList) 
    {
        models.insert(m);
    }
}

// ****************************************************************
// * 함 수 명: Coupled
// * 설    명: Coupled 클래스의 생성자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
Coupling& Coupled::GetThisCoupling() 
{
    return m_ThisCoupling; 
}

// ****************************************************************
// * 함 수 명: Coupled
// * 설    명: Coupled 클래스의 생성자를 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 20
// ****************************************************************
CouplingMap& Coupled::GetChildCoupling() 
{
    return m_ChildCoupling; 
}


