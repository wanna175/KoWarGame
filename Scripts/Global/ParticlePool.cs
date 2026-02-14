using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
public class ParticleEntry
{
    public string key; // 식별자 (예: "Explosion", "Smoke")
    public GameObject prefab; // 파티클 프리팹 (root에 ParticleSystem이 있거나 자식에 있어도 됨)
    public int initialSize = 5;
    public bool expandable = true;
}

public class ParticlePool : MonoBehaviour
{
    public List<ParticleEntry> entries = new List<ParticleEntry>();

    // 내부 자료구조
    private Dictionary<string, Queue<GameObject>> _pools = new Dictionary<string, Queue<GameObject>>();
    private Dictionary<string, ParticleEntry> _entryMap = new Dictionary<string, ParticleEntry>();


    void Awake()
    {
        // 엔트리 초기화 및 프리인스턴스
        foreach (var e in entries)
        {
            if (e == null || e.prefab == null || string.IsNullOrEmpty(e.key)) continue;

            _entryMap[e.key] = e;
            var q = new Queue<GameObject>();
            _pools[e.key] = q;

            GameObject parent = new GameObject($"ParticlePool_{e.key}");
            parent.transform.SetParent(transform, false);

            for (int i = 0; i < Mathf.Max(0, e.initialSize); i++)
            {
                var go = InstantiateParticle(e.prefab, parent.transform);
                go.SetActive(false);
                q.Enqueue(go);
            }
        }
    }

    // Play by key (위치/회전)
    public ParticleSystem Play(string key, Vector3 position, Quaternion rotation, Transform parent = null)
    {
        if (!_entryMap.ContainsKey(key))
        {
            Debug.LogWarning($"ParticlePool: 등록되지 않은 key '{key}' 입니다.");
            return null;
        }
        return PlayInternal(_entryMap[key], position, rotation, parent);
    }

    // Play by prefab (직접 prefab을 넘기는 경우)
    public ParticleSystem Play(GameObject prefab, Vector3 position, Quaternion rotation, Transform parent = null, bool allowExpand = true)
    {
        // 임시 엔트리로 동작 (key 미사용)
        var entry = new ParticleEntry { key = prefab.name + "_ad-hoc", prefab = prefab, initialSize = 0, expandable = allowExpand };
        return PlayInternal(entry, position, rotation, parent, adhoc: true);
    }

    // 내부 구현
    private ParticleSystem PlayInternal(ParticleEntry entry, Vector3 position, Quaternion rotation, Transform parent = null, bool adhoc = false)
    {
        Queue<GameObject> q = null;
        if (!adhoc)
        {
            q = _pools[entry.key];
        }
        GameObject instance = null;

        if (q != null && q.Count > 0)
        {
            instance = q.Dequeue();
        }
        else
        {
            if (!adhoc && !entry.expandable)
            {
                Debug.LogWarning($"ParticlePool: '{entry.key}' 풀 비어있고 확장 불가");
                return null;
            }
            // 새로 생성
            Transform poolParent = adhoc ? transform : transform.Find($"ParticlePool_{entry.key}") ?? transform;
            instance = InstantiateParticle(entry.prefab, poolParent);
        }

        instance.transform.SetParent(parent, worldPositionStays: true);
        position.z = 500;
        instance.transform.position = position;
        instance.transform.rotation = rotation;
        instance.SetActive(true);

        // ParticleSystem을 찾아서 Play
        var ps = instance.GetComponent<ParticleSystem>() ?? instance.GetComponentInChildren<ParticleSystem>();
        if (ps == null)
        {
            Debug.LogWarning("ParticlePool: 생성한 오브젝트에 ParticleSystem이 없습니다.");
            return null;
        }

        // 리셋(필요할 수 있음)
        ps.Clear(true);
        ps.Play(true);

        // 재생 끝나면 반환하도록 코루틴 시작
        StartCoroutine(WaitAndReturn(ps.gameObject, entry, adhoc));

        return ps;
    }

    // 인스턴스 생성 헬퍼
    private GameObject InstantiateParticle(GameObject prefab, Transform parent)
    {
        var go = Instantiate(prefab, parent);
        go.name = prefab.name; // 보기 좋게
        // 비활성 상태로 시작 (생성시 바로 활성화 안하려면 호출측에서 활성화)
        go.SetActive(false);
        return go;
    }

    // 파티클이 완전히 끝날 때까지 기다렸다가 반환
    private IEnumerator WaitAndReturn(GameObject particleGO, ParticleEntry entry, bool adhoc)
    {
        var ps = particleGO.GetComponent<ParticleSystem>() ?? particleGO.GetComponentInChildren<ParticleSystem>();
        if (ps == null)
        {
            yield break;
        }

        // IsAlive(true) : 자식 포함 전체 생존 여부
        while (ps.IsAlive(true))
        {
            yield return null;
        }

        // 재생 끝났으면 정리
        particleGO.SetActive(false);

        if (!adhoc)
        {
            _pools[entry.key].Enqueue(particleGO);
            // 부모를 pool parent로 돌려놓음 (선택사항)
            var poolParent = transform.Find($"ParticlePool_{entry.key}");
            if (poolParent != null) particleGO.transform.SetParent(poolParent, false);
        }
        else
        {
            // adhoc인 경우 확장 생성 객체는 그냥 Destroy하거나 풀에 넣을지 결정
            if (entry.expandable)
            {
                // adhoc 확장 객체를 풀에 저장해둘 수도 있음.
                string k = entry.key;
                if (!_pools.ContainsKey(k))
                {
                    _pools[k] = new Queue<GameObject>();
                    _entryMap[k] = entry;
                    var parentGO = new GameObject($"ParticlePool_{k}");
                    parentGO.transform.SetParent(transform, false);
                }
                _pools[k].Enqueue(particleGO);
            }
            else
            {
                Destroy(particleGO);
            }
        }
    }

    // 외부에서 수동으로 반환시키고 싶을 때 사용
    public void ReturnNow(GameObject particleGO, string key)
    {
        var ps = particleGO.GetComponent<ParticleSystem>() ?? particleGO.GetComponentInChildren<ParticleSystem>();
        if (ps != null) ps.Stop(true, ParticleSystemStopBehavior.StopEmittingAndClear);

        particleGO.SetActive(false);
        if (_pools.ContainsKey(key))
        {
            _pools[key].Enqueue(particleGO);
            var poolParent = transform.Find($"ParticlePool_{key}");
            if (poolParent != null) particleGO.transform.SetParent(poolParent, false);
        }
        else
        {
            Destroy(particleGO);
        }
    }
}
