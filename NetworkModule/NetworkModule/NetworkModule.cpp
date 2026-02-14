// ********************************************************************
// * 소스정의: NetworkModule.cpp
// * 설    명: 동적라이브러리 export 함수를 정의
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 21
// ********************************************************************
#include "pch.h"
#include "NetworkModule.h"
#include "DataTrsfModel.h"

#pragma comment(lib, "ws2_32.lib")

// 전역 인스턴스
static DATATRSFER::DataTrsfModel g_oNetworkModule;

// C 스타일 export 함수
extern "C" {
	NET_API bool_t ConnectNetwork(const char_t* strIpAddress, const char_t* strPortNo, const char_t* strUdpPortNo)
	{
		return g_oNetworkModule.Start(strIpAddress,strPortNo,strUdpPortNo);
	}
	NET_API void DisconnectNetwork()
	{
		g_oNetworkModule.Finish();
	}
	NET_API void SendPacketToNetwork(int8_t nPacketType)
	{
		g_oNetworkModule.PushMessageToServer(nPacketType,0,0,-1);
	}
	NET_API void SendPacketToNetworkForLogin(int8_t nPacketType,const char_t* strUserId, const char_t* strUserPW)
	{
		g_oNetworkModule.PushLoginMessageToServer(nPacketType,strUserId, strUserPW);
	}
	NET_API void SendPacketToNetworkPos(int8_t nPacketType, int32_t xpos, int32_t ypos,int16_t nId)
	{
		g_oNetworkModule.PushMessageToServer(nPacketType,xpos,ypos,nId);
	}
	NET_API int_t  ReceivePacketFromNetwork(char_t* czOutBuffer, int_t nBufSize)
	{
		return g_oNetworkModule.PopPacketFromRecvQueue(czOutBuffer, nBufSize);
	}

}
