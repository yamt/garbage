
#include <stdio.h>

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

int
main()
{
    VideoCapture cap(0);
#if 0
    cap.set(CAP_PROP_FRAME_WIDTH, 200);
    cap.set(CAP_PROP_FRAME_HEIGHT, 200);
#endif
    if (!cap.isOpened()) {
        fprintf(stderr, "failed to open camera");
        exit(1);
    }
    Mat frame;
    while (1) {
        cap.read(frame);
        if (frame.empty()) {
            fprintf(stderr, "empty frame");
            break;
        }
        Mat sframe;
        //imshow("live", frame);
        //snapshot = frame.clone();
        resize(frame, sframe, Size(100, 100), -1, -1);
        imshow("small", sframe);
#if 0
        Mat gray;
        cvtColor(sframe, gray, COLOR_BGR2GRAY, 0);
        //cvtColor(sframe, gray, COLOR_BGR2RGB, 0);
        imshow("gray", gray);
#endif
        waitKey(5);
    }
}
