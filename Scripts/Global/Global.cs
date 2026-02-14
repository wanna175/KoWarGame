// **************************************************************
//  소 스 명: Global.cs					
//  설    명: config 관련 전역상수및 Global class를 정의한다.
//  작 성 자: KJH							
//  작성일자: 2025. 08. 19					
// **************************************************************
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace GameMode
{
    enum MouseMode
    {
        None = 0,
        Move = 1,
        Attack = 2,
    }
}
enum UnitType
{
    MILITARY_UNIT = 0,
    MILITARY_UNIT_E = 1,
    INFANTRY = 2,
    INFANTRY_E = 3,
    ARTILLERY = 4,
    ARTILLERY_E = 5,


    MISSILE = 6,
}
// **************************************************************
//  클래스명: Global.cs					
//  설    명: config 관련 전역상수및 Global class를 정의한다.
//  작 성 자: KJH							
//  작성일자: 2025. 08. 19						
// **************************************************************
public static class Global 
{
    #region Resolution related constants
    //화면 기준 해상도 변수
    public const float G_DESIGN_WIDTH = 1920.0f;
    public const float G_DESIGN_HEIGHT = 1080.0f;
    public static readonly Vector3 G_DESIGN_SIZE = new Vector3(G_DESIGN_WIDTH, G_DESIGN_HEIGHT, 0.0f);

    #endregion
    

    #region SceneName
    public const string SCENE_PREFAB        = "ScenePrefab";
    public const string USER_LOGIN_SCENE    = "UserLoginScene";
    public const string MAIN_SCENE          = "MainScene";
    public const string LOADING_SCENE       = "LoadingScene";
    #endregion

    #region Achievement
    /// <summary>
    /// simulation run
    /// </summary>
    public const string ACHIEVEMENT_TITLE_01   = "1st DEVS";
    public const string ACHIEVEMENT_CONTENT_01 = "Run the first simulation";
    public const int ACHIEVEMENT_TOTAL_01 = 1;

    public const string ACHIEVEMENT_TITLE_02   = "99th DEVS";
    public const string ACHIEVEMENT_CONTENT_02 = "Run the 99th simulation";
    public const int ACHIEVEMENT_TOTAL_02 = 99;

    public const string ACHIEVEMENT_TITLE_03 = "999th DEVS";
    public const string ACHIEVEMENT_CONTENT_03 = "Run the 999th simulation";
    public const int ACHIEVEMENT_TOTAL_03 = 999;
    
    /// <summary>
    /// artillery kill
    /// </summary>
    public const string ACHIEVEMENT_TITLE_04 = "First Blood";
    public const string ACHIEVEMENT_CONTENT_04 = "Destroy one self-propelled artillery unit";
    public const int ACHIEVEMENT_TOTAL_04 = 1;

    public const string ACHIEVEMENT_TITLE_05 = "Penta kill";
    public const string ACHIEVEMENT_CONTENT_05 = "Destroy five self-propelled artillery unit";
    public const int ACHIEVEMENT_TOTAL_05 = 5;

    public const string ACHIEVEMENT_TITLE_06 = "A Beginner";
    public const string ACHIEVEMENT_CONTENT_06 = "Destroy ten self-propelled artillery unit";
    public const int ACHIEVEMENT_TOTAL_06 = 10;

    public const string ACHIEVEMENT_TITLE_07 = "Slayer";
    public const string ACHIEVEMENT_CONTENT_07 = "I'm a war fanatic";
    public const int ACHIEVEMENT_TOTAL_07 = 99;


    /// <summary>
    /// artillery die
    /// </summary>
    public const string ACHIEVEMENT_TITLE_08 = "The first sacrifice";
    public const string ACHIEVEMENT_CONTENT_08 = "Destroyed one self-propelled artillery unit by enemy";
    public const int ACHIEVEMENT_TOTAL_08 = 1;

    public const string ACHIEVEMENT_TITLE_09 = "A man who can't care for soldiers";
    public const string ACHIEVEMENT_CONTENT_09 = "Destroyed 99 self-propelled artillery unit by enemy";
    public const int ACHIEVEMENT_TOTAL_09 = 99;
    /// <summary>
    /// sim end
    /// </summary>
    public const string ACHIEVEMENT_TITLE_10 = "Winner";
    public const string ACHIEVEMENT_CONTENT_10 = "I'm going to win the war game";
    public const int ACHIEVEMENT_TOTAL_10 = 1;

    #endregion


    // **************************************************************			
    //  설    명: NetworkModule 관련 상수를 정의한다.
    //  작 성 자: KJH							
    //  작성일자: 2025. 08. 19				
    // **************************************************************
    //public const string SERVER_IP_ADDRESS = "172.20.10.2";
    public const string SERVER_IP_ADDRESS = "192.168.84.109";
    public const string SERVER_TCP_PORT_NO = "12345";
    public const string SERVER_UDP_PORT_NO = "12346";

    public const int BUF_SIZE = 4096;

}
