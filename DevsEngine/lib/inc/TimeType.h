// ********************************************************************
// * 헤더정의: TimeType.h 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ********************************************************************
#pragma once

#include <limits>
#include <string>

#include "TypeDef.h"

using std::string;

#define Infinity ( std::numeric_limits<double_t>::has_infinity ? std::numeric_limits<double_t>::infinity() : 1.7976931348623158e+308  ) 
#define Epsilon (1e-6)

// ***************************************************************
// * 클래스명: TimeType
// * 설    명: 시뮬레이션 시간을 정의한 클래스 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ****************************************************************
class TimeType {
public:
	TimeType(void) = default;
	TimeType(double_t t) {m_dTime = t;}
	TimeType(const TimeType& t) = default;
	virtual ~TimeType(void) = default;

	string toString() const;

	double_t GetValue() const { return m_dTime; }
	operator double() const { return m_dTime; }
// ***************************************************************
// * 함 수 명: operator
// * 설    명: 여러 operator를 오버라이드 한다. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 17
// ****************************************************************
	TimeType& operator=(TimeType& value ) { m_dTime = value.GetValue(); return *this; }
	TimeType& operator=(double_t value) {m_dTime = value; return *this;}

	TimeType& operator+=(double_t value) { m_dTime = (*this).GetValue() + value; return *this; }

	TimeType& operator+=(TimeType& value) { m_dTime = ((*this) + value).GetValue(); return *this; }
	TimeType& operator-=(TimeType& value) { m_dTime = ((*this) - value).GetValue(); return *this; }
	TimeType& operator*=(TimeType& value) { m_dTime = ((*this) * value).GetValue(); return *this; }
	TimeType& operator/=(TimeType& value) { m_dTime = ((*this) / value).GetValue(); return *this; }

	TimeType operator+(TimeType &flt) {if( m_dTime == Infinity || flt.GetValue() == Infinity ) return TimeType(Infinity); else return TimeType(m_dTime + flt.GetValue());}
	TimeType operator-(TimeType &flt) {if( m_dTime == Infinity || flt.GetValue() == Infinity ) return TimeType(Infinity); else return TimeType(m_dTime - flt.GetValue());}
	TimeType operator*(TimeType &flt) {if( m_dTime == 0 || flt.GetValue() == 0 ) return TimeType(0.0); else if( m_dTime != 0 && flt.GetValue() != 0 && (m_dTime == Infinity || flt.GetValue() == Infinity) ) return TimeType(Infinity); else return TimeType(m_dTime * flt.GetValue());}
	TimeType operator/(TimeType &flt) {if( m_dTime == Infinity || flt.GetValue() == 0 ) return TimeType(Infinity); else if( m_dTime != Infinity && flt.GetValue() == Infinity ) return TimeType(0.0); else return TimeType(m_dTime / flt.GetValue());}

	bool_t operator==(TimeType &flt) {if( m_dTime == Infinity && flt.GetValue() == Infinity ) return true; else return (-Epsilon < (m_dTime-flt.GetValue())) && ((m_dTime-flt.GetValue()) < Epsilon); }
	bool_t operator!=(TimeType &flt) {return !((*this) == flt) ;}
	bool_t operator<(TimeType &flt) {if( m_dTime == Infinity && flt.GetValue() == Infinity ) return false; else return (-Epsilon > (m_dTime-flt.GetValue()));}
	bool_t operator>(TimeType &flt) {if( m_dTime == Infinity && flt.GetValue() == Infinity ) return false; else return (m_dTime-flt.GetValue()) > Epsilon;}
	bool_t operator<=(TimeType &flt) {return ((*this)==flt || (*this)<flt);}
	bool_t operator>=(TimeType &flt) {return ((*this)==flt || (*this)>flt);}
	
	friend TimeType operator+(double_t value, TimeType &flt) {return TimeType(value + flt.GetValue());}
	friend TimeType operator-(double_t value, TimeType &flt) {return TimeType(value - flt.GetValue());}
	friend TimeType operator*(double_t value, TimeType &flt) {return TimeType(value * flt.GetValue());}
	friend TimeType operator/(double_t value, TimeType &flt) {return TimeType(value / flt.GetValue());}

	friend bool_t operator==(double_t value, TimeType &flt) {return (TimeType(value) == flt);}
	friend bool_t operator!=(double_t value, TimeType &flt) {return (TimeType(value) != flt);}
	friend bool_t operator<(double_t value, TimeType &flt) {return (TimeType(value) < flt);}
	friend bool_t operator>(double_t value, TimeType &flt) {return (TimeType(value) > flt);}
	friend bool_t operator<=(double_t value, TimeType &flt) {return (TimeType(value) <= flt);}
	friend bool_t operator>=(double_t value, TimeType &flt) {return (TimeType(value) >= flt);}

protected:
    double_t m_dTime;
};



