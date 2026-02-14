using UnityEngine;
using TMPro;

public class CMessageDisplayer : MonoBehaviour
{
    [SerializeField] private Transform content;  // ScrollView의 Content (슬롯 6개 부모)
    private bool scrollToBottom = false;

    private void Start()
    {
        CNetworkManager.Instance.m_cMsgDisplayer = this;
    }
    void LateUpdate()
    {
        if (scrollToBottom)
        {
            var scrollRect = content.GetComponentInParent<UnityEngine.UI.ScrollRect>();
            if (scrollRect != null)
                scrollRect.verticalNormalizedPosition = 0f;

            scrollToBottom = false;
        }
    }
    /// <summary>
    /// 새로운 메시지를 표시 (최신 메시지는 항상 맨 아래)
    /// </summary>
    public void ShowMessage(string user, string time, string message)
    {
        // Content의 자식이 하나라도 있어야 함
        if (content.childCount == 0)
            return;

        // 맨 위 슬롯 가져와서 맨 아래로 보냄
        Transform topSlot = content.GetChild(0);
        topSlot.SetAsLastSibling();

        // 슬롯 내용 갱신
        UpdateSlot(topSlot.gameObject, user, time, message);
        topSlot.gameObject.SetActive(true);

        scrollToBottom = true; // 플래그 설정
    }

    /// <summary>
    /// 슬롯 내부 텍스트 3개 업데이트
    /// </summary>
    private void UpdateSlot(GameObject slot, string user, string time, string message)
    {
        var text = slot.GetComponent<CMessageBox>();
        text.SetMessageName(user);
        text.SetMessageTime(time);
        text.SetDescription(message);
    }

    /// <summary>
    /// 모든 슬롯 초기화
    /// </summary>
    public void ClearAll()
    {
        foreach (Transform child in content)
            child.gameObject.SetActive(false);
    }
}
