using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using static UnityEngine.UI.CanvasScaler;

public class CUnitManager : MonoBehaviour
{
    // -------------------
    // 데이터 구조
    // -------------------
    public class UnitData
    {
        public int id;
        public float lat;
        public float lon;
        public int unitType;
        public int health;
        public int attack;
        public bool selected;
        public bool isfade;
        public bool isAttacked;
        public bool isDead;
    }

    private Dictionary<int, UnitData> m_units = new Dictionary<int, UnitData>();
    private Dictionary<int, CBaseUnit> m_activeUnits = new Dictionary<int, CBaseUnit>();
    private UnitData m_oHighlightUnit;
    //private CBaseUnit m_oHighlightUnit;
    private GameMode.MouseMode m_MouseMode;
    // 미사일 id와 파티클 매핑
    private Dictionary<int, GameObject> missileSmokeMap = new Dictionary<int, GameObject>();

    // -------------------
    // UI
    // -------------------
    [SerializeField] private Text m_txtUnitName;
    [SerializeField] private Text m_txtUnitType;
    [SerializeField] private Text m_txtUnitAttack;
    [SerializeField] private Text m_txtUnitHealth;

    [SerializeField] private GameObject m_oUnitMode;
    [SerializeField] private Text m_txtUnitMode;

    //pie menu
    [SerializeField] private CPieMenu m_oPieMenu;

    // -------------------
    // View 관리
    // -------------------
    [SerializeField] private CObjectPool unitPool; // 오브젝트 풀
    [SerializeField] private ParticlePool particlePool; // effect pool
    [SerializeField] private GameObject m_oParentObject;
    private Camera mainCamera;
    [SerializeField] private int poolSize = 200; // 동시에 화면에 나올 수 있는 유닛 최대 수
    private CMapController mapController;


    private bool m_bIsExit = false;
    private void Awake()
    {
        unitPool.InitObjectPool(poolSize);
        ResetUnitInfo();
        mainCamera = Camera.main;
        
        // TODO : 테스트 용 : 부하 테스트도 여기서 하면 됨
        /*for (int i = 0; i < 6; ++i)
        {
            OnUnitUpdateDataFromServer(i, (float)(126.9780f + (0.2 * i)), (float)(37.5665f + 0.2 * i),i);
        }*/
    }
    private void Start()
    {
        // 맵 컨트롤러 찾기
        mapController = FindObjectOfType<CMapController>();
        if (mapController == null)
        {
            Debug.LogError("CMapController not found in scene!");
            return;
        }
    }
    public void StartUnitControl(Vector2 poslatLon, Vector3 posWorld)
    {
        if (m_bIsExit)
        {
            return;
        }
#if UNITY_EDITOR
        if (!Application.isPlaying) return;
#endif

        if (m_oHighlightUnit != null && !m_oPieMenu.IsActive())
        {
            if (m_MouseMode.Equals(GameMode.MouseMode.None))
            {
                m_activeUnits[m_oHighlightUnit.id].OnDelSelect();
                ResetUnitInfo();
            }
        }
        if (m_oHighlightUnit != null && m_oPieMenu.IsActive() && Input.GetKeyDown(KeyCode.M))
        {
            m_oPieMenu.OnMoveBtnClick();
        }
        else if (m_oHighlightUnit != null && m_oPieMenu.IsActive() && Input.GetKeyDown(KeyCode.A))
        {
            m_oPieMenu.OnAttackBtnClick();
        }
        else if (m_oHighlightUnit != null && m_oPieMenu.IsActive() && Input.GetKeyDown(KeyCode.D))
        {
            m_oPieMenu.OnXButtonClick();
        }
        if (EventSystem.current != null && EventSystem.current.IsPointerOverGameObject())
            return; // UI 위에서는 무시

        //move mode일때
        if (m_MouseMode.Equals(GameMode.MouseMode.Move))
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                SetUnitModeDefault();
                return;
            }
            else if (Input.GetMouseButtonDown(0))
            {
                //Debug.Log(poslatLon.y +" "+ poslatLon.x);
                //마우스를 클릭했을때 위도 경도를 
                CNetworkManager.Instance.SendPacketTypePos((byte)NetworkPackets.PACKET_TYPE.REQUEST_ORDER_IN_SALLY,(int)(poslatLon.y*1000),(int)(poslatLon.x*1000), (short)m_oHighlightUnit.id);
            }
            //점선 그리기
        }
        //attack mode일때
        if (m_MouseMode.Equals(GameMode.MouseMode.Attack))
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                SetUnitModeDefault();
                return;
            }
            else if (Input.GetMouseButtonDown(0))
            {
                if (m_oHighlightUnit.unitType == (int)UnitType.ARTILLERY || m_oHighlightUnit.unitType == (int)UnitType.ARTILLERY_E)
                {
                    CNetworkManager.Instance.SendPacketTypePos((byte)NetworkPackets.PACKET_TYPE.REQUEST_ORDER_IN_ATTACK, (int)(poslatLon.y * 1000), (int)(poslatLon.x * 1000), (short)m_oHighlightUnit.id);
                }
            }
        }

        if (Input.GetMouseButtonUp(0) && m_MouseMode.Equals(GameMode.MouseMode.None)) // 좌클릭
        {
            UnitData nearest = FindNearestUnit(poslatLon.y, poslatLon.x);

            if (m_oHighlightUnit != null)
            {
                m_activeUnits[m_oHighlightUnit.id].OnDelSelect();
                m_oHighlightUnit.selected = false;
            }


            if (nearest != null)
            {
                nearest.selected = true;
                m_oHighlightUnit = nearest;
                UpdateUI(m_oHighlightUnit);

                //if (m_oHighlightUnit.view != null)
                m_oPieMenu.OnSelectUnit(LatLonToWorldPosition(m_oHighlightUnit));
                m_activeUnits[m_oHighlightUnit.id].OnSelect();
            }
            else
            {
                ResetUnitInfo();
            }

        }
    }
    public Vector3 LatLonToWorldPosition(UnitData unit)
    {
        if (mapController == null) return Vector3.zero;

        Vector2 latLon = new Vector2(unit.lon, unit.lat);

        Vector3 worldPos = mapController.LatLonToWorld(latLon);
        return worldPos;
    }

    // -------------------
    // 서버 데이터 갱신
    // -------------------
    public void OnUnitUpdateDataFromServer(int id, float lat, float lon, int type = 0, int hp = 100, int atk = 10)
    {
        if (!m_units.ContainsKey(id))
        {
            //Debug.Log("new obj 생성");
            m_units[id] = new UnitData
            {
                id = id,
                lat = lat,
                lon = lon,
                unitType = type,
                health = hp,
                attack = atk,
                selected = false,
                isfade = true,
                isAttacked = false,
                isDead = false,
            };
        }
        else
        {
            m_units[id].lat = lat; 
            m_units[id].lon = lon;
            m_units[id].health = hp;
            m_units[id].attack = atk;
            m_units[id].isfade = false;            
        }
    }
    public void OnUnitAttackedFromServer(int id)
    {
        if (m_units.ContainsKey(id))
        {
            m_units[id].isAttacked = true;
        }
        
    }
    public void OnUnitDestroyFromServer(int id)
    {
        if (m_units.ContainsKey(id))
        {
            m_units[id].isDead = true;
        }
    }
    // -------------------
    // 가장 가까운 유닛 찾기
    // -------------------
    private UnitData FindNearestUnit(float lat, float lon, float maxDist = 0.1f)
    {
        UnitData nearest = null;
        float minDist = maxDist;
        //TODO : 공간을 나눠서 처리해야 될 필요성이 있음...나중에 고려해보자
        foreach (var kvp in m_units)
        {
            UnitData unit = kvp.Value;

            //만일 미사일 이라면 선택하지 않음
            if (unit.unitType == (int)UnitType.MISSILE) continue;

            // 단순 유클리드 거리 계산 (lat/lon 차이)
            float dx = unit.lat - lat;
            dx = dx > 0 ? dx : -dx;
            float dy = unit.lon - lon;
            dy = dy > 0 ? dy : -dy;
            
            float dist = dx + dy;

            if (dist < minDist)
            {
                minDist = dist;
                nearest = unit;
            }
        }
        return nearest; // 범위 내에 없으면 null
    }
    // -------------------
    // 유닛 렌더링 관리
    // -------------------
    public void RenderVisibleUnits()
    {
        if (m_bIsExit)
        {
            return;
        }
#if UNITY_EDITOR
        if (!Application.isPlaying) return;
#endif
        HashSet<int> visibleNow = new HashSet<int>();

        //TODO : 공간을 나눠서 처리해야 될 필요성이 있음...나중에 고려해보자
        foreach (var unit in m_units)
        {
            Vector3 unitPos = LatLonToWorldPosition(unit.Value);
            Vector3 viewportPos = mainCamera.WorldToViewportPoint(unitPos);

            bool isVisible =
                viewportPos.x >= 0 && viewportPos.x <= 1 &&
                viewportPos.y >= 0 && viewportPos.y <= 1;
            if (isVisible)
            {
                visibleNow.Add(unit.Value.id);

                if (!m_activeUnits.ContainsKey(unit.Value.id))
                {
                    // 풀에서 꺼내오기
                    if (unit.Value.isfade)
                    {
                        var obj = unitPool.GetWithFadeIn(unitPos, unit.Value.unitType);
                        m_activeUnits[unit.Value.id] = obj;
                        unit.Value.isfade = false;
                        // 미사일 생성 시
                        if (m_units[unit.Value.id].unitType == (int)UnitType.MISSILE)
                        {
                            Vector3 missilePos = LatLonToWorldPosition(m_units[unit.Value.id]);
                            var ps = particlePool.Play("Smoke", missilePos, Quaternion.identity, m_oParentObject.transform);
                            if (ps != null)
                                missileSmokeMap[unit.Value.id] = ps.gameObject;
                        }
                    }
                    else
                    {
                        var obj = unitPool.Get(unitPos, unit.Value.unitType);
                        m_activeUnits[unit.Value.id] = obj;
                        // 미사일 생성 시
                        if (m_units[unit.Value.id].unitType == (int)UnitType.MISSILE)
                        {
                            Vector3 missilePos = LatLonToWorldPosition(m_units[unit.Value.id]);
                            var ps = particlePool.Play("Smoke", missilePos, Quaternion.identity, m_oParentObject.transform);
                            if (ps != null)
                                missileSmokeMap[unit.Value.id] = ps.gameObject;
                        }

                        //피격시
                        if (unit.Value.isAttacked)
                        {
                            obj.OnAttacked();
                            unit.Value.isAttacked = false;
                        }
                    }
                }
                else
                {
                    // 현재 위치
                    Vector3 currentPos = m_activeUnits[unit.Value.id].transform.position;

                    // 목표 위치
                    Vector3 targetPos = unitPos;

                    // 보간 속도 (값이 클수록 빨라짐)
                    float lerpSpeed = 2.5f;

                    // 선형 보간으로 위치 갱신
                    m_activeUnits[unit.Value.id].transform.position =
                        Vector3.Lerp(currentPos, targetPos, Time.deltaTime * lerpSpeed);
                    // 미사일일 경우 꼬리 연기도 위치를 갱신
                    if (m_units[unit.Value.id].unitType == (int)UnitType.MISSILE)
                    {
                        if (missileSmokeMap.TryGetValue(unit.Value.id, out var smoke))
                        {
                            // 꼬리 연기를 항상 미사일 위치로 이동시킴
                            smoke.transform.position = m_activeUnits[unit.Value.id].transform.position;
                        }
                    }

                    //피격시
                    if (unit.Value.isAttacked)
                    {
                        m_activeUnits[unit.Value.id].OnAttacked();
                        unit.Value.isAttacked = false;
                    }
                }
            }
        }

        // 이번 프레임에 안 보이면/ 객체가 죽었다면 풀로 반환
        var keys = new List<int>(m_activeUnits.Keys);
        foreach (var id in keys)
        {
            if (!visibleNow.Contains(id))
            {
                var obj = m_activeUnits[id];
                if (missileSmokeMap.TryGetValue(id, out var smoke))
                {
                    particlePool.ReturnNow(smoke, "Smoke");
                    missileSmokeMap.Remove(id);
                }
                unitPool.Return(obj);
                m_activeUnits.Remove(id);
            }
            else if (m_units[id].isDead)
            {
                var obj = m_activeUnits[id];
                //미사일이라면 폭발효과를 낸다.
                if (m_units[id].unitType == (int)UnitType.MISSILE)
                {
                    ParticleSystem objParticle = obj.GetComponent<ParticleSystem>();
                    Vector3 unitPos = LatLonToWorldPosition(m_units[id]);
                    particlePool.Play("Explosion", unitPos, Quaternion.identity,m_oParentObject.transform);
                    if (missileSmokeMap.TryGetValue(id, out var smoke))
                    {
                        particlePool.ReturnNow(smoke, "Smoke");
                        missileSmokeMap.Remove(id);
                    }
                }
                m_units.Remove(id);
                m_activeUnits.Remove(id);
                unitPool.ReturnWithFadeOut(obj);
            }
        }
    }
   

    // -------------------
    // UI & 모드 관리
    // -------------------
    private void UpdateUI(UnitData unit)
    {
        string strUnitType;
        switch (unit.unitType)
        {
            case (int)UnitType.MILITARY_UNIT:
                strUnitType = "Military Unit";
                break;
            case (int)UnitType.MILITARY_UNIT_E:
                strUnitType = "Military Unit";
                break;
            case (int)UnitType.INFANTRY:
                strUnitType = "Infantry";
                break;
            case (int)UnitType.INFANTRY_E:
                strUnitType = "Infantry";
                break;
            case (int)UnitType.ARTILLERY:
                strUnitType = "Artillery";
                break;
            case (int)UnitType.ARTILLERY_E:
                strUnitType = "Artillery";
                break;
            default:
                strUnitType = "None";
                break;
        }
        m_txtUnitName.text = strUnitType;
        m_txtUnitType.text = strUnitType;
        m_txtUnitAttack.text = "x " + unit.attack;
        m_txtUnitHealth.text = "x " + unit.health;
    }
    private void ResetUnitInfo()
    {
        m_MouseMode = GameMode.MouseMode.None;
        m_oHighlightUnit = null;
        m_txtUnitName.text = " - ";
        m_txtUnitType.text = " - ";
        m_txtUnitAttack.text = "x - ";
        m_txtUnitHealth.text = "x - ";
        m_oUnitMode.SetActive(false);
    }
    
    public void SetUnitModeMove()
    {
        m_MouseMode = GameMode.MouseMode.Move;
        m_txtUnitMode.text = "M mode";
        m_oUnitMode.SetActive(true);
    }
    public void SetUnitModeAttack()
    {
        m_MouseMode = GameMode.MouseMode.Attack;
        m_txtUnitMode.text = "A mode";
        m_oUnitMode.SetActive(true);
    }
    public void SetUnitModeDefault()
    {
        m_MouseMode = GameMode.MouseMode.None;
        m_oUnitMode.SetActive(false);
    }
    private void OnDestroy()
    {
        m_bIsExit = true;
        m_activeUnits.Clear();
        m_units.Clear();
    }
    private void OnApplicationQuit()
    {
        m_bIsExit = true;
    }
}
