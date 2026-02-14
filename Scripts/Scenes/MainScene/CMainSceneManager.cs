using System.IO;
using UnityEngine;
using UnityEngine.UI;

public class CMainSceneManager : CSceneManager
{
    public override string m_strSceneName => Global.MAIN_SCENE;


    [SerializeField] private CPopupManager m_cPopupManager;
    [SerializeField] private CUnitManager m_cUnitManager;
    [SerializeField] private CSimulationManager m_cSimulationManager;

    public CMapController m_mapController;
    private KoreaMapData m_mapData;
    private string csvPath;
    public Text infoText;


    /// <summary>
    /// 맵 데이터 디버그용
    /// //////////////////////////////////////////////
    /// </summary>
    public SpriteRenderer mapSprite;   // 원본 지도
    public float overlayAlpha = 0.5f;

    private GameObject overlayObj;

    #region function
    public override void Awake()
    {
        base.Awake();
    }
    public override void Start()
    {
        base.Start();

        CNetworkManager.Instance.m_cMainSceneManager = this;
        CNetworkManager.Instance.m_cSimulationManager = m_cSimulationManager;
        CNetworkManager.Instance.m_cUnitManager = m_cUnitManager;

        //load obj info packet send
        CNetworkManager.Instance.SendPacketType((byte)NetworkPackets.PACKET_TYPE.REQUEST_MILITARY_UNIT_INFO);

        csvPath = Path.Combine(Application.streamingAssetsPath, "korea_mapdata2.csv");
        m_mapData = new KoreaMapData();
        m_mapData.LoadCSV(csvPath);

        //debug용 맵 오버레이 맞추기
        //ShowOverlay(m_mapData);
    }
    public override void Update()
    {
        base.Update();
#if UNITY_EDITOR
        if (!Application.isPlaying) return;
#endif

        if (m_cSimulationManager.m_bIsSimulationStart == false) return;
        //테스트 시에만 해제

        Vector2 mouseWorld = m_mapController.cam.ScreenToWorldPoint(Input.mousePosition);
        Vector2 latLon = m_mapController.GetLatLon(mouseWorld);
        int tile = m_mapData.GetTileValue(latLon.x, latLon.y,
                                        m_mapController.latLonMin.x, m_mapController.latLonMax.x,
                                        m_mapController.latLonMin.y, m_mapController.latLonMax.y);

        string tileName = tile == 0 ? "바다" : tile == 1 ? "육지" : "산";
        infoText.text = $"Lat: {latLon.x:F4}, Lon: {latLon.y:F4} → {tileName}";
        m_cUnitManager.RenderVisibleUnits();
        m_cUnitManager.StartUnitControl(latLon,mouseWorld);
    }
    void ShowOverlay(KoreaMapData mapData)
    {
        int h = mapData.height;
        int w = mapData.width;

        Texture2D overlayTex = new Texture2D(w, h, TextureFormat.RGBA32, false);

        // Y축 뒤집어서 픽셀 채우기
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                //int flippedY = h - 1 - y; // 상하 반전
                int flippedY = y; // 그냥 그대로
                int v = mapData.tiles[y, x];

                Color c = Color.clear;
                if (v == 0) c = new Color(0, 0, 1, overlayAlpha);       // 바다 = 파랑
                else if (v == 1) c = new Color(0, 1, 0, overlayAlpha);  // 육지 = 초록
                else if (v == 2) c = new Color(0.5f, 0.5f, 0.5f, overlayAlpha); // 산 = 회색

                overlayTex.SetPixel(x, flippedY, c);
            }
        }

        overlayTex.Apply();

        // Sprite 생성 (pixelsPerUnit = 100으로 맞추기)
        Sprite overlaySprite = Sprite.Create(
            overlayTex,
            new Rect(0, 0, w, h),
            new Vector2(0.5f, 0.5f),
            100f
        );

        // Overlay 오브젝트 생성
        if (overlayObj != null) Destroy(overlayObj);
        overlayObj = new GameObject("MapOverlay");
        SpriteRenderer sr = overlayObj.AddComponent<SpriteRenderer>();
        sr.sprite = overlaySprite;
        sr.sortingOrder = mapSprite.sortingOrder + 1; // 지도 위에 출력

        // 위치/스케일 지도와 동일하게 맞추기
        overlayObj.transform.position = mapSprite.transform.position;
        overlayObj.transform.localScale = mapSprite.transform.localScale;
    }

    public void OnStayButtonClick()
    {
        const string strPopupName = "Exit_popup";

        m_cPopupManager.HidePopup(strPopupName, true);
    }
    public void OnExitButtonClick()
    {
        const string strPopupName = "Exit_popup";

        m_cPopupManager.ShowPopup(strPopupName, true);
    }

    #endregion
}
