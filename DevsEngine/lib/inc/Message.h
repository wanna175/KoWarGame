// ********************************************************************
// * 헤더정의: Message.h 
// * 설    명: 프로세서간 통신을 위한 메세지 클래스 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ********************************************************************		

#pragma once

#include "TimeType.h"
#include <map>
#include <string>

using std::string;

namespace DEVSIM{
	namespace MSG_TYPE{
		enum MSG_TYPE{
			X_MSG,
			Y_MSG,
			STAR_MSG,
			DONE_MSG
		};
	}

typedef struct _MessageDetail{
	int8_t  m_nObjType;	
	int_t   m_nObjID;
	int32_t m_nDstXpos;
	int32_t m_nDstYpos;
}StDetail;
// ****************************************************************
// * 클래스명: Message
// * 설    명: 실행기가 주고 받는 메세지를 정의한 클래스
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ****************************************************************
class Message {
protected:
	string				m_strSourceName;	// Name to the source model.
	string				m_strPort;			// Port Name.
    string              m_strContent;   	// TODO : Content 필요하다면 클래스로 정의
	TimeType			m_Time;				// 이벤트 발생 시간, Done일 경우 다음 이벤트 발생 시간.
	StDetail			m_strtDetail;		// 메세지 디테일
	int_t				m_Type;				// msg type
	bool_t  			m_bNetFlag;			// tcp로 보낼시 true, udp로 보낼시 false	
public:
	Message();
	//Message(Message &msg);
	Message(const string& name, const string& port, TimeType time, int_t type);

	virtual ~Message();

// ********************************************************************
// * 함수정의: Get/Set
// * 설    명: 멤버 변수에 접근하는 getter, setter 함수 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
	// SourceName
	void SetSourceName(const string& name) {m_strSourceName = name;}
	string GetSourceName() const {return m_strSourceName;}

	// Port
	void SetPort(const string& port) {m_strPort = port;}
	string GetPort() const {return m_strPort;}

	// Content
	void SetContent(const string& content) {m_strContent = content;}
	string GetContent() const { return m_strContent;}
	
	// Time
	void SetTime(TimeType time) {m_Time = time;}
	TimeType GetTime() const {return m_Time;}

	//Detail
	void SetDetail(int8_t objType, int_t objId)
	{ 
		m_strtDetail.m_nObjType = objType; 
		m_strtDetail.m_nObjID = objId;
	}
	void SetDetail(int8_t objType, int_t objId, int32_t xpos, int32_t ypos)
	{ 
		m_strtDetail.m_nObjType = objType; 
		m_strtDetail.m_nObjID = objId;
		m_strtDetail.m_nDstXpos = xpos;
		m_strtDetail.m_nDstYpos = ypos;
	}
	int_t   GetObjId() const {return m_strtDetail.m_nObjID;}
	int8_t  GetObjType() const {return m_strtDetail.m_nObjType;}
	int32_t GetDestinationXpos() const {return m_strtDetail.m_nDstXpos;}
	int32_t GetDestinationYpos() const {return m_strtDetail.m_nDstYpos;}

	//Msg type
	void SetType(int_t type) {m_Type = type;}
	int_t GetType() const {return m_Type;}

	//network flag
	void SetNetFlag(bool_t bFlag) {m_bNetFlag = bFlag;}
	bool_t GetNetFlag() const {return m_bNetFlag;}
	//Message& operator=(Message &message);
	bool_t operator==(Message &message) const;
};

}