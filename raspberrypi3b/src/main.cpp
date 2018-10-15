#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     480
#define FRAME_HEIGHT    480
#define H_MIN           17
#define H_MAX           32
#define S_MIN           52
#define S_MAX           256
#define V_MIN           43
#define V_MAX           256
#define MAX_NUM_OBJECTS 50
#define MIN_OBJECT_AREA 20*20
#define MAX_OBJECT_AREA FRAME_HEIGHT*FRAME_WIDTH/1.5

sem_t semCaptureFrameCplt, semProcessFrameCplt;
bool bFoundObject = false;
UMat frame, HSV, thresh, contour;
Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
//dilate with larger element so make sure object is nicely visible
Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
int x,y;
void captureFrame(void);
void processFrame(void);
void trackingObject(void);
void drawObject(int x, int y,UMat &frame);
string intToString(int number);
// int main(int argc, char **argv)
// {
//     // List of tracker types in OpenCV 3.4.1
//     string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
//     // vector <string> trackerTypes(types, std::end(types));
 
//     // Create a tracker
//     string trackerType = trackerTypes[6];
 
//     Ptr<Tracker> tracker;
 
    
//         if (trackerType == "BOOSTING")
//             tracker = TrackerBoosting::create();
//         if (trackerType == "MIL")
//             tracker = TrackerMIL::create();
//         if (trackerType == "KCF")
//             tracker = TrackerKCF::create();
//         if (trackerType == "TLD")
//             tracker = TrackerTLD::create();
//         if (trackerType == "MEDIANFLOW")
//             tracker = TrackerMedianFlow::create();
//         if (trackerType == "GOTURN")
//             tracker = TrackerGOTURN::create();
//         if (trackerType == "MOSSE")
//             tracker = TrackerMOSSE::create();
//         if (trackerType == "CSRT")
//             tracker = TrackerCSRT::create();
   

//     // Read video
//     VideoCapture video(0);
//     // Exit if video is not opened
//     if(!video.isOpened())
//     {
//         cout << "Could not read video file" << endl; 
//         return 1; 
//     } 
//     // Read frame continiously
//     UMat frame; 

//         bool ok = video.read(frame); 


//    std::cout << "Number of threads = " 
//               <<  std::thread::hardware_concurrency() << std::endl;
//     // Read image
//     UMat im;
//     cvtColor(frame, im, COLOR_BGR2GRAY);
 
// // Set up the detector with default parameters.
//     //SimpleBlobDetector detector;
 
// // Detect blobs.
//     //std::vector<KeyPoint> keypoints;
//     //detector.detect( im, keypoints);
 
// // Draw detected blobs as red circles.
// // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
//     //UMat im_with_keypoints;
//     //drawKeypoints( im, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
 
// // Show blobs
// UMat canny_output;
// vector<vector<Point> > contours;
// vector<Vec4i> hierarchy;
 
// // detect edges using canny
// Canny( im, canny_output, 50, 150, 3 );
 
// // find contours
// findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
 
// // get the moments
// vector<Moments> mu(contours.size());
// for( int i = 0; i<contours.size(); i++ )
// { mu[i] = moments( contours[i], false ); }
 
// // get the centroid of figures.
// vector<Point2f> mc(contours.size());
// for( int i = 0; i<contours.size(); i++)
// { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }
 
 
// // draw contours
// // Mat drawing(canny_output.size(), CV_8UC3, Scalar(255,255,255));
// // for( int i = 0; i<contours.size(); i++ )
// // {
// // Scalar color = Scalar(167,151,0); // B G R values
// // drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
// // circle( drawing, mc[i], 4, color, -1, 8, 0 );
// // }
 
// // // show the resultant image
// // namedWindow( "Contours", WINDOW_AUTOSIZE );
// // imshow( "Contours", drawing );
// // waitKey(0);
// //     imshow("keypoints", im );
// //     waitKey(1);
//     // Define initial bounding box 
//     //Rect2d bbox(287, 283, 86, 320); 
//     Rect2d bbox(mc[0].x, mc[0].y, 100, 100);
//     // Uncomment the line below to select a different bounding box 
//     // bbox = selectROI(frame, false); 
//     // Display bounding box. 
//     rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
//     //circle(frame, Point(250,250),80, Scalar( 255, 0, 0 ), 2, 1 ); 
//     imshow("Tracking", frame); 
//     tracker->init(frame, bbox);
     
//     while(video.read(frame))
//     {     
//         // Start timer
//         double timer = (double)getTickCount();
         
//         // Update the tracking result
//         bool ok = tracker->update(frame, bbox);
         
//         // Calculate Frames per second (FPS)
//         float fps = getTickFrequency() / ((double)getTickCount() - timer);
         
//         if (ok)
//         {
//             // Tracking success : Draw the tracked object
//             rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
//             //circle(frame, Point(250,250),80, Scalar( 255, 0, 0 ), 2, 1 );
//         }
//         else
//         {
//             // Tracking failure detected.
//             putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
//         }
         
//         // Display tracker type on frame
//         putText(frame, trackerType + " Tracker", Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);
         
//         // Display FPS on frame
//         putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
 
//         // Display frame.
//         imshow("Tracking", frame);
         
//         // Exit if ESC pressed.
//         int k = waitKey(1);
//         if(k == 27)
//         {
//             break;
//         }
 
//     }
// }

int main()
{
    sem_init(&semCaptureFrameCplt, 0, 0);
    sem_init(&semProcessFrameCplt, 0, 0);
    std::thread thread1(captureFrame);
    std::thread thread2(processFrame);
    std::thread thread3(trackingObject);
    thread1.join();
    thread2.join();
    thread3.join();
    return 0;
}
void captureFrame(void)
{
    VideoCapture video(0);
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
    } 
    // Read frame continiously
    video.set(CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	video.set(CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    sleep(1);
    while (1)
    {
        bool ok = video.read(frame); 
        if (ok)
        {
            std::cout << "thread 1\n";       
            if (bFoundObject)
            {
                sem_post(&semProcessFrameCplt);
            }
            else
            {
                sem_post(&semCaptureFrameCplt);
            }
        }
    }
}

void processFrame(void)
{
    while(1)
    {     
        sem_wait(&semCaptureFrameCplt);     
        std::cout << "thread 2\n";
        //process frame to find object;
        //convert frame from BGR to HSV colorspace
        cvtColor(frame,HSV,COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),thresh);
        
        erode(thresh,thresh,erodeElement);
        erode(thresh,thresh,erodeElement);

        dilate(thresh,thresh,dilateElement);
        dilate(thresh,thresh,dilateElement);

        //these two vectors needed for output of findContours
        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;
        //find contours of filtered image using openCV findContours function
        findContours(thresh,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
        
        //use moments method to find our filtered object
	    double refArea = 0;
	    bool objectFound = false;
        if (hierarchy.size() > 0) 
        {
            int numObjects = hierarchy.size();
            //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
            if(numObjects<MAX_NUM_OBJECTS)
            {
                for (int index = 0; index >= 0; index = hierarchy[index][0]) 
                {
                    Moments moment = moments((cv::Mat)contours[index]);
                    double area = moment.m00;
                    //if the area is less than 20 px by 20px then it is probably just noise
                    //if the area is the same as the 3/2 of the image size, probably just a bad filter
                    //we only want the object with the largest area so we safe a reference area each
                    //iteration and compare it to the area in the next iteration.
                    if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea)
                    {
                        x = moment.m10/area;
                        y = moment.m01/area;
                        bFoundObject = true;
                        refArea = area;
                    }
                    else 
                        bFoundObject = false;


                }
                //let user know you found an object
                // if(bFoundObject ==true)
                // {
                //     putText(frame,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
                //     //draw object location on screen
                //     drawObject(x,y,frame);
                // }

            }
            else 
            putText(frame,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
        }
        sem_post(&semProcessFrameCplt);
    }
}
void trackingObject(void)
{
    while(1)
    {
        sem_wait(&semProcessFrameCplt);
        
        waitKey(1);
        if (bFoundObject)
        {
            // tracking object
            Ptr<Tracker> tracker;
            tracker = TrackerMOSSE::create();
            Rect2d bbox(x, y, 150, 150);

            bool ok = tracker->update(frame, bbox);
            if (ok)
            {
                rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
                std::cout << "Tracking \n";
            }
            else
            {
                // Tracking failure detected.
                putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
                bFoundObject = false;
            }

        }
        imshow("asdasd", frame);
        std::cout << "thread 3\n";
    }
}

void thread4(void)
{
    
}

void drawObject(int x, int y,UMat &frame)
{

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

    //UPDATE:JUNE 18TH, 2013
    //added 'if' and 'else' statements to prevent
    //memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
    line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
    line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
    line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

}

string intToString(int number)
{


	std::stringstream ss;
	ss << number;
	return ss.str();
}