// ********************************************************************
// * 소스정의: Message.cpp
// * 설    명: 프로세서간 통신을 위한 메세지 클래스 구현
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
#include "Message.h"

using namespace DEVSIM;
// ********************************************************************
// * 함수정의: Message 
// * 설    명: 메세지를 생성하는 생성자 함수들
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
// 기본 생성자
Message::Message()
	: m_strSourceName(""), m_strPort(""), m_strContent(""), m_Time(0), m_Type(0)
{}

// // 복사 생성자
// Message::Message(Message &msg)
// 	: m_strSourceName(msg.m_strSourceName), m_strPort(msg.m_strPort), m_strContent(msg.m_strContent), m_Time(msg.m_Time), m_Type(msg.m_Type)
// {}

// 특정 포트 및 시간으로 초기화하는 생성자
Message::Message(const string& name, const string& port, TimeType time, int_t type)
	: m_strSourceName(name), m_strPort(port), m_strContent(""), m_Time(time), m_Type(type)
{}

// ********************************************************************
// * 함수정의: ~Message 
// * 설    명: Massage class 소멸자
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
// 소멸자
Message::~Message() {
	// 특별한 동적 할당된 리소스가 없다면 기본 소멸자로 충분
}

// ********************************************************************
// * 함수정의: operator= 
// * 설    명: 대입 연산자 오버로딩
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
// Message& Message::operator=(Message &message) 
// {
// 	if (this != &message) 
// 	{
// 		m_strSourceName = message.m_strSourceName;
// 		m_strPort = message.m_strPort;
// 		m_strContent = message.m_strContent;
// 		m_Time = message.m_Time;
// 	}
// 	return *this;
// }

// ********************************************************************
// * 함수정의: operator== 
// * 설    명: 비교 연산자 오버로딩
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 18
// ********************************************************************	
bool_t Message::operator==(Message &message) const
{
	return((m_strSourceName)==(message.m_strSourceName) && (m_strPort)==(message.m_strPort));
}
