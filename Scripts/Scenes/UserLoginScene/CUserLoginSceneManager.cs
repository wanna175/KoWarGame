using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CUserLoginSceneManager : CSceneManager
{
    #region property
    public override string m_strSceneName => Global.USER_LOGIN_SCENE;
    #endregion

    #region variable
    [SerializeField] private InputField m_oEmailInputField;
    [SerializeField] private InputField m_oPasswordInputField;

    [SerializeField] private CPopupManager m_cPopupManager;
    #endregion

    #region function
    public override void Awake()
    {
        base.Awake();
    }
    public override void Start()
    {
        base.Start();
        if (CNetworkManager.Instance.m_bIsConnect == false)
        {
            ShowNetworkConnectionFailPopup();
        }
    }
    public override void Update()
    {
        base.Update();
        
    }

    private void ShowNetworkConnectionFailPopup()
    {
        const string strPopupName = "ConnectionFail_popup";

        m_cPopupManager.ShowPopup(strPopupName, true);
    }
    public void OnLoginButtonClick()
    {
        const string strPopupName = "Connection_popup";
        const string strPopupNameLoginFail = "LoginFailure_popup";
        Debug.Log(m_oEmailInputField.text);

        m_cPopupManager.ShowPopup(strPopupName, true);

        LoginProcessStart();
        //popup 띄우고 로그인 기다림
        StartCoroutine(WaitForLogin(() =>
        {
            if (CNetworkManager.Instance.m_bIsLoginFailure)
            {
                m_cPopupManager.HidePopup(strPopupName,false);
                m_cPopupManager.ShowPopup(strPopupNameLoginFail, false);
                return;
            }
            SceneFadeOut(() =>
            {
                CLoadingSceneManager.LoadScene(Global.MAIN_SCENE);
            });
        }));
        
    }
    public void OnStayButtonClick()
    {
        const string strPopupName = "Exit_popup";

        m_cPopupManager.HidePopup(strPopupName,true);
    }
    public void OnExitButtonClick()
    {
        const string strPopupName = "Exit_popup";

        m_cPopupManager.ShowPopup(strPopupName, true);
    }

    public void OnLoginFailureConfirmClick()
    {
        const string strPopupName = "LoginFailure_popup";

        m_cPopupManager.HidePopup(strPopupName, true);
    }
    private void LoginProcessStart()
    {
        string strUserEmail = m_oEmailInputField.text;
        string strUserPassword = m_oPasswordInputField.text;
        Debug.Log("login process start");
        CNetworkManager.Instance.SendPacketLogin((byte)NetworkPackets.PACKET_TYPE.REQUEST_LOGIN,strUserEmail,strUserPassword);
    }

    protected IEnumerator WaitForLogin(System.Action onComplete = null)
    {
        //2초대기
        yield return new WaitForSeconds(2.0f);
        // loginSuccess 또는 fail 중 하나가 true 될 때까지 대기
        while (!CNetworkManager.Instance.m_bIsLoginSuccess && !CNetworkManager.Instance.m_bIsLoginFailure)
        {
            yield return null; // 다음 프레임까지 대기
        }

        // 대기 종료 후 실행할 작업
        onComplete?.Invoke();
    }

    #endregion
}
