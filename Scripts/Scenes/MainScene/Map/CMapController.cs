using UnityEngine;
using UnityEngine.EventSystems;


public class CMapController : MonoBehaviour
{
    public Camera cam;
    public float dragSpeed = 0.2f;
    public float zoomSpeed = 2f;
    public Vector2 minBounds;
    public Vector2 maxBounds;

    private Vector3 dragOrigin;
    private Transform mapTransform;

    // 위경도 변환용
    public Vector2 latLonMin; // (최소 위도, 최소 경도)
    public Vector2 latLonMax; // (최대 위도, 최대 경도)

    bool hasFocus = true;

    void OnApplicationFocus(bool focus)
    {
        hasFocus = focus;
    }

    void Start()
    {
        mapTransform = transform;
        if (cam == null) cam = Camera.main;
    }

    void Update()
    {
#if UNITY_EDITOR
        // 마우스가 Game 뷰 안에 있을 때만 처리
        Vector3 mousePos = Input.mousePosition;
        if (mousePos.x < 0 || mousePos.y < 0 ||
            mousePos.x > Screen.width || mousePos.y > Screen.height)
        {
            return; // 범위 밖이면 무시
        }
#endif
        if (!Application.isFocused) return;
        if (!hasFocus) return;
        if (EventSystem.current != null && EventSystem.current.IsPointerOverGameObject())
            return; // UI 위에서는 무시

        HandleZoom();
        HandleDrag();
    }

    private void HandleZoom()
    {
        float scroll = Input.GetAxis("Mouse ScrollWheel");
        if (Mathf.Abs(scroll) > 0.01f)
        {
            float newScale = Mathf.Clamp(
                mapTransform.localScale.x + scroll * zoomSpeed,
                0.5f, 8f
            );
            mapTransform.localScale = Vector3.one * newScale;
        }
    }

    private void HandleDrag()
    {
        if (Input.GetMouseButtonDown(1))
        {
            dragOrigin = cam.ScreenToWorldPoint(Input.mousePosition);
        }

        if (Input.GetMouseButton(1))
        {
            Vector3 currentPos = cam.ScreenToWorldPoint(Input.mousePosition);
            Vector3 difference = currentPos - dragOrigin;
            mapTransform.position += difference * dragSpeed * mapTransform.localScale.x;

            dragOrigin = currentPos;

            ClampPosition();
            
        }
    }

    private void ClampPosition()
    {
        Vector3 pos = mapTransform.position;
        Vector3 scale = mapTransform.localScale;
        pos.x = Mathf.Clamp(pos.x, minBounds.x * scale.x, maxBounds.x * scale.x);
        pos.y = Mathf.Clamp(pos.y, minBounds.y * scale.y, maxBounds.y * scale.y);
        mapTransform.position = pos;
    }

    // 화면 좌표 → 위경도 변환
    public Vector2 GetLatLon(Vector3 worldPos)
    {
        Bounds mapBounds = GetComponent<SpriteRenderer>().bounds;

        float tX = Mathf.InverseLerp(mapBounds.min.x, mapBounds.max.x, worldPos.x);
        float tY = Mathf.InverseLerp(mapBounds.min.y, mapBounds.max.y, worldPos.y);

        float lat = Mathf.Lerp(latLonMin.x, latLonMax.x, tY);
        float lon = Mathf.Lerp(latLonMin.y, latLonMax.y, tX);

        return new Vector2(lat, lon);
    }

    // 위경도 -> 월드 좌표 변환
    public Vector3 LatLonToWorld(Vector2 latLon)
    {
        Bounds mapBounds = GetComponent<SpriteRenderer>().bounds;

        float tX = Mathf.InverseLerp(latLonMin.y, latLonMax.y, latLon.y);
        float tY = Mathf.InverseLerp(latLonMin.x, latLonMax.x, latLon.x);

        float worldX = Mathf.Lerp(mapBounds.min.x, mapBounds.max.x, tX);
        float worldY = Mathf.Lerp(mapBounds.min.y, mapBounds.max.y, tY);

        return new Vector3(worldX, worldY, 20f);
    }

}
