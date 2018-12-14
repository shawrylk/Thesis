using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
public class bpsPinMove : MonoBehaviour
{

    private Vector3 distance;
    private float positionX;
    private float positionY;
    public Vector3 position;
    private const int activeRegion = 80;
    private void OnMouseDown()
    {
        distance = Camera.main.WorldToScreenPoint(transform.position);
        positionX = Input.mousePosition.x - distance.x;
        positionY = Input.mousePosition.y - distance.y;
    }

    private void OnMouseDrag()
    {
        Vector3 mousePosition = new Vector3(Input.mousePosition.x - positionX, Input.mousePosition.y - positionY, distance.z);
        Vector3 objPosition = Camera.main.ScreenToWorldPoint(mousePosition);
        position = objPosition;
        transform.position = objPosition;

    }

    private void Awake()
    {
        position = new Vector3(0, -449, -500);
}

}