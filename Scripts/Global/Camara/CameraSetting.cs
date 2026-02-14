using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraSetting : MonoBehaviour
{
    #region property
    public static float ScreenWidth
    {
        get
        {
#if UNITY_EDITOR
            return Camera.main.pixelWidth;
#else
            return ScreenWidth;
#endif
        }
    }
    public static float ScreenHeight
    {
        get
        {
#if UNITY_EDITOR
            return Camera.main.pixelHeight;
#else
            return ScreenHeight;
#endif
        }
    }
    #endregion

    #region variable
    private Camera m_oCamera = null;
    [SerializeField] GameObject m_oScalingTarget = null;
    #endregion

    #region function
    //초기화
    public void Awake()
    {
        m_oCamera = GetComponent<Camera>();
        SetUpCamera();

    }
    public void Start()
    {
        SetUpScalingTarget();
    }
    //카메라를 설정한다.
    private void SetUpCamera()
    {
        m_oCamera.orthographic = true;
        m_oCamera.orthographicSize = Global.G_DESIGN_HEIGHT / 2.0f;
        m_oCamera.transform.localRotation = Quaternion.AngleAxis(30, Vector3.right);//카메라를 x축으로 45돌린다.
        m_oCamera.transform.localPosition = new Vector3(0, 1000, -1000);
        m_oCamera.farClipPlane = 5000;
    }
    //현재 디바이스의 해상도에 맞춰서 오브젝트를 스케일한다.
    private void SetUpScalingTarget()
    {
        //디바이스의 해상도
        float fRatio = Global.G_DESIGN_WIDTH / Global.G_DESIGN_HEIGHT;
        float ratio_width = ScreenHeight * fRatio;
        if (ScreenWidth < ratio_width - float.Epsilon)
            m_oScalingTarget.transform.localScale = Vector3.one * (ScreenWidth / ratio_width);
    }
    #endregion
}
