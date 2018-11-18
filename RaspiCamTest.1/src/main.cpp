#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

using namespace cv;
using namespace std;
 
float Brightness;
float Contrast ;
float Saturation;
float Gain;
float Thres;
int B;
int C;
int S;
int G;
int T;
char winName[20]="Live";
Mat frame, gray, thresh;
VideoCapture cap(0);

void onTrackbar_changed(int, void*)
{
 Brightness =float(B)/100;
 Contrast   =float(C)/100;
 Saturation =float(S)/100;
 Gain       =float(G)/100;
Thres       =float(T)/255;
cap.set(CAP_PROP_BRIGHTNESS,Brightness);
cap.set(CAP_PROP_CONTRAST, Contrast);
cap.set(CAP_PROP_SATURATION, Saturation);
cap.set(CAP_PROP_GAIN, Gain);

}

int main(int, char**)
{

    if(!cap.isOpened())  // check if we succeeded
        return -1;

    cout<<"Press 's' to save snapshot"<<endl;
 namedWindow(winName);

 Brightness = cap.get(CAP_PROP_BRIGHTNESS);
 Contrast   = cap.get(CAP_PROP_CONTRAST );
 Saturation = cap.get(CAP_PROP_SATURATION);
 Gain       = cap.get(CAP_PROP_GAIN);

 cout<<"===================================="<<endl<<endl;
 cout<<"Default Brightness -------> "<<Brightness<<endl;
 cout<<"Default Contrast----------> "<<Contrast<<endl;
 cout<<"Default Saturation--------> "<<Saturation<<endl;
 cout<<"Default Gain--------------> "<<Gain<<endl<<endl;
 cout<<"===================================="<<endl;

  B=int(Brightness*100);
  C=int(Contrast*100);
  S=int(Saturation*100);
  G=int(Gain*100);
  T=int(25*255);  
createTrackbar( "Brightness",winName, &B, 100, onTrackbar_changed );
createTrackbar( "Contrast",winName, &C, 100,onTrackbar_changed );
createTrackbar( "Saturation",winName, &S, 100,onTrackbar_changed);
createTrackbar( "Gain",winName, &G, 100,onTrackbar_changed);
createTrackbar( "Thres",winName, &T, 255,onTrackbar_changed);
    int i=0;
    char name[10];
    for(;;)
    {

        cap >> frame; // get a new frame from camera
		cvtColor(frame,gray,COLOR_BGR2HLS);
        //medianBlur(gray, gray, 5);
        //std::cout << T <<std::endl;
        threshold(gray,thresh,T,255,0);
        imshow(winName, frame);
        imshow("gray", gray);
        imshow("thresh", thresh);
        char c=waitKey(30);

        if(c=='s') {
     sprintf(name,"%d.jpg",i++);
     imwrite(name,frame);
    }
        if( c== 27) break;
    }
return 0;
}