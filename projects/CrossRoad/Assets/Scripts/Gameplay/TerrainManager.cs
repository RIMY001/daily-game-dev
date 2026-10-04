using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TerrainManager : MonoBehaviour
{
    public float offsetY;
    public List<GameObject> terrainObjects;
    private GameObject spawnObject;
    private int lastIndex;

    //private void Start()
    //{
    //    CheckPosition();
    //}

    public void CheckPosition()
    {
        Debug.Log($"进入CheckPosition 管理器Y:{transform.position.y} 相机Y:{Camera.main.transform.position.y} 差值:{transform.position.y - Camera.main.transform.position.y}");
        if (transform.position.y - Camera.main.transform.position.y < offsetY / 2)
        {
            Debug.Log("====满足条件，移动管理器+生成地形====");
            transform.position = new Vector3(0, Camera.main.transform.position.y + offsetY, 0);
            Debug.Log($"赋值完成后，管理器Y = {transform.position.y}");
            SpawnTerrain();
        }
    }

    private void SpawnTerrain()
    {
        var randomIndex = Random.Range(0, terrainObjects.Count);

        while (lastIndex == randomIndex)
        {
            randomIndex = Random.Range(0, terrainObjects.Count);
        }
        lastIndex = randomIndex;

        spawnObject = terrainObjects[randomIndex];

        Instantiate(spawnObject, transform.position, Quaternion.identity);
    }
}
