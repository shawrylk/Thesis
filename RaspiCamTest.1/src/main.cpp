#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

using namespace cv;
using namespace std;
 
int B;
int C;
int S;
int G;
int T;
char winName[20]="Live";
Mat frame, gray, thresh1, thresh2;
VideoCapture cap(0);

void onTrackbar_changed(int, void*)
{

cap.set(CAP_PROP_BRIGHTNESS,B);
cap.set(CAP_PROP_CONTRAST, C);
cap.set(CAP_PROP_SATURATION, S);

}

int main(int, char**)
{

    if(!cap.isOpened())  // check if we succeeded
        return -1;

    cout<<"Press 's' to save snapshot"<<endl;
 namedWindow(winName);

  B=70;
  C=100;
  S=100;
  G=150;
  T=110;
createTrackbar( "Brightness",winName, &B, 100, onTrackbar_changed );
createTrackbar( "Contrast",winName, &C, 100,onTrackbar_changed );
createTrackbar( "Saturation",winName, &S, 100,onTrackbar_changed);
createTrackbar( "InThres",winName, &G, 255,onTrackbar_changed);
createTrackbar( "Thres",winName, &T, 255,onTrackbar_changed);
    int i=0;
    char name[10];
    for(;;)
    {

        cap >> frame; // get a new frame from camera
		cvtColor(frame,gray,COLOR_BGR2GRAY);
        //medianBlur(gray, gray, 5);
        //std::cout << T <<std::endl;
        //threshold(gray,thresh1,G,255,1);
        threshold(gray,thresh2,T,255,1);
        imshow(winName, frame);
        //imshow("thresh1", thresh1);
        imshow("thresh2", thresh2);
        char c=waitKey(30);

        if(c=='s') {
     sprintf(name,"%d.jpg",i++);
     imwrite(name,frame);
    }
        if( c== 27) break;
    }
return 0;
}