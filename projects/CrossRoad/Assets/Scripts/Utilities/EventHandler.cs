using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EventHandler
{
    public static event Action<int> GetPointEvent;

    public static void CallGetPointEvent(int point)
    {
        //if (GetPointEvent != null)
        //{
        //    GetPointEvent.Invoke(point);
        //}
        GetPointEvent?.Invoke(point);
    }
}
