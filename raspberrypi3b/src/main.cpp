#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
 
int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));
 
    // Create a tracker
    string trackerType = trackerTypes[6];
 
    Ptr<Tracker> tracker;
 
    
        if (trackerType == "BOOSTING")
            tracker = TrackerBoosting::create();
        if (trackerType == "MIL")
            tracker = TrackerMIL::create();
        if (trackerType == "KCF")
            tracker = TrackerKCF::create();
        if (trackerType == "TLD")
            tracker = TrackerTLD::create();
        if (trackerType == "MEDIANFLOW")
            tracker = TrackerMedianFlow::create();
        if (trackerType == "GOTURN")
            tracker = TrackerGOTURN::create();
        if (trackerType == "MOSSE")
            tracker = TrackerMOSSE::create();
        if (trackerType == "CSRT")
            tracker = TrackerCSRT::create();
   

    // Read video
    VideoCapture video(0);
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
        return 1; 
    } 
    // Read frame continiously
    UMat frame; 

        bool ok = video.read(frame); 


   std::cout << "Number of threads = " 
              <<  std::thread::hardware_concurrency() << std::endl;
    // Read image
    UMat im;
    cvtColor(frame, im, COLOR_BGR2GRAY);
 
// Set up the detector with default parameters.
    //SimpleBlobDetector detector;
 
// Detect blobs.
    //std::vector<KeyPoint> keypoints;
    //detector.detect( im, keypoints);
 
// Draw detected blobs as red circles.
// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
    //UMat im_with_keypoints;
    //drawKeypoints( im, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
 
// Show blobs
UMat canny_output;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
 
// detect edges using canny
Canny( im, canny_output, 50, 150, 3 );
 
// find contours
findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
 
// get the moments
vector<Moments> mu(contours.size());
for( int i = 0; i<contours.size(); i++ )
{ mu[i] = moments( contours[i], false ); }
 
// get the centroid of figures.
vector<Point2f> mc(contours.size());
for( int i = 0; i<contours.size(); i++)
{ mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }
 
 
// draw contours
// Mat drawing(canny_output.size(), CV_8UC3, Scalar(255,255,255));
// for( int i = 0; i<contours.size(); i++ )
// {
// Scalar color = Scalar(167,151,0); // B G R values
// drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
// circle( drawing, mc[i], 4, color, -1, 8, 0 );
// }
 
// // show the resultant image
// namedWindow( "Contours", WINDOW_AUTOSIZE );
// imshow( "Contours", drawing );
// waitKey(0);
//     imshow("keypoints", im );
//     waitKey(1);
    // Define initial bounding box 
    //Rect2d bbox(287, 283, 86, 320); 
    Rect2d bbox(mc[0].x, mc[0].y, 100, 100);
    // Uncomment the line below to select a different bounding box 
    // bbox = selectROI(frame, false); 
    // Display bounding box. 
    rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
    //circle(frame, Point(250,250),80, Scalar( 255, 0, 0 ), 2, 1 ); 
    imshow("Tracking", frame); 
    tracker->init(frame, bbox);
     
    while(video.read(frame))
    {     
        // Start timer
        double timer = (double)getTickCount();
         
        // Update the tracking result
        bool ok = tracker->update(frame, bbox);
         
        // Calculate Frames per second (FPS)
        float fps = getTickFrequency() / ((double)getTickCount() - timer);
         
        if (ok)
        {
            // Tracking success : Draw the tracked object
            rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
            //circle(frame, Point(250,250),80, Scalar( 255, 0, 0 ), 2, 1 );
        }
        else
        {
            // Tracking failure detected.
            putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
        }
         
        // Display tracker type on frame
        putText(frame, trackerType + " Tracker", Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);
         
        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
 
        // Display frame.
        imshow("Tracking", frame);
         
        // Exit if ESC pressed.
        int k = waitKey(1);
        if(k == 27)
        {
            break;
        }
 
    }
}

void thread1(void)
{
    VideoCapture video(0);
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
    } 
    // Read frame continiously
    UMat frame; 
    while (1)
    {
        bool ok = video.read(frame); 


    }
}

void thread2(void)
{
    
}
void thread3(void)
{

}

void thread4(void)
{
    
}