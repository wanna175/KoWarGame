using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CAchievementManager : MonoBehaviour
{
    [SerializeField] Transform m_parent;               // 업적 UI를 담을 부모 (ex. ScrollView Content)
    [SerializeField] GameObject m_prefabAchievement;   // 업적 프리팹 (CAchievement 붙어있는 오브젝트)


    [SerializeField] private CPopupManager m_cPopupManager;
    [SerializeField] private string m_popupName;

    //done된 achievement
    [SerializeField] private Text m_achieveTitle;

    struct Achievement
    {
        public int id;
        public string name;
        public string description;
        public int total;
        public int current;
        public bool isDone;

        public Achievement(int id, string name, string description, int total, int current, bool isDone)
        {
            this.id = id;
            this.name = name;
            this.description = description;
            this.total = total;
            this.current = current;
            this.isDone = isDone;
        }
    }

    private List<Achievement> m_listAchievements = new List<Achievement>();
    private Dictionary<int, CAchievement> m_dicUI = new Dictionary<int, CAchievement>();

    void Start()
    {
        // 예시 업적 등록 ( 나중에  current 값 user database 에서 받아와야함)
        AddAchievement(0, Global.ACHIEVEMENT_TITLE_01, Global.ACHIEVEMENT_CONTENT_01, Global.ACHIEVEMENT_TOTAL_01, 0, false);
        AddAchievement(1, Global.ACHIEVEMENT_TITLE_02, Global.ACHIEVEMENT_CONTENT_02, Global.ACHIEVEMENT_TOTAL_02, 0, false);
        AddAchievement(2, Global.ACHIEVEMENT_TITLE_03, Global.ACHIEVEMENT_CONTENT_03, Global.ACHIEVEMENT_TOTAL_03, 0, false);
        AddAchievement(3, Global.ACHIEVEMENT_TITLE_04, Global.ACHIEVEMENT_CONTENT_04, Global.ACHIEVEMENT_TOTAL_04, 0, false);
        AddAchievement(4, Global.ACHIEVEMENT_TITLE_05, Global.ACHIEVEMENT_CONTENT_05, Global.ACHIEVEMENT_TOTAL_05, 0, false);
        AddAchievement(5, Global.ACHIEVEMENT_TITLE_06, Global.ACHIEVEMENT_CONTENT_06, Global.ACHIEVEMENT_TOTAL_06, 0, false);
        AddAchievement(6, Global.ACHIEVEMENT_TITLE_07, Global.ACHIEVEMENT_CONTENT_07, Global.ACHIEVEMENT_TOTAL_07, 0, false);
        AddAchievement(7, Global.ACHIEVEMENT_TITLE_08, Global.ACHIEVEMENT_CONTENT_08, Global.ACHIEVEMENT_TOTAL_08, 0, false);
        AddAchievement(8, Global.ACHIEVEMENT_TITLE_09, Global.ACHIEVEMENT_CONTENT_09, Global.ACHIEVEMENT_TOTAL_09, 0, false);
        AddAchievement(9, Global.ACHIEVEMENT_TITLE_10, Global.ACHIEVEMENT_CONTENT_10, Global.ACHIEVEMENT_TOTAL_10, 0, false);
        
        CNetworkManager.Instance.m_cAchievementManager = this;
    }

    /// <summary>
    /// 업적 추가 및 UI 생성
    /// </summary>
    void AddAchievement(int id, string name, string desc, int total, int current, bool isDone)
    {
        Achievement ach = new Achievement(id, name, desc, total, current, isDone);
        m_listAchievements.Add(ach);

        // UI 프리팹 생성
        GameObject obj = Instantiate(m_prefabAchievement, m_parent);
        CAchievement comp = obj.GetComponent<CAchievement>();

        // CAchievement 초기화
        comp.InitializeAchievement(name, desc, total, current, isDone);

        m_dicUI.Add(id, comp);
    }

    /// <summary>
    /// 업적 진행도 갱신
    /// </summary>
    public void AddProgress(int id, int value = 1)
    {
        if (m_listAchievements[id].id == id)
        {
            Achievement ach = m_listAchievements[id];
            if (ach.isDone) return;

            ach.current += value;
            if (ach.current >= ach.total)
            {
                ach.current = ach.total;
                ach.isDone = true;
            }

            m_listAchievements[id] = ach;
            if (ach.isDone)
            {
                SetUnlockedAchievement(id);
                m_cPopupManager.ShowTimedPopup(m_popupName);
            }
                // UI 갱신
            if (m_dicUI.ContainsKey(id))
            { 
                m_dicUI[id].AddCurrentValue(ach.current);
            }
        }
    }
    /// <summary>
    /// button 관련
    /// </summary>
    public void AllToggleOn()
    {
        foreach (var kv in m_dicUI)
        {
            kv.Value.gameObject.SetActive(true); // 전체 보이기
        }
    }

    public void ToDoToggleOn()
    {
        foreach (var kv in m_dicUI)
        {
            // 아직 완료되지 않은 업적만 보이기
            kv.Value.gameObject.SetActive(!kv.Value.IsFinished());
        }
    }

    public void CompleteToggleOn()
    {
        foreach (var kv in m_dicUI)
        {
            // 완료된 업적만 보이기
            kv.Value.gameObject.SetActive(kv.Value.IsFinished());
        }
    }

    private void SetUnlockedAchievement(int id)
    {
        string txt;
        switch (id) 
        {
            case 0: txt = Global.ACHIEVEMENT_TITLE_01; break;
            case 1: txt = Global.ACHIEVEMENT_TITLE_02; break;
            case 2: txt = Global.ACHIEVEMENT_TITLE_03; break;
            case 3: txt = Global.ACHIEVEMENT_TITLE_04; break;
            case 4: txt = Global.ACHIEVEMENT_TITLE_05; break;
            case 5: txt = Global.ACHIEVEMENT_TITLE_06; break;
            case 6: txt = Global.ACHIEVEMENT_TITLE_07; break;
            case 7: txt = Global.ACHIEVEMENT_TITLE_08; break;
            case 8: txt = Global.ACHIEVEMENT_TITLE_09; break;
            case 9: txt = Global.ACHIEVEMENT_TITLE_10; break;
            default:txt = "None"; break;
        }
        m_achieveTitle.text = txt;
    }

}
