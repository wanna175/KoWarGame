using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CSimulationManager : MonoBehaviour
{
    public bool m_bIsSimulationStart {  get; private set; }
    [SerializeField] private GameObject m_oPanel;
    [SerializeField] private Button m_oStartButton;

    [SerializeField] private Text m_oSimulationTime;
    [SerializeField] private Text m_txtPacketTest;

    private void Awake()
    {
        m_bIsSimulationStart = false;
        m_oPanel.SetActive(true);
        m_oStartButton.onClick.AddListener(OnSimulationStartBtnClick);
    }
    public void OnSimulationStartBtnClick()
    {
        CNetworkManager.Instance.SendPacketType((byte)NetworkPackets.PACKET_TYPE.REQUEST_SIM_START);
        m_oStartButton.interactable = false;  
    }
    public void RecvSimulationStart()
    {
        m_bIsSimulationStart = true;
        m_oPanel.SetActive(false);
    }
    public void OnUpdateSimulationTimeFromServer(float time)
    {
        m_oSimulationTime.text = time.ToString();
    }
    
    
}
