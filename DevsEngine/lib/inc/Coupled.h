// ********************************************************************
// * 헤더정의: Coupled.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include "Model.h"
#include "Coupling.h"
#include "TypeDef.h"
#include "TimeType.h"
#include "StateVariable.h"
#include "Message.h"
#include "MACRO.h"

#include <string>
#include <set>
#include <memory>
#include <map>

using std::string;
using std::shared_ptr;
using std::unique_ptr;

namespace DEVSIM{

typedef struct ModelInfo{
	string 			  m_strName;
	shared_ptr<Model> m_Model;
}StModelInfo;
// ****************************************************************
// * 클래스명: Coupled
// * 설    명: CoupledModel을 정의한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 19
// ****************************************************************
class Coupled : public Model{
public:
	typedef const string& String;
	typedef shared_ptr<Model> pModel;
    typedef std::set<shared_ptr<Model>> ModelVector;
    typedef std::map<shared_ptr<Model>, unique_ptr<Coupling>> CouplingMap;
protected:
	ModelVector		m_ChildList;		//하위 모델들.
	Coupling 		m_ThisCoupling;		//External Input Coupling 이 경우 out model은 nullptr이다.
	CouplingMap 	m_ChildCoupling;	//Internal Coupling, Internal Output Coupling
										//만일 IOC 인 경우 toModel은 nullptr이라고 하자.
	
public:
    Coupled(String name);
    virtual ~Coupled();

    bool_t AddComponent(pModel model);
	pModel GetComponent(String name);
	
	bool_t ExistComponent(pModel model);
	int_t NumberComponent();


	bool_t RemoveComponent(pModel model);

	bool_t AddCoupling(pModel src_model, String src_port, pModel dst_model, String dst_port);

	bool_t RemoveCoupling(pModel src_model, String src_port, pModel dst_model, String dst_port);
	bool_t RemoveCoupling(pModel src_model, String src_port);

	//bool_t SetPriority(int_t num, ...);
	
	//void SetDeleteChildModel( bool flag ) { m_bDeleteChildModel = flag; }

	void GetAllComponents( ModelVector& models );
	Coupling&		GetThisCoupling();
	CouplingMap&	GetChildCoupling();

};
}