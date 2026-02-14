// **************************************************************
// * 소 스 명: CNetworkManager.cs						
// * 설    명: 서버와 연결하기 위한 NetworkModule을 사용하는 클래스를 정의한다.
// * 작 성 자: KJH							
// * 작성일자: 2025. 08. 20					
// **************************************************************
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using UnityEngine.Analytics;

using NetworkPackets;
using System.Net;
// **************************************************************
// * 클래스명: CNetworkManager				
// * 설    명: Unity에서 서버와 연결하기 위한 모듈과 연결하는 클래스
// * 작 성 자: KJH							
// * 작성일자: 2025. 08. 20						
// **************************************************************
public class CNetworkManager : Singleton<CNetworkManager>
{
#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
	const string DLL_NAME = "NetworkModule";
#endif
	// **************************************************************
	// * 설    명: dynamic linking library에서 exports된 함수들을
	// *           사용하기 위하여 바인딩한다.
	// * 작 성 자: KJH							
	// * 작성일자: 2025. 08. 20					
	// **************************************************************
	[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
	[return: MarshalAs(UnmanagedType.I1)]
	private static extern bool ConnectNetwork(string strIpAddress, string strPortNo, string strUdpPortNo);

	[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
	private static extern void DisconnectNetwork();

	[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
	private static extern void SendPacketToNetwork(sbyte nPacketType); // int8_t

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    private static extern void SendPacketToNetworkForLogin(sbyte nPacketType,string strUserId, string strUserPW);

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
	private static extern void SendPacketToNetworkPos(sbyte nPacketType,int xpos,int ypos,short nObjId); 
	
	[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
	private static extern int ReceivePacketFromNetwork([Out] byte[] czOutBuffer, int nBufSize);

	///////////////////////////////////////////////////recv buffer
	private readonly byte[] czReciveBuf = new byte[Global.BUF_SIZE];

	public bool m_bIsConnect { get; private set; }
	public bool m_bIsLoginSuccess { get; private set; }
	public bool m_bIsLoginFailure { get; private set; }
	public bool m_bIsSimulationStart {  get; private set; }


	///////////////////////////////////////////////////////씬 오브젝트들 MainSceneManager에서 싹다 등록하도록 하자.
	///////////TODO : 구조가 매우 더러우니 나중에 리펙터링해야겟다.
	public CMainSceneManager m_cMainSceneManager { private get; set; }
	public CSimulationManager m_cSimulationManager { private get; set; }
	public CUnitManager m_cUnitManager { private get; set; }
	public CMessageDisplayer m_cMsgDisplayer { private get; set; }

	public CAchievementManager m_cAchievementManager { private get; set; }

	// ********************************************************************
	// * 함 수 명: Start
	// * 설    명: 멤버변수들을 초기화하고 서버 네트워크와 연결한다.
	// * 작 성 자: KJH
	// * 작성날짜: 2025. 08. 20
	// ********************************************************************
	protected override void Awake()
	{
		base.Awake();
		string strIpAddress = Global.SERVER_IP_ADDRESS;
		string strPortNo = Global.SERVER_TCP_PORT_NO;
		string strUdpPortNo = Global.SERVER_UDP_PORT_NO;

		
		m_bIsConnect = ConnectNetwork(strIpAddress, strPortNo, strUdpPortNo);
		m_bIsSimulationStart = false;
		Debug.Log("ConnectNetwork: " + m_bIsConnect);
	}

	// ********************************************************************
	// * 함 수 명: Update
	// * 설    명: frame 당 네트워크에서 받아온 패킷을 수신한다.
	// * 작 성 자: KJH
	// * 작성날짜: 2025. 08. 20
	// ********************************************************************
	public unsafe void Update()
	{
		if (!m_bIsConnect) return;

		int nTotalBytes = ReceivePacketFromNetwork(czReciveBuf, czReciveBuf.Length);
		if (nTotalBytes <= 0)
		{
			return;
		}

		int offset = 0;

		fixed (byte* pBase = czReciveBuf)
		{
			while (offset < nTotalBytes)
			{
				StPacketHeader* pPacketHeader = (StPacketHeader*)(pBase + offset);
				//로그인 요청후 udp connect까지 연결이 되었을 때 
				if (pPacketHeader->nType == (byte)PACKET_TYPE.HELLO_UDP_RESPONSE)
				{
					offset += sizeof(StPacketHeader);
					//login success
					if (pPacketHeader->nFlagField == (byte)NETWORK_FLAG.LOGIN_SUCCESS) 
					{
                        m_bIsLoginSuccess = true;
						m_bIsLoginFailure = false;
                    }
					//login fail
					else 
					{
						m_bIsLoginFailure = true;
						m_bIsLoginSuccess = false;
					}
				}
				//엔진의 시뮬레이션 시작 응답
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.RESPONSE_SIM_START)
				{
					Debug.Log("Response engine start simulation!!!");
					offset += sizeof(StStartResponseEnginePacket);
					if (m_cSimulationManager != null)
					{
						m_cSimulationManager.RecvSimulationStart();
					}
					if (m_cAchievementManager != null)
					{
						//TODO : 좀더 유지보수 좋게 할 방법을 생각해보자.
						m_cAchievementManager.AddProgress(0);
                        m_cAchievementManager.AddProgress(1);
                        m_cAchievementManager.AddProgress(2);
                    }
				}
				//엔진의 부대정보 전송
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.RESPONSE_MILITARY_UNIT_INFO)
				{
					Debug.Log("Response militaryUnitInfo from server");
					StResponseLoadMilitaryUnitInfoPacket* pResponseLoadObjPacket = (StResponseLoadMilitaryUnitInfoPacket*)(pBase + offset);
					offset += sizeof(StResponseLoadMilitaryUnitInfoPacket);
					//화면에 부대를 띄워야함
					//IPAddress.NetworkToHostOrder(pResponseLoadObjPacket->strtMilitaryInfo.);
					//pResponseLoadObjPacket->strtMilitaryInfo.nID;
					ushort nId = ntohs(pResponseLoadObjPacket->strtMilitaryInfo.nID);
					uint xpos = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nXpos);
					uint ypos = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nYpos);
					uint health = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nUnitHealth);
					uint artillery = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nTotalArtillery);
					byte isLoadDone = pResponseLoadObjPacket->strtHeader.nFlagField;
					//Debug.Log("id : "+nId + " xpos " + pResponseLoadObjPacket->strtMilitaryInfo.nXpos + " " + xpos);
					if (m_cUnitManager != null)
					{
						int nUnitType = nId < 50 ? (int)UnitType.MILITARY_UNIT : (int)UnitType.MILITARY_UNIT_E;
						m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos/1000.0f,nUnitType,(int)health,(int)artillery);
					}
					if (isLoadDone == (byte)NETWORK_FLAG.FIN) 
					{
                        if (m_cMsgDisplayer != null)
                        {
							m_cMsgDisplayer.ShowMessage("Load", GetCurrentTime(), "Load Military unit infomation is done");
                        }
                    }
				}
				//엔진의 출격명령 응답 전송
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.RESPONSE_ORDER_IN_SALLY)
				{
					Debug.Log("Response OrderInSally from engine");
					StResponseOrderInSallyPacket* pResponsePacket = (StResponseOrderInSallyPacket*)(pBase + offset);
					offset += sizeof(StResponseOrderInSallyPacket);
					ushort nId = ntohs(pResponsePacket->strtMilitaryInfo.nID);
					uint xpos = ntohl(pResponsePacket->strtMilitaryInfo.nXpos);
					uint ypos = ntohl(pResponsePacket->strtMilitaryInfo.nYpos);
					uint health = ntohl(pResponsePacket->strtMilitaryInfo.nUnitHealth);
					uint artillery = ntohl(pResponsePacket->strtMilitaryInfo.nTotalArtillery);
					if (m_cUnitManager != null)
					{
						int nUnitType = nId < 50 ? (int)UnitType.MILITARY_UNIT : (int)UnitType.MILITARY_UNIT_E;
						m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)artillery);
					}
					nId = ntohs(pResponsePacket->strtArtilleryInfo.nID);
					xpos = ntohl(pResponsePacket->strtArtilleryInfo.nXpos);
					ypos = ntohl(pResponsePacket->strtArtilleryInfo.nYpos);
					health = ntohl(pResponsePacket->strtArtilleryInfo.nUnitHealth);
					uint nTotalMissile = ntohl(pResponsePacket->strtArtilleryInfo.nTotalMissile);
					if (m_cUnitManager != null)
					{
						int nUnitType = nId / 100 < 50 ? (int)UnitType.ARTILLERY : (int)UnitType.ARTILLERY_E;
						m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)nTotalMissile);
					}
                    if (m_cMsgDisplayer != null)
                    {
                        m_cMsgDisplayer.ShowMessage("Sally", GetCurrentTime(), "Artillery" + nId + "get order in sally");
                    }
                }
				//엔진의 발사명령 응답 전송
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.RESPONSE_ORDER_IN_ATTACK)
				{
					Debug.Log("Response OrderInAttack from engine");
					StResponseOrderInAttackPacket* pResponsePacket = (StResponseOrderInAttackPacket*)(pBase + offset);
					offset += sizeof(StResponseOrderInAttackPacket);
					uint nId = ntohl(pResponsePacket->strtMissileInfo.nID);
					uint xpos = ntohl(pResponsePacket->strtMissileInfo.nXpos);
					uint ypos = ntohl(pResponsePacket->strtMissileInfo.nYpos);
                    Debug.Log("mis id : " + nId);
                    if (m_cUnitManager != null)
					{
						m_cUnitManager.OnUnitUpdateDataFromServer((int)nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, (int)UnitType.MISSILE);
					}
					ushort nArtId = ntohs(pResponsePacket->strtArtilleryInfo.nID);
					xpos = ntohl(pResponsePacket->strtArtilleryInfo.nXpos);
					ypos = ntohl(pResponsePacket->strtArtilleryInfo.nYpos);
					uint health = ntohl(pResponsePacket->strtArtilleryInfo.nUnitHealth);
					uint nTotalMissile = ntohl(pResponsePacket->strtArtilleryInfo.nTotalMissile);
                    Debug.Log("art id : " + nArtId);
                    if (m_cUnitManager != null)
					{
						int nUnitType = nArtId / 100 < 50 ? (int)UnitType.ARTILLERY : (int)UnitType.ARTILLERY_E;
						m_cUnitManager.OnUnitUpdateDataFromServer(nArtId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)nTotalMissile);
					}
                    if (m_cMsgDisplayer != null)
                    {
                        m_cMsgDisplayer.ShowMessage("Attack", GetCurrentTime(), "Artillery" + nArtId + " start attack");
                    }
                }
				//엔진의 자주포객체 위치 동기화 패킷 전송
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.SYNC_ARTILLERY_COORDINATE)
				{
					//Debug.Log("Sync artillery unit arrived!");
					StSyncArtilleryCoordinatePacket* pResponsePacket = (StSyncArtilleryCoordinatePacket*)(pBase + offset);
					offset += sizeof(StSyncArtilleryCoordinatePacket);
					
					ushort nId = ntohs(pResponsePacket->strtArtilleryInfo.nID);
					uint xpos = ntohl(pResponsePacket->strtArtilleryInfo.nXpos);
					uint ypos = ntohl(pResponsePacket->strtArtilleryInfo.nYpos);
					uint health = ntohl(pResponsePacket->strtArtilleryInfo.nUnitHealth);
					uint nTotalMissile = ntohl(pResponsePacket->strtArtilleryInfo.nTotalMissile);
					if (m_cUnitManager != null)
					{
						int nUnitType = nId / 100 < 50 ? (int)UnitType.ARTILLERY : (int)UnitType.ARTILLERY_E;
						m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)nTotalMissile);
					}
                    if (pResponsePacket->strtHeader.nFlagField == (byte)NETWORK_FLAG.TCP)
                    {
                        if (m_cMsgDisplayer != null)
                        {
                            m_cMsgDisplayer.ShowMessage("Sally Done", GetCurrentTime(), "Artillery" + nId + " arrived at destination");
                        }
                    }
                }
				//엔진의 missile객체 위치 동기화 패킷 전송
				else if (pPacketHeader->nType == (byte)PACKET_TYPE.SYNC_MISSILE_COORDINATE)
				{
					Debug.Log("Sync Missile unit arrived!");
					StSyncMissileCoordinatePacket* pResponsePacket = (StSyncMissileCoordinatePacket*)(pBase + offset);
					offset += sizeof(StSyncMissileCoordinatePacket);

					uint nId = ntohl(pResponsePacket->strtMissileInfo.nID);
					uint xpos = ntohl(pResponsePacket->strtMissileInfo.nXpos);
					uint ypos = ntohl(pResponsePacket->strtMissileInfo.nYpos);
					if (m_cUnitManager != null)
					{

                        if (pResponsePacket->strtHeader.nFlagField == (byte)NETWORK_FLAG.TCP)
                        {
                            m_cUnitManager.OnUnitDestroyFromServer((int)nId);
                        }
						else
						{
                            m_cUnitManager.OnUnitUpdateDataFromServer((int)nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, (int)UnitType.MISSILE);
							Debug.Log("udp!!!!!");
                        }
                    }
				}
                //엔진의 피격이벤트 동기화 패킷 전송
                else if (pPacketHeader->nType == (byte)PACKET_TYPE.SYNC_ARTILLERY_ATTACKED)
                {
                    Debug.Log("Sync artillery unit attacked!");
                    StSyncArtilleryAttackedPacket* pResponsePacket = (StSyncArtilleryAttackedPacket*)(pBase + offset);
                    offset += sizeof(StSyncArtilleryAttackedPacket);

					
                    ushort nId = ntohs(pResponsePacket->strtArtilleryInfo.nID);
                    uint xpos = ntohl(pResponsePacket->strtArtilleryInfo.nXpos);
                    uint ypos = ntohl(pResponsePacket->strtArtilleryInfo.nYpos);
                    uint health = ntohl(pResponsePacket->strtArtilleryInfo.nUnitHealth);
                    uint nTotalMissile = ntohl(pResponsePacket->strtArtilleryInfo.nTotalMissile);
                    if (m_cMsgDisplayer != null)
                    {
                        m_cMsgDisplayer.ShowMessage("Attacked", GetCurrentTime(), "Artillery" + nId + " is under attack");
                    }

                    int nUnitType = nId / 100 < 50 ? (int)UnitType.ARTILLERY : (int)UnitType.ARTILLERY_E;

                    if (health <= 0)
					{

                        m_cUnitManager.OnUnitDestroyFromServer(nId);
                        if (m_cMsgDisplayer != null)
                        {
                            m_cMsgDisplayer.ShowMessage("Destroy", GetCurrentTime(), "Artillery" + nId + " is destroy to missile");
                        }
                        if (m_cAchievementManager != null)
						{
							if (nUnitType == (int)UnitType.ARTILLERY_E)
							{
                                m_cAchievementManager.AddProgress(3);
                                m_cAchievementManager.AddProgress(4);
                                m_cAchievementManager.AddProgress(5);
                                m_cAchievementManager.AddProgress(6);
                            }
							else
							{
                                m_cAchievementManager.AddProgress(7);
                                m_cAchievementManager.AddProgress(8);
                            }
                        }

                    }

                    if (m_cUnitManager != null)
                    {

                        if (health <= 0)
                        {
                            m_cUnitManager.OnUnitDestroyFromServer(nId);
                        }
                        else
						{
                            m_cUnitManager.OnUnitAttackedFromServer(nId);
                            m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)nTotalMissile);
                        }
                    }
                }
                //엔진의 부대 피격이벤트 전송
                else if (pPacketHeader->nType == (byte)PACKET_TYPE.SYNC_MILITARY_UNIT_ATTACKED)
                {
                    Debug.Log("militaryUnit attacked");
                    StSyncMilitaryUnitAttackedPacket* pResponseLoadObjPacket = (StSyncMilitaryUnitAttackedPacket*)(pBase + offset);
                    offset += sizeof(StSyncMilitaryUnitAttackedPacket);
                    ushort nId = ntohs(pResponseLoadObjPacket->strtMilitaryInfo.nID);
                    uint xpos = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nXpos);
                    uint ypos = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nYpos);
                    uint health = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nUnitHealth);
                    uint artillery = ntohl(pResponseLoadObjPacket->strtMilitaryInfo.nTotalArtillery);

                    if (health <= 0)
                    {
                        m_cUnitManager.OnUnitDestroyFromServer(nId);
                        if (m_cMsgDisplayer != null)
                        {
                            m_cMsgDisplayer.ShowMessage("Destroy", GetCurrentTime(), "Military unit" + nId + " is destroy to missile");
                        }
                    }
					else
					{
                        m_cUnitManager.OnUnitAttackedFromServer(nId);
                        if (m_cMsgDisplayer != null)
                        {
                            m_cMsgDisplayer.ShowMessage("Attacked", GetCurrentTime(), "Military unit" + nId + " is under attack");
                        }
                    }

                    if (m_cUnitManager != null)
                    {
                        int nUnitType = nId < 50 ? (int)UnitType.MILITARY_UNIT : (int)UnitType.MILITARY_UNIT_E;

                        m_cUnitManager.OnUnitUpdateDataFromServer(nId, (float)xpos / 1000.0f, (float)ypos / 1000.0f, nUnitType, (int)health, (int)artillery);
                    }
                }
                //엔진의 시간 동기화 패킷 전송
                else if (pPacketHeader->nType == (byte)PACKET_TYPE.SYNC_SIM_TIME)
				{
					StSyncSimulationTimePacket* pResponsePacket = (StSyncSimulationTimePacket*)(pBase + offset);
					//Debug.Log("Sync simulation time recv , current time : ");
					offset += sizeof(StSyncSimulationTimePacket);
					int nTime = IPAddress.NetworkToHostOrder(pResponsePacket->nCurrentSimulationTime);
					if (m_cSimulationManager != null)
					{
						m_cSimulationManager.OnUpdateSimulationTimeFromServer(nTime);
					}
				}
				else
				{
					Debug.Log("Do not find Correct Packet Header Info");
					return;
				}


			}
		}
	}
	// ********************************************************************
	// * 함 수 명: OnApplicationQuit
	// * 설    명: application이 종료되면 네트워크 연결을 종료한다.
	// * 작 성 자: KJH
	// * 작성날짜: 2025. 08. 20
	// ********************************************************************
	protected override void OnDestroy()
	{
		base.OnDestroy();
		DisconnectNetwork();
	}

	// ********************************************************************
	// * 함 수 명: OnApplicationQuit
	// * 설    명: application이 종료되면 네트워크 연결을 종료한다.
	// * 작 성 자: KJH
	// * 작성날짜: 2025. 08. 20
	// ********************************************************************
	private void OnApplicationQuit()
	{
		//DisconnectNetwork();
	}

	// ********************************************************************
	// * 함 수 명: SendPacketType
	// * 설    명: Packet type을 받아서 서버로 전송한다.
	// * 작 성 자: KJH
	// * 작성날짜: 2025. 08. 20
	// ********************************************************************
	public void SendPacketType(byte type)
	{
		unchecked { SendPacketToNetwork((sbyte)type); }
	}
	public void SendPacketLogin(byte type,string  username, string password)
	{
		unchecked { SendPacketToNetworkForLogin((sbyte)type, username, password); }
	}
	public void SendPacketTypePos(byte type, int xpos, int ypos, short nObjId)
	{
		unchecked { SendPacketToNetworkPos((sbyte)type, xpos, ypos, nObjId); }
	}

	// 32비트: ntohl (uint32_t)
	public uint ntohl(uint value)
	{
		return ((value & 0x000000FFU) << 24) |
			   ((value & 0x0000FF00U) << 8) |
			   ((value & 0x00FF0000U) >> 8) |
			   ((value & 0xFF000000U) >> 24);
	}

	// 16비트: ntohs (uint16_t)
	public ushort ntohs(ushort value)
	{
		return (ushort)(
			((value & 0x00FF) << 8) |
			((value & 0xFF00) >> 8)
		);
	}
    private string GetCurrentTime()
    {
        // 현재 PC의 시스템 시간
        DateTime now = DateTime.Now;

        // 원하는 형식으로 변환 (예: "15:30:45")
        return now.ToString("HH:mm:ss");
    }
}
