using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class Block : MonoBehaviour
{
   
    void Update()
    {
        //  FIXME
        CheckPosition();
    }

    private void CheckPosition()
    {
        if (Camera.main.transform.position.y - transform.position.y > 25)
        {
            Destroy(this.gameObject);
        }                                                          
    }
}
