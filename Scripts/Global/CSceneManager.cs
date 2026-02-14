using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
using DG.Tweening;

public abstract class CSceneManager : MonoBehaviour
{
    #region property
    public abstract string m_strSceneName { get; }

    #endregion

    #region variable
    [SerializeField] private Image m_oFadeImage;
    private float m_fFadeDuration;
    private bool m_bIsExit = false;

    #endregion
    #region function
    public virtual void Awake() {
        m_fFadeDuration = 0.3f;
        m_oFadeImage.color = Color.black;
    }
    public virtual void Start() {
        SceneFadeIn();
    }
    public virtual void Update() 
    {
        if (m_bIsExit) return;
    }


    public void SceneFadeIn()
    {
        m_oFadeImage.DOFade(0f, m_fFadeDuration);
    }
    public void SceneFadeOut(System.Action onComplete = null)
    {
        m_oFadeImage.DOFade(1f, m_fFadeDuration).OnComplete(() =>
        {
            onComplete?.Invoke();
        });
    }
    protected IEnumerator DelayTime(System.Action onComplete = null)
    {
        // 2초 대기
        yield return new WaitForSeconds(2.0f);

        // 대기 후 실행할 작업
        onComplete?.Invoke();
    }

    private void OnDestroy()
    {
        m_bIsExit = true;
    }
    private void OnApplicationQuit()
    {
        m_bIsExit = true;
    }
    #endregion
}
