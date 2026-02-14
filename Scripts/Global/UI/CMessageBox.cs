using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CMessageBox : MonoBehaviour
{
    [SerializeField] private Text m_txtName;
    [SerializeField] private Text m_txtTime;
    [SerializeField] private Text m_txtDescription;
    // Start is called before the first frame update

    public void SetMessageName(string text)
    {
        m_txtName.text = text;
    }
    public void SetMessageTime(string text) 
    {
        m_txtTime.text = text; 
    }
    public void SetDescription(string text) 
    {
        m_txtDescription.text = text; 
    }
}
