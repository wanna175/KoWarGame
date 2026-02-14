using UnityEngine;

/// <summary>
/// 씬에 컴포넌트로 존재하는 싱글톤.
/// - 없으면 자동 생성(끄기 가능)
/// - DontDestroyOnLoad로 영속(끄기 가능)
/// - Application 종료 중에는 Instance 반환 안 함(유효성 보호)
/// </summary>
public abstract class Singleton<T> : MonoBehaviour where T : MonoBehaviour
{
    private static T _instance;
    private static readonly object _lock = new object();
    private static bool _applicationIsQuitting;

    /// <summary>없을 때 GameObject를 만들어 자동 생성할지 여부 (타입별 static 이라 각 파생마다 따로 동작)</summary>
    public static bool AutoCreateIfMissing = true;

    /// <summary>Awake에서 DontDestroyOnLoad 적용할지 여부</summary>
    public static bool MakePersistent = true;

    public static T Instance
    {
        get
        {
            if (_applicationIsQuitting) return null;

            lock (_lock)
            {
                if (_instance != null) return _instance;

#if UNITY_2023_1_OR_NEWER
                _instance = FindFirstObjectByType<T>(FindObjectsInactive.Include);
#else
                _instance = FindObjectOfType<T>();
#endif
                if (_instance != null) return _instance;

                if (!AutoCreateIfMissing) return null;

                var go = new GameObject(typeof(T).Name + " (Singleton)");
                _instance = go.AddComponent<T>();

                if (MakePersistent && _instance is MonoBehaviour mb)
                    DontDestroyOnLoad(mb.gameObject);

                return _instance;
            }
        }
    }

    protected virtual void Awake()
    {
        if (_instance == null)
        {
            _instance = this as T;

            if (MakePersistent)
                DontDestroyOnLoad(gameObject);
        }
        else if (_instance != this)
        {
            // 중복 방지: 이미 인스턴스가 있으면 자신을 파괴
            Destroy(gameObject);
        }
    }

    protected virtual void OnDestroy()
    {
        // 에디터에서 Play 종료/빌드 종료 시 중복 생성 방지
        if (Application.isPlaying)
            _applicationIsQuitting = true;
    }

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
    private static void ResetStaticsOnDomainReload()
    {
        _applicationIsQuitting = false;
        _instance = null;
        // 옵션 기본값 재설정(원하면 수정 가능)
        AutoCreateIfMissing = true;
        MakePersistent = true;
    }
}
