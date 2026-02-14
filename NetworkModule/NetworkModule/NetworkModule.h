// ********************************************************************
// * 소스정의: NetworkModule.h
// * 설    명: 동적라이브러리 export 함수를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 21
// ********************************************************************
#pragma once
#include "pch.h"
#include <cstdbool>

#ifdef NETWORKMODULE_EXPORTS
#define NET_API __declspec(dllexport)
#else
#define NET_API __declspec(dllimport)
#endif

extern "C" {
	NET_API bool_t ConnectNetwork(const char_t* strIpAddress, const char_t* strPortNo, const char_t* strUdpPortNo);
	NET_API void DisconnectNetwork();
	NET_API void SendPacketToNetwork(int8_t nPacketType);
	NET_API void SendPacketToNetworkForLogin(int8_t nPacketType,const char_t* strUserId, const char_t* strUserPW);
	NET_API void SendPacketToNetworkPos(int8_t nPacketType, int32_t xpos, int32_t ypos, int16_t nObjId);
	NET_API int_t  ReceivePacketFromNetwork(char_t* czOutBuffer, int_t nBufSize);
}
