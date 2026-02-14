using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CScenePrefabManager : CSceneManager
{
    #region property
    public override string m_strSceneName => Global.SCENE_PREFAB;
    #endregion

    #region variable
    #endregion

    #region function
    public override void Awake()
    {
        base.Awake();
    }
    public override void Start()
    {
        base.Start();

    }
    public override void Update()
    {
        base.Update();
        
    }
    private void OnbuttonClick()
    {
    }
   
    #endregion
}
