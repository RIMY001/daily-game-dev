using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TransitionManager : MonoBehaviour
{
    private CanvasGroup canvasGroup;
    public float scaler;

    private void Awake()
    {
        canvasGroup = GetComponent<CanvasGroup>();

    }

    private void Start()
    {
        StartCoroutine(Fade(0));
    }

    private IEnumerator Fade(int amount)
    {
        canvasGroup.blocksRaycasts = true;

        while (canvasGroup.alpha != amount)
        {
            switch (amount)
            {
                case 1:
                    canvasGroup.alpha += Time.deltaTime * scaler;
                    break;
                case 0:
                    canvasGroup.alpha -= Time.deltaTime * scaler;
                    break;
            }
            yield return null;
        }

        canvasGroup.blocksRaycasts = false;
    }
}
