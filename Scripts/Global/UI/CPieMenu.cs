using UnityEngine;

public class CPieMenu : MonoBehaviour
{
    private Animator m_Animator;
    [SerializeField] private GameObject m_PieCanvas;
    [SerializeField] private GameObject m_BG;

    [SerializeField] private CUnitManager m_UnitManager;

    private CBaseUnit m_oSelectUnit;
    private void Awake()
    {
        m_Animator = GetComponent<Animator>();
    }
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private void OnEnable()
    {
        m_Animator.SetTrigger("ActiveTrue");
    }

    private void OnEndAnimation()
    {
        //this.gameObject.SetActive(false);
       // m_BG.SetActive(false);
        m_PieCanvas.SetActive(false);
        
    }
    public void OnXButtonClick()
    {
        m_UnitManager.SetUnitModeDefault();
        m_Animator.SetTrigger("ActiveFalse");
    }
    public void OnMoveBtnClick()
    {
        m_UnitManager.SetUnitModeMove();
        m_Animator.SetTrigger("ActiveFalse");
    }
    public void OnAttackBtnClick()
    {
        m_UnitManager.SetUnitModeAttack();
        m_Animator.SetTrigger("ActiveFalse");
    }
    public void OnSelectUnit(Vector3 unitPos)
    {
        this.transform.position = unitPos;
        m_PieCanvas.SetActive(true);
        
    }
    public bool IsActive()
    {
        return m_PieCanvas.activeSelf;
    }
}
