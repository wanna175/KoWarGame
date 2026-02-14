
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class CBaseUnit : MonoBehaviour
{
    public SpriteRenderer m_oSpriteRenderer { get; set; }
    int m_nDefaultSortingOrder;

    public string m_strUnitType { get; }

    private Material mat;
    private Material defaultMat;

    #region ÇÔ¼ö
    private void Awake()
    {
        m_oSpriteRenderer = GetComponent<SpriteRenderer>();
    }
    public void InitializeUnit(Material mat,Material defaultMat, int sortingOrder)
    {
        this.mat = mat;
        this.defaultMat = defaultMat;
        m_oSpriteRenderer.material = defaultMat;
        //mat.SetFloat("_GlowStrength", isHighlighted ? 2f : 0f);
        m_nDefaultSortingOrder = sortingOrder; 
        m_oSpriteRenderer.sortingOrder = sortingOrder;
    }

    public virtual void OnSelect()
    {
        m_oSpriteRenderer.material = mat;

        mat.SetFloat("_GlowStrength", 2f);

        m_oSpriteRenderer.sortingOrder = 11;
    }
    public virtual void OnDelSelect()
    {
        m_oSpriteRenderer.material = defaultMat;
        //mat.SetFloat("_GlowStrength", 0f);
        
        m_oSpriteRenderer.sortingOrder = m_nDefaultSortingOrder;
    }
    public virtual void OnAttacked()
    {
        StartCoroutine(HitFlash());
    }
    IEnumerator HitFlash()
    {
        Color origin = m_oSpriteRenderer.material.color;
        m_oSpriteRenderer.material.color = Color.white;
        yield return new WaitForSeconds(0.2f);
        m_oSpriteRenderer.material.color = origin;
    }
    #endregion
}
