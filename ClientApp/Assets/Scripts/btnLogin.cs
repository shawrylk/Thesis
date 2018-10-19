using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using System;
using System.IO;
using System.Net;
using System.Text;
using System.Net.Sockets;

public class btnLogin : MonoBehaviour {

    public InputField userName;
    public InputField paswword;
    public Text loginFails;
    private string scene = "BallnPlateScene";
    private byte[] data = new byte[1024];
    private int size = 1024;
    private bool logined = false;
    private bool btnLoginPressed = false;
    private bool receiveData = false;
    private void Start()
    {
        loginFails.enabled = false;
    }

    private void Update()
    {
        if (receiveData == true)
        {
            if (logined == false)
            {
                if (btnLoginPressed == true)
                    loginFails.enabled = true;
            }
            else
            {
                SceneManager.LoadScene(scene);
            }
        }
    }

    public void LoadLevel()
    {
        try
        {
            IPEndPoint iep = new IPEndPoint(IPAddress.Parse("192.168.1.116"), 22396);
            Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream,
            ProtocolType.Tcp);
            client.Connect(iep);
            NetworkStream ns = new NetworkStream(client);
            StreamWriter sw = new StreamWriter(ns);
            sw.WriteLine("LOGIN:" + userName.text + ":" + paswword.text);
            sw.Flush();
            client.BeginReceive(data, 0, size, SocketFlags.None, new AsyncCallback(ReceiveData), client);
            btnLoginPressed = true;

        }
        catch
        {
            Debug.Log("server fails");
        }

    }

    private void ReceiveData(IAsyncResult iar)
    {
        try
        {
            receiveData = true;
            Socket client = (Socket)iar.AsyncState;
            int recv = client.EndReceive(iar);
            if (recv == 0)
            {
                client.Close();
                return;
            }
            string receivedData = Encoding.ASCII.GetString(data, 0, recv);
            receivedData = receivedData.Substring(0, receivedData.Length);
            String[] mang = receivedData.Split(':');
            switch (mang[0])
            {
                case "SUCCEED":
                    logined = true;
                    break;
                case "FAIL":
                   
                    break;

                default:
                    Debug.Log(mang[0]);
                    break;

            }
        }
        catch
        {

        }

    }
    private void SendData(IAsyncResult iar)
    {
        try
        {
            Socket client = (Socket)iar.AsyncState;
            int send = client.EndSend(iar);
            client.BeginReceive(data, 0, size, SocketFlags.None, new AsyncCallback(ReceiveData), client);
        }
        catch
        {

        }

    }

}
