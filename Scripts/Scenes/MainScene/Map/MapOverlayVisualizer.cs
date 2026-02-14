using UnityEngine;

public class MapOverlayVisualizer : MonoBehaviour
{
    private KoreaMapData mapData;       // CSV 배열
    public SpriteRenderer mapSprite;   // 원본 지도
    public float overlayAlpha = 0.5f;

    private Texture2D overlayTex;
    private SpriteRenderer overlayRenderer;

    private const string csvPath = "Assets/Resources/Object/Map/korea_mapdata2.csv";
    void Start()
    {
        Debug.Log("dfdfdfdf");
        mapData = new KoreaMapData();
        mapData.LoadCSV(csvPath);
        int w = mapData.width;
        int h = mapData.height;

        overlayTex = new Texture2D(w, h);
        overlayTex.filterMode = FilterMode.Point;

        // 색상 지정
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                Color c = Color.clear;
                int v = mapData.tiles[y, x];
                if (v == 0) c = new Color(0, 0, 1, overlayAlpha);       // 바다
                else if (v == 1) c = new Color(0, 1, 0, overlayAlpha);  // 육지
                else if (v == 2) c = new Color(0.5f, 0.5f, 0.5f, overlayAlpha); // 산
                overlayTex.SetPixel(x, y, c);
            }
        }
        overlayTex.Apply();

        // Sprite 생성
        Sprite overlaySprite = Sprite.Create(overlayTex,
            new Rect(0, 0, w, h), new Vector2(0.5f, 0.5f), 1f);

        // 오버레이용 GameObject
        GameObject overlayObj = new GameObject("MapOverlay");
        overlayRenderer = overlayObj.AddComponent<SpriteRenderer>();
        overlayRenderer.sprite = overlaySprite;
        overlayRenderer.sortingOrder = mapSprite.sortingOrder + 1;

        // 위치/스케일 맞추기
        overlayObj.transform.position = mapSprite.transform.position;
        overlayObj.transform.localScale = mapSprite.transform.localScale;
    }
}
