using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Spawner : MonoBehaviour
{
    public int direction;
    public List<GameObject> spawnObjects;

    private void Start()
    {
        //  重复生成
        InvokeRepeating(nameof(Spawn), 2f, Random.Range(5f, 7f));
    }

    private void Spawn()
    {
        int index = Random.Range(0, spawnObjects.Count);
        //  生成小车
        GameObject target = Instantiate(spawnObjects[index], transform.position, Quaternion.identity, transform);
        //  关联另外脚本的dir用以调整小车出现的方向
        target.GetComponent<MoveForward>().dir = direction;
    }
}
