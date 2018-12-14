using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class bpsCircle : MonoBehaviour
{

    public int vertexCount = 40;
    public float lineWidth = 5f;
    private const int Y_OFFSET = -449;
    private LineRenderer lineRenderer;

    private void Awake()
    {
        lineRenderer = GetComponent<LineRenderer>();
    }

    public void SetupCirlce(float radius, Vector3 center)
    {
        lineRenderer.widthMultiplier = lineWidth;
        float deltaTheta = (2f * Mathf.PI) / vertexCount;
        float theta = 0f;
        lineRenderer.positionCount = vertexCount;
        for (int i = 0; i < lineRenderer.positionCount; i++)
        {
            Vector3 pos = new Vector3(radius * Mathf.Cos(theta) + center.x, Y_OFFSET, radius * Mathf.Sin(theta) + center.z);
            lineRenderer.SetPosition(i, pos);
            theta += deltaTheta;
        }

    }

}
