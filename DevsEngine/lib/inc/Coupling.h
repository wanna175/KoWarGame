// ********************************************************************
// * 헤더정의: Coupling.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************
#pragma once

#include "Model.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

using std::string;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;

namespace DEVSIM{

// ****************************************************************
// * 클래스명: CouplingInfo
// * 설    명: Flatterning 된 커플링 정보를 저장하는 클래스 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class CouplingInfo
{
public:
	typedef const string& String;
	typedef shared_ptr<Model> pModel;
public:
	CouplingInfo(pModel s_model,String s_port, pModel d_model, String d_port )
	{
		m_pFromModel = s_model;
		m_strFromPort = s_port;
		m_pToModel = d_model;
		m_strToPort = d_port;
	}

public:
	pModel					m_pFromModel;			//out 모델이름
	string					m_strFromPort;			//out 포트
	pModel					m_pToModel;			//in 모델이름
	string					m_strToPort;			//in 포트
};
typedef std::vector<unique_ptr<CouplingInfo>> CouplingInfoVector;


// ****************************************************************
// * 클래스명: CouplingElem
// * 설    명: Coupling요소를 정의하는 클래스 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class  CouplingElem 
{
public:
	typedef const string& String;
	typedef shared_ptr<Model> pModel;
public:
	pModel 				m_pToModel;		//in 모델
	string 				m_strToPort;			//in 포트

public:
	CouplingElem(pModel name, String port) : m_pToModel(name), m_strToPort(port){}
};

// ****************************************************************
// * 클래스명: Coupling
// * 설    명: 커플링 정보를 담는 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class Coupling 
{
public:
	typedef const string& String;
	typedef shared_ptr<Model> pModel;
	typedef std::multimap< string, unique_ptr<CouplingElem>> CouplingMap;			// <out 포트, in < 모델, 포트 >> 
	typedef std::pair< CouplingMap::const_iterator, CouplingMap::const_iterator > CouplingMapIterPair;

	pModel						m_pModel;			//  out 모델
	CouplingMap					m_CouplingList;		// <out 포트, in < 모델, 포트 >> 

public:
	Coupling() {m_pModel =  nullptr;}
	Coupling(pModel model) {m_pModel = model;} 
	~Coupling();

    void AddCoupling(String  srcport, pModel  dstmodel, const String  dstport);
    void SetModel(pModel model) {m_pModel = model;}
	CouplingMapIterPair GetInfluence(String) const;

	void RemoveCoupling(pModel destination);	// Remove coupling of destination model
    void RemoveCoupling(String srcport, pModel dstmodel, String dstport);
    void RemoveCoupling(String srcport);

	// Flatterning
	void GetAllCouplingInfo(CouplingInfoVector& vec);
	void clear();
};

}