using DG.Tweening;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UIElements;

public class CObjectPool : MonoBehaviour
{
    [SerializeField] private CBaseUnit prefab;
    [SerializeField] private Sprite[] unitSprites;
    [SerializeField] private int[] unitSortingOrder;
    [SerializeField] private GameObject m_oParentObject;

    [SerializeField] private Material m_defaultMaterial;
    [SerializeField] private Material m_unitMaterial;
    [SerializeField] private Material m_MilitaryUnitMaterial;
    [SerializeField] private Material m_eMilitaryUnitMaterial;
    [SerializeField] private Material m_MissileMaterial;

    private Queue<CBaseUnit> pool = new Queue<CBaseUnit>();

    public void InitObjectPool(int nSize)
    {
        for (int i = 0; i < nSize; ++i)
        {
            CBaseUnit obj = Instantiate(prefab, Vector3.zero, Quaternion.identity, m_oParentObject.transform);
            Return(obj);
        }
    }

    public CBaseUnit Get(Vector3 position, int unitType)
    {
        CBaseUnit obj;
        if (pool.Count > 0)
        {
            obj = pool.Dequeue();
            obj.gameObject.SetActive(true);
        }
        else
        {
            obj = Instantiate(prefab,position,Quaternion.identity,m_oParentObject.transform);
        }
        obj.transform.position = position;

        obj.m_oSpriteRenderer.sprite = unitSprites[unitType];
        if (unitType == (int)UnitType.MILITARY_UNIT || unitType == (int)UnitType.INFANTRY || unitType == (int)UnitType.ARTILLERY)
        {
            obj.InitializeUnit(m_MilitaryUnitMaterial,m_defaultMaterial, unitSortingOrder[unitType]);
        }
        else if (unitType == (int)UnitType.MILITARY_UNIT_E || unitType == (int)UnitType.INFANTRY_E || unitType == (int)UnitType.ARTILLERY_E)
        {
            obj.InitializeUnit(m_eMilitaryUnitMaterial, m_defaultMaterial, unitSortingOrder[unitType]);
        }
        else if (unitType == (int)UnitType.MISSILE)
        {
            obj.InitializeUnit(m_MissileMaterial,m_MissileMaterial,unitSortingOrder[unitType]);
        }
        else
        {
            Debug.Log("Do not find proper unit type");
            return null;
        }
        return obj;
    }

    public void Return(CBaseUnit obj)
    {
        obj.gameObject.SetActive(false);
        pool.Enqueue(obj);
    }

    public CBaseUnit GetWithFadeIn(Vector3 position, int unitType)
    {
        CBaseUnit obj;
        if (pool.Count > 0)
        {
            obj = pool.Dequeue();
            obj.gameObject.SetActive(true);
        }
        else
        {
            obj = Instantiate(prefab, position, Quaternion.identity, m_oParentObject.transform);
        }
        obj.transform.position = position;

        obj.m_oSpriteRenderer.sprite = unitSprites[unitType];
        if (unitType == (int)UnitType.MILITARY_UNIT || unitType == (int)UnitType.INFANTRY || unitType == (int)UnitType.ARTILLERY)
        {
            obj.InitializeUnit(m_MilitaryUnitMaterial, m_defaultMaterial, unitSortingOrder[unitType]);
        }
        else if (unitType == (int)UnitType.MILITARY_UNIT_E || unitType == (int)UnitType.INFANTRY_E || unitType == (int)UnitType.ARTILLERY_E)
        {
            obj.InitializeUnit(m_eMilitaryUnitMaterial, m_defaultMaterial, unitSortingOrder[unitType]);
        }
        else if (unitType == (int)UnitType.MISSILE)
        {
            obj.InitializeUnit(m_MissileMaterial, m_MissileMaterial, unitSortingOrder[unitType]);
        }
        else
        {
            Debug.Log("Do not find proper unit type");
            return null;
        }

        obj.m_oSpriteRenderer.DOFade(0f, 0f); // 먼저 알파를 0으로 세팅
        obj.m_oSpriteRenderer.DOFade(1f, 0.3f); // 0.3초 동안 페이드 인

        return obj;
    }

    public void ReturnWithFadeOut(CBaseUnit obj)
    {
        obj.m_oSpriteRenderer.DOFade(0f, 0.3f).OnComplete(() =>
        {
            obj.gameObject.SetActive(false);
            pool.Enqueue(obj);
        });
    }
}
