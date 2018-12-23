using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using System.IO;
using System.Net;
using System.Text;
using System.Net.Sockets;
using System.Linq;
using System.Threading;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;
using System.Runtime.InteropServices;
using UnityEngine.Events;
using System.Collections;

enum bpsCommandTypeDef : Int16
{
    BPS_UPDATE_PID,
    BPS_MODE_SETPOINT,
    BPS_MODE_CIRCLE,
    BPS_MODE_RECTANGLE,
    BPS_MODE_DEFAULT
};

[Serializable]
struct bpsSocketSendData
{
    public Byte[] data;
    public bpsSocketSendData(Int16 command, Int16 ballXOrdinate, Int16 ballYOrdinate)
    {
        data = new byte[50];
        data[0] = (Byte)command;
        data[1] = (Byte)(command >> 8);
        data[2] = (Byte)ballXOrdinate;
        data[3] = (Byte)(ballXOrdinate >> 8);
        data[4] = (Byte)ballYOrdinate;
        data[5] = (Byte)(ballYOrdinate >> 8);
    }
    public bpsSocketSendData(Int16 command, Int16 ballXOrdinate, Int16 ballYOrdinate, UInt16 radius, UInt16 speed)
    {
        data = new byte[50];
        data[0] = (Byte)command;
        data[1] = (Byte)(command >> 8);
        data[2] = (Byte)ballXOrdinate;
        data[3] = (Byte)(ballXOrdinate >> 8);
        data[4] = (Byte)ballYOrdinate;
        data[5] = (Byte)(ballYOrdinate >> 8);
        data[6] = (Byte)radius;
        data[7] = (Byte)(radius >> 8);
        data[8] = (Byte)speed;
        data[9] = (Byte)(speed >> 8);
    }
    public bpsSocketSendData(Int16 command, Int16 vertexTopLeftXOrdinate, Int16 vertexTopLeftYOrdinate, Int16 vertexTopRightXOrdinate,
                            Int16 vertexTopRightYOrdinate, Int16 vertexBotLeftXOrdinate, Int16 vertexBotLeftYOrdinate,
                            Int16 vertexBotRightXOrdinate, Int16 vertexBotRightYOrdinate)
    {
        data = new byte[50];
        data[0] = (Byte)command;
        data[1] = (Byte)(command >> 8);
        data[2] = (Byte)vertexTopLeftXOrdinate;
        data[3] = (Byte)(vertexTopLeftXOrdinate >> 8);
        data[4] = (Byte)vertexTopLeftYOrdinate;
        data[5] = (Byte)(vertexTopLeftYOrdinate >> 8);
        data[6] = (Byte)vertexTopRightXOrdinate;
        data[7] = (Byte)(vertexTopRightXOrdinate >> 8);
        data[8] = (Byte)vertexTopRightYOrdinate;
        data[9] = (Byte)(vertexTopRightYOrdinate >> 8);
        data[10] = (Byte)vertexBotLeftXOrdinate;
        data[11] = (Byte)(vertexBotLeftXOrdinate >> 8);
        data[12] = (Byte)vertexBotLeftYOrdinate;
        data[13] = (Byte)(vertexBotLeftYOrdinate >> 8);
        data[14] = (Byte)vertexBotRightXOrdinate;
        data[15] = (Byte)(vertexBotRightXOrdinate >> 8);
        data[16] = (Byte)vertexBotRightYOrdinate;
        data[17] = (Byte)(vertexBotRightYOrdinate >> 8);
    }
    public bpsSocketSendData(Int16 command, float KpOuterXAxis, float KpOuterYAxis, float KpInnerXAxis, float KpInnerYAxis,
                                    float KiOuterXAxis, float KiOuterYAxis, float KiInnerXAxis, float KiInnerYAxis,
                                     float KdOuterXAxis, float KdOuterYAxis, float KdInnerXAxis, float KdInnerYAxis)
    {
        data = new byte[50];
        data[0] = (Byte)command;
        data[1] = (Byte)(command >> 8);
        float[] farr = new float[12] { KpOuterXAxis , KpOuterYAxis, KpInnerXAxis, KpInnerYAxis,  KiOuterXAxis, KiOuterYAxis,
                                         KiInnerXAxis, KiInnerYAxis, KdOuterXAxis, KdOuterYAxis, KdInnerXAxis, KdInnerYAxis, };

        Buffer.BlockCopy(farr, 0, data, 2, 48);
    }

}

public class bpsBallAndPlateScript : MonoBehaviour {
    private Client client = new Client();
    private Transform cameraTransform;
    private Transform cameraDesiredLookAt;
    private const float CAMERA_TRANSITION_SPEED = 6.0f;
    private const int BALL_OFFSET = 240;
    private const int Y_OFFSET = -449;
    private const int Z_OFFSET = -500;
    private const int RADIUS_MIN = 30;
    private const int ACTIVE_REGION = 100;
    private const int DISTANCE_MIN = 50;
    private float radius = 70;
    private float oldRadius = 0;
    private int mode = 1;
    private bool logined = false;
    private bool isDataToSend = false;
    Vector3 pinOffset = new Vector3(0, 1, 0);
    Vector3 pin1OldPosition = Vector3.zero;
    Vector3 pin2OldPosition = Vector3.zero;
    Vector3 pin3OldPosition = Vector3.zero;
    Vector3 pin4OldPosition = Vector3.zero;
    Vector3 pinDragPosition = Vector3.zero;
    public GameObject pnlOuterLoop;
    public GameObject pnlInnerLoop;

    private void Start()
    {
        cameraTransform = Camera.main.transform;
        pnlOuterLoop.SetActive(false);
        pnlInnerLoop.SetActive(false);
        GameObject.Find("txtLoginFail").GetComponent<Text>().enabled = false;
        GameObject.Find("pin").GetComponent<Rigidbody>().freezeRotation = true;
        GameObject.Find("pin1").GetComponent<Rigidbody>().freezeRotation = true;
        GameObject.Find("pin2").GetComponent<Rigidbody>().freezeRotation = true;
        GameObject.Find("pin3").GetComponent<Rigidbody>().freezeRotation = true;
        GameObject.Find("pin4").GetComponent<Rigidbody>().freezeRotation = true;
        GameObject.Find("tglGroup").GetComponent<ToggleGroup>().GetComponentsInChildren<Toggle>()[0].
                onValueChanged.AddListener((bool a) => selectSetpoint(a));
        GameObject.Find("tglGroup").GetComponent<ToggleGroup>().GetComponentsInChildren<Toggle>()[1].
                onValueChanged.AddListener((bool a) => selectCirlce(a));
        GameObject.Find("tglGroup").GetComponent<ToggleGroup>().GetComponentsInChildren<Toggle>()[2].
                onValueChanged.AddListener((bool a) => selectRectangle(a));
        GameObject.Find("btnOuterLoop").GetComponent<Button>().onClick.AddListener(() => tglPnlOuterLoop());
        GameObject.Find("btnInnerLoop").GetComponent<Button>().onClick.AddListener(() => tglPnlInnerLoop());

        for (int i = 0; i < 6; i++)
            pnlOuterLoop.GetComponentsInChildren<InputField>()[i].onEndEdit.AddListener((string a) => updatePID(a));
        for (int i = 0; i < 6; i++)
            pnlInnerLoop.GetComponentsInChildren<InputField>()[i].onEndEdit.AddListener((string a) => updatePID(a));

        selectSetpoint(true);

        if (PlayerPrefs.HasKey("KpxInner"))
        {
            pnlInnerLoop.GetComponentsInChildren<InputField>()[0].text = PlayerPrefs.GetFloat("KpxInner").ToString();
            pnlInnerLoop.GetComponentsInChildren<InputField>()[1].text = PlayerPrefs.GetFloat("KixInner").ToString();
            pnlInnerLoop.GetComponentsInChildren<InputField>()[2].text = PlayerPrefs.GetFloat("KdxInner").ToString();
            pnlInnerLoop.GetComponentsInChildren<InputField>()[3].text = PlayerPrefs.GetFloat("KpyInner").ToString();
            pnlInnerLoop.GetComponentsInChildren<InputField>()[4].text = PlayerPrefs.GetFloat("KiyInner").ToString();
            pnlInnerLoop.GetComponentsInChildren<InputField>()[5].text = PlayerPrefs.GetFloat("KdyInner").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[0].text = PlayerPrefs.GetFloat("KpxOuter").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[1].text = PlayerPrefs.GetFloat("KixOuter").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[2].text = PlayerPrefs.GetFloat("KdxOuter").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[3].text = PlayerPrefs.GetFloat("KpyOuter").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[4].text = PlayerPrefs.GetFloat("KiyOuter").ToString();
            pnlOuterLoop.GetComponentsInChildren<InputField>()[5].text = PlayerPrefs.GetFloat("KdyOuter").ToString();
            updatePID("a");
        }


    }

    private void FixedUpdate()
    {
        if (client.intArr != null)
        {
            if (client.intArr[0] == -1 || client.intArr[1] == -1)
            {
                GameObject.Find("Sphere").GetComponent<Rigidbody>().position =
                new Vector3(0, -420, 0);
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().enabled = false;
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().Clear();
            }
            else
            {
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().enabled = true;
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().time = 10;
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().startWidth = 3;
                GameObject.Find("Sphere").GetComponent<TrailRenderer>().endWidth = 1;
                GameObject.Find("Sphere").GetComponent<Rigidbody>().position =
                new Vector3(client.intArr[0] - BALL_OFFSET, -420, client.intArr[1] - BALL_OFFSET + Z_OFFSET);
            }
        }
        
    }

    private void Update()
    {
        if (logined)
        {
            if (cameraDesiredLookAt != null)
            {
                cameraTransform.rotation = Quaternion.Slerp
                    (cameraTransform.rotation, cameraDesiredLookAt.rotation, CAMERA_TRANSITION_SPEED * Time.deltaTime);
            }

            Transform pin = GameObject.Find("pin").GetComponent<Transform>().transform;
            Transform pin1 = GameObject.Find("pin1").GetComponent<Transform>().transform;
            Transform pin2 = GameObject.Find("pin2").GetComponent<Transform>().transform;
            Transform pin3 = GameObject.Find("pin3").GetComponent<Transform>().transform;
            Transform pin4 = GameObject.Find("pin4").GetComponent<Transform>().transform;

            if (GameObject.Find("tglGroup").GetComponent<ToggleGroup>().GetComponentsInChildren<Toggle>()[0].isOn) // setpoint
            {
                if (Input.GetMouseButtonDown(0))
                {
                    Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
                    RaycastHit hit;
                    if (Physics.Raycast(ray, out hit))
                    {
                        if (hit.transform.name == "Plane")
                            if (Mathf.Abs(hit.point.x) < ACTIVE_REGION)
                                if (Mathf.Abs(hit.point.z - Z_OFFSET) < ACTIVE_REGION)
                                {
                                    GameObject.Find("pin").GetComponent<Transform>().transform.position = hit.point + pinOffset;
                                    bpsSocketSendData sendData = new bpsSocketSendData((Int16)bpsCommandTypeDef.BPS_MODE_SETPOINT,
                                            (Int16)(hit.point.x + BALL_OFFSET), (Int16)(hit.point.z - Z_OFFSET + BALL_OFFSET));
                                    client.SendAsync(sendData.data, sendData.data.Length);
                                }
                    }
                }
                Vector3 newPinDragPosition = GameObject.Find("pin").GetComponent<bpsPinMove>().position;
                if (newPinDragPosition != pinDragPosition || mode == 1)
                {
                    pinDragPosition = newPinDragPosition;
                    bpsSocketSendData sendData = new bpsSocketSendData((Int16)bpsCommandTypeDef.BPS_MODE_SETPOINT,
                                               (Int16)(pinDragPosition.x + BALL_OFFSET), (Int16)(pinDragPosition.z - Z_OFFSET + BALL_OFFSET));
                    client.SendAsync(sendData.data, sendData.data.Length);
                    mode = 0;
                }
                PointWithinActiveregion(pin, 0);
            }
            else if (GameObject.Find("tglGroup").GetComponent<ToggleGroup>().GetComponentsInChildren<Toggle>()[1].isOn) // circle
            {
                if (Input.GetMouseButtonDown(0))
                {
                    Ray ray = Camera.main.ScreenPointToRay(Input.
                        mousePosition);
                    RaycastHit hit;
                    if (Physics.Raycast(ray, out hit))
                    {
                        if (hit.transform.name == "Plane")
                            radius = Vector3.Distance(hit.point, pin.position);
                    }
                }
                PointWithinActiveregion(pin, radius);

                if (pin.position.x < pin.position.z - Z_OFFSET)
                    if (Mathf.Abs(pin.position.x) + radius > ACTIVE_REGION)
                        radius = ACTIVE_REGION - Mathf.Abs(pin.position.x);
                    else
                    if (Mathf.Abs(pin.position.z - Z_OFFSET) + radius > ACTIVE_REGION)
                        radius = ACTIVE_REGION - Mathf.Abs(pin.position.z - Z_OFFSET);
                if (radius < RADIUS_MIN)
                    radius = RADIUS_MIN;
                GameObject.Find("circle").GetComponent<bpsCircle>().SetupCirlce(radius, pin.position);

                Vector3 newPinDragPosition = GameObject.Find("pin").GetComponent<bpsPinMove>().position;

                if (newPinDragPosition != pinDragPosition || oldRadius != radius || mode == 2)
                {
                    pinDragPosition = newPinDragPosition;
                    bpsSocketSendData sendData = new bpsSocketSendData((Int16)bpsCommandTypeDef.BPS_MODE_CIRCLE,
                                               (Int16)(pinDragPosition.x + BALL_OFFSET), (Int16)(pinDragPosition.z - Z_OFFSET + BALL_OFFSET), (UInt16)radius, (UInt16)1);
                    client.SendAsync(sendData.data, sendData.data.Length);
                    oldRadius = radius;
                    mode = 0;
                }

            }
            else // rectangle
            {
                if (DistanceFromPointToLine(pin1.position, pin2.position, pin3.position) < DISTANCE_MIN)
                    pin1.position = pin1OldPosition;
                else if (pin1.position != pin1OldPosition)
                {
                    pin1OldPosition = pin1.position;
                    isDataToSend = true;
                }
                PointWithinActiveregion(pin1, 0);

                if (DistanceFromPointToLine(pin2.position, pin1.position, pin4.position) > -DISTANCE_MIN)
                    pin2.position = pin2OldPosition;
                else if (pin2.position != pin2OldPosition)
                {
                    pin2OldPosition = pin2.position;
                    isDataToSend |= true;
                }
                PointWithinActiveregion(pin2, 0);

                if (DistanceFromPointToLine(pin3.position, pin1.position, pin4.position) < DISTANCE_MIN)
                    pin3.position = pin3OldPosition;
                else if (pin3.position != pin3OldPosition)
                {
                    pin3OldPosition = pin3.position;
                    isDataToSend |= true;
                }
                PointWithinActiveregion(pin3, 0);

                if (DistanceFromPointToLine(pin4.position, pin2.position, pin3.position) > -DISTANCE_MIN)
                    pin4.position = pin4OldPosition;
                else if (pin4.position != pin4OldPosition)
                {
                    pin4OldPosition = pin4.position;
                    isDataToSend |= true;
                }
                PointWithinActiveregion(pin4, 0);

                if (isDataToSend)
                {
                    bpsSocketSendData sendData = new bpsSocketSendData((Int16)bpsCommandTypeDef.BPS_MODE_RECTANGLE, 
                        (Int16)(pin1.position.x + BALL_OFFSET), (Int16)(pin1.position.z - Z_OFFSET + BALL_OFFSET), 
                        (Int16)(pin2.position.x + BALL_OFFSET), (Int16)(pin2.position.z - Z_OFFSET + BALL_OFFSET), 
                        (Int16)(pin3.position.x + BALL_OFFSET), (Int16)(pin3.position.z - Z_OFFSET + BALL_OFFSET), 
                        (Int16)(pin4.position.x + BALL_OFFSET), (Int16)(pin4.position.z - Z_OFFSET + BALL_OFFSET));
                    client.SendAsync(sendData.data, sendData.data.Length);
                    isDataToSend = false;
                }

            }
        }
    }
 
    private void OnDestroy()
    {
        client.Close();
    }

    public void Login()
    {
        client = new Client();
        client.ConnectToServer();
        if (client.Login())
        {
            client.RecvAsync();
            cameraDesiredLookAt = GameObject.Find("btnOuterLoop").GetComponent<Button>().transform;
            logined = true;
        }
        else
            GameObject.Find("txtLoginFail").GetComponent<Text>().enabled = true;
    }

    private void selectSetpoint(bool a)
    {
        GameObject.Find("pin").GetComponent<Renderer>().enabled = true;
        GameObject.Find("pin1").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin2").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin3").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin4").GetComponent<Renderer>().enabled = false;
        GameObject.Find("circle").GetComponent<LineRenderer>().enabled = false;
        mode = 1;
    }

    private void selectCirlce(bool a)
    {
        GameObject.Find("pin").GetComponent<Renderer>().enabled = true;
        GameObject.Find("pin1").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin2").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin3").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin4").GetComponent<Renderer>().enabled = false;
        GameObject.Find("circle").GetComponent<LineRenderer>().enabled = true;
        mode = 2;
    }

    private void selectRectangle(bool a)
    {
        GameObject.Find("pin").GetComponent<Renderer>().enabled = false;
        GameObject.Find("pin1").GetComponent<Renderer>().enabled = true;
        GameObject.Find("pin2").GetComponent<Renderer>().enabled = true;
        GameObject.Find("pin3").GetComponent<Renderer>().enabled = true;
        GameObject.Find("pin4").GetComponent<Renderer>().enabled = true;
        GameObject.Find("circle").GetComponent<LineRenderer>().enabled = false;
        mode = 3;
    }

    private float DistanceFromPointToLine(Vector3 P, Vector3 A, Vector3 B)
    {
        Vector3 crossProduct = Vector3.Cross((P - A), (P - B));
        return crossProduct.y > 0 ? (Vector3.Magnitude(crossProduct) / Vector3.Magnitude(A - B))
                                    : (Vector3.Magnitude(crossProduct) / Vector3.Magnitude(A - B)) * -1;
    }

    private void PointWithinActiveregion(Transform pin, float radius)
    {
        if (Mathf.Abs(pin.position.x) + radius > ACTIVE_REGION)
            pin.position = new Vector3((pin.position.x > 0 ? ACTIVE_REGION - radius : -ACTIVE_REGION + radius),
                                                                            pin.position.y, pin.position.z);
        if (Mathf.Abs(pin.position.z - Z_OFFSET) + radius > ACTIVE_REGION)
            pin.position = new Vector3(pin.position.x, pin.position.y, (pin.position.z > Z_OFFSET ? ACTIVE_REGION - radius :
                                                                        -ACTIVE_REGION + radius) + Z_OFFSET);

    }

    private void tglPnlOuterLoop()
    {
        pnlOuterLoop.SetActive(!pnlOuterLoop.activeSelf);
        if (pnlOuterLoop.activeSelf == true)
        {
            pnlInnerLoop.SetActive(false);
            logined = false;
        }
        else
            logined = true;
    }

    private void tglPnlInnerLoop()
    {
        pnlInnerLoop.SetActive(!pnlInnerLoop.activeSelf);
        if (pnlInnerLoop.activeSelf == true)
        { 
            pnlOuterLoop.SetActive(false);
            logined = false;
        }
        else
            logined = true;
    }

    private void updatePID(string a)
    {
        Debug.Log(a);
        float[] farr = new float[12];
        for (int i = 0; i < 6; i++)
            float.TryParse(pnlOuterLoop.GetComponentsInChildren<InputField>()[i].text, out farr[i]);
        for (int i = 6; i < 12; i++)
            float.TryParse(pnlInnerLoop.GetComponentsInChildren<InputField>()[i - 6].text, out farr[i]);

        bpsSocketSendData sendData = new bpsSocketSendData((Int16)bpsCommandTypeDef.BPS_UPDATE_PID,
                                    farr[0], farr[3], farr[6], farr[9], farr[1], farr[4], 
                                    farr[7], farr[10], farr[2], farr[5], farr[8], farr[11]);
        client.SendAsync(sendData.data, sendData.data.Length);
        PlayerPrefs.SetFloat("KpxInner", farr[6]);
        PlayerPrefs.SetFloat("KixInner", farr[7]);
        PlayerPrefs.SetFloat("KdxInner", farr[8]);
        PlayerPrefs.SetFloat("KpyInner", farr[9]);
        PlayerPrefs.SetFloat("KiyInner", farr[10]);
        PlayerPrefs.SetFloat("KdyInner", farr[11]);
        PlayerPrefs.SetFloat("KpxOuter", farr[0]);
        PlayerPrefs.SetFloat("KixOuter", farr[1]);
        PlayerPrefs.SetFloat("KdxOuter", farr[2]);
        PlayerPrefs.SetFloat("KpyOuter", farr[3]);
        PlayerPrefs.SetFloat("KiyOuter", farr[4]);
        PlayerPrefs.SetFloat("KdyOuter", farr[5]);

    }
}

public class Client 
{
    private bool socketReady = false;
    private Socket client;
    private NetworkStream stream;
    private StreamReader reader;
    private StreamWriter writer;
    private static byte[] recvData = new byte[8];
    private static int size = 8;
    public Int16[] intArr;
    public void ConnectToServer()
    {
        if (socketReady)
            return;
        try
        {
            IPEndPoint iep = new IPEndPoint(IPAddress.Parse(GameObject.Find("infIP").GetComponent<InputField>().text), 
                                        Int32.Parse(GameObject.Find("infPort").GetComponent<InputField>().text));
            client = new Socket(AddressFamily.InterNetwork, SocketType.Stream,
            ProtocolType.Tcp);
            client.Connect(iep);
            stream = new NetworkStream(client);
            reader = new StreamReader(stream);
            writer = new StreamWriter(stream);
            socketReady = true;
            intArr = new Int16[2];

        }
        catch (Exception e)
        {
            Debug.Log("socket error: " + e.Message);
        }
    }

    public bool Login()
    {
        if (!socketReady)
            return false;

        Byte[] data = Encoding.ASCII.GetBytes("LOGIN:" + GameObject.Find("infUser").GetComponent<InputField>().text + ":"
                        + GameObject.Find("infPass").GetComponent<InputField>().text + ":");
        stream.Write(data, 0, data.Length);

        data = new Byte[8];
        int len = stream.Read(data, 0, data.Length);
        string receivedData = Encoding.ASCII.GetString(data, 0, len);
        if (receivedData != null)
            if (receivedData == "SUCCEED:")
                return true;
        client.Close();
        socketReady = false;
        return false;
    }

    public void RecvAsync()
    {
        client.BeginReceive(recvData, 0, size, SocketFlags.None, new AsyncCallback(ReceiveData), client);
    }

    public void SendAsync(byte[] data, int size)
    {
        if (data != null && size != 0 && socketReady)
            client.BeginSend(data, 0, size, SocketFlags.None, new AsyncCallback(SendData), client);
    }

    public void Close()
    {
        //client.Shutdown(SocketShutdown.Both);
        stream.Flush();
        stream.Close();
        stream.Dispose();
        client.Close();
        socketReady = false;

    }
    private void ReceiveData(IAsyncResult iar)
    {
        Socket client = (Socket)iar.AsyncState;
        int len = client.EndReceive(iar);
        if (len == 0)
        {
            client.Close();
            return;
        }
        intArr = Copy_Byte_Buffer_To_Int16_Buffer(recvData);
        for (int i = 0; i < len / 2; i++)
            Debug.Log(i + " := " + intArr[i]);
        client.BeginReceive(recvData, 0, size, SocketFlags.None, new AsyncCallback(ReceiveData), client);
    }


    private void SendData(IAsyncResult iar)
    {
        try
        {
            Socket client = (Socket)iar.AsyncState;
            int send = client.EndSend(iar);
        }
        catch
        {

        }
    }

    private static  Int16[] Copy_Byte_Buffer_To_Int16_Buffer(byte[] buffer)

    {
        Int16[] result = new Int16[1];
        int size = buffer.Length;
        if ((size % 2) != 0)
        {
            /* Error here */
            return result;
        }
        else
        {
            result = new Int16[size / 2];
            IntPtr ptr_src = Marshal.AllocHGlobal(size);
            Marshal.Copy(buffer, 0, ptr_src, size);
            Marshal.Copy(ptr_src, result, 0, result.Length);
            Marshal.FreeHGlobal(ptr_src);
            return result;
        }
    }

    public T FromByteArray<T>(byte[] data)
    {
        if (data == null)
            return default(T);
        BinaryFormatter bf = new BinaryFormatter();
        using (MemoryStream ms = new MemoryStream(data))
        {
            object obj = bf.Deserialize(ms);
            return (T)obj;
        }
    }

    public byte[] ToByteArray<T>(T obj)
    {
        if (obj == null)
            return null;
        BinaryFormatter bf = new BinaryFormatter();
        using (MemoryStream ms = new MemoryStream())
        {
            bf.Serialize(ms, obj);
            return ms.ToArray();
        }
    } 
}