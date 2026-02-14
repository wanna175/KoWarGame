using DG.Tweening;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CPopupManager : MonoBehaviour
{
    [SerializeField] private List<CanvasGroup> m_vecPopupList;
    private Dictionary<string, CanvasGroup> m_dicPopupDictionary = new Dictionary<string, CanvasGroup>();
    // Start is called before the first frame update
    private void Awake()
    {
        foreach (var popup in m_vecPopupList)
        {
            popup.gameObject.SetActive(false);
            popup.alpha = 0;
            m_dicPopupDictionary.Add(popup.name, popup);
        }
    }
  
    public void ShowPopup(string popupName, bool isAnimation)
    {
        if (!m_dicPopupDictionary.ContainsKey(popupName)) return;

        var popup = m_dicPopupDictionary[popupName];
        popup.gameObject.SetActive(true);

        if (!isAnimation) 
        {
            popup.alpha = 0;
            popup.DOFade(1, 0.15f);
            return;
        }

        popup.alpha = 0;
        popup.transform.localScale = Vector3.zero;

        Sequence seq = DOTween.Sequence();
        seq.Append(popup.DOFade(1, 0.3f));
        seq.Join(popup.transform.DOScale(Vector3.one, 0.3f).SetEase(Ease.OutBack));
    }
    
    public void HidePopup(string popupName, bool isAnimation)
    {
        if (!m_dicPopupDictionary.ContainsKey(popupName)) return;

        var popup = m_dicPopupDictionary[popupName];

        if (!isAnimation)
        {
            popup.gameObject.SetActive(false);
            popup.DOFade(0, 0.15f);
            return;
        }

        Sequence seq = DOTween.Sequence();
        seq.Append(popup.DOFade(0, 0.3f));
        seq.Join(popup.transform.DOScale(Vector3.zero, 0.3f).SetEase(Ease.InBack))
           .OnComplete(() => popup.gameObject.SetActive(false));
    }

    public void ShowTimedPopup(string popupName, float duration = 3f, bool isAnimation = true)
    {
        // 먼저 팝업을 보여주고
        ShowPopup(popupName, isAnimation);

        // duration(기본 2초) 뒤에 자동으로 닫기
        DOVirtual.DelayedCall(duration, () =>
        {
            HidePopup(popupName, isAnimation);
        });
    }

}
