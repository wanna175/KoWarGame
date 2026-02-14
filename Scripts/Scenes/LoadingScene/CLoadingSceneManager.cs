using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class CLoadingSceneManager : CSceneManager
{
    #region 프로퍼티
    public override string m_strSceneName => Global.LOADING_SCENE;
    #endregion

    #region 변수
    private static string m_strNextSceneName;
    [SerializeField] private GameObject circle_bar = null;
    #endregion

    #region 함수
    public override void Start()
    {
        base.Start();
        Time.timeScale = 1.0f;
        StartCoroutine(LoadSceneAsync(m_strNextSceneName, LoadSceneMode.Single));
    }
    public override void Update()
    {
        base.Update();
        circle_bar.transform.Rotate(0, 0, 300 * Time.deltaTime);
    }
    public static void LoadScene(string strSceneName)
    {
        m_strNextSceneName = strSceneName;
        SceneManager.LoadScene(Global.LOADING_SCENE);
    }
    private IEnumerator LoadSceneAsync(string sceneName, LoadSceneMode loadSceneMode)
    {
        yield return new WaitForSeconds(3.0f);
        var op = SceneManager.LoadSceneAsync(sceneName, loadSceneMode);
        do
        {
         
            yield return new WaitForEndOfFrame();
        } while (!op.isDone);
        yield break;
    }

    #endregion

}
