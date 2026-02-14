using UnityEngine;

public class CUnitScaleFixer : MonoBehaviour
{
    private Vector3 baseScale;
    private Transform map;

    [Range(0f, 1f)]
    public float followRatio = 0.7f; // 0=맵 따라감, 1=고정

    [Header("최소/최대 유닛 크기")]
    public float minScale = 0.5f;
    public float maxScale = 2.0f;

    void Start()
    {
        baseScale = transform.localScale;
        map = transform.parent; // mapController가 부모라고 가정
    }

    void LateUpdate()
    {
        if (map == null) return;

        float parentScale = map.localScale.x; // 2D라서 x=y=z라 가정
        float inverse = 1f / parentScale;

        // followRatio 비율로 섞기
        float blended = Mathf.Lerp(1f, inverse, followRatio);

        // 제한 적용
        blended = Mathf.Clamp(blended, minScale, maxScale);

        transform.localScale = baseScale * blended;
    }
}
