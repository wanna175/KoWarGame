using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;

public class MultiTogglePopupController : MonoBehaviour
{
    [System.Serializable]
    public class TogglePopupPair
    {
        public Toggle toggle;
        public string popupName;
        public bool bIsAnimation;        //default popup이면 false
    }

    [SerializeField] private List<TogglePopupPair> m_vecToggleList;
    [SerializeField] private CPopupManager m_cPopupManager;

    private void Start()
    {

        foreach (var pair in m_vecToggleList)
        {
            var toggle = pair.toggle;     
            var popupName = pair.popupName;
            var isAnimation = pair.bIsAnimation;

            toggle.onValueChanged.AddListener((isOn) =>
            {
                if (isOn)
                    m_cPopupManager.ShowPopup(popupName,isAnimation);
                else
                    m_cPopupManager.HidePopup(popupName, isAnimation);
            });

            // 시작 상태 반영
            if (toggle.isOn)
                m_cPopupManager.ShowPopup(popupName, isAnimation);
        }
    }
}
