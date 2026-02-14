// ********************************************************************
// * 소스정의: Singleton.h
// * 설    명: 싱글턴 템플릿 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
#pragma once
#include "pch.h"

// ********************************************************************
// * 클래스명: Singleton<T>
// * 설    명: 싱글턴 템플릿 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 06. 30
// ********************************************************************
template<typename T>
class Singleton {
protected:
    Singleton() = default;
    virtual ~Singleton() = default;
public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator =(const Singleton&) = delete;
    Singleton& operator =(Singleton&&) = delete;

    static T& GetInstance() {
        static std::unique_ptr<T> _inst = std::make_unique<T>();
        return *_inst;
    }
};
