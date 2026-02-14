using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CAchievement : MonoBehaviour
{
    [SerializeField] Text m_txtName;
    [SerializeField] Text m_txtDescription;
    [SerializeField] Slider m_slider;
    [SerializeField] GameObject m_oSuccessPanel;

    private int m_nTotalValue;
    private int m_nCurrentValue;
    private bool m_isFinished;

    public void InitializeAchievement(string Name,string content,int total, int current, bool isFin)
    {
        m_txtName.text = Name;
        m_txtDescription.text = content;
        m_nTotalValue = total;
        m_nCurrentValue = current;
        m_isFinished = isFin;

        IsCompleteAchievement();

    }
    public void AddCurrentValue(int currentValue)
    {
        if (m_isFinished) return;

        m_nCurrentValue = currentValue;
        if (m_nTotalValue <=  m_nCurrentValue) m_isFinished = true;

        IsCompleteAchievement();
    }
    public void IsCompleteAchievement()
    {
        if (m_isFinished)
        {
            m_oSuccessPanel.SetActive(true);
            m_slider.gameObject.SetActive(false);
        }
        else
        {
            m_slider.value = (float)m_nCurrentValue / m_nTotalValue;
            m_oSuccessPanel.SetActive(false);
            m_slider.gameObject.SetActive(true);
        }
    }
    public bool IsFinished()
    {
        return m_isFinished;
    }
    


}
