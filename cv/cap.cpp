
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;

int
main()
{
        /*
         * Note: opencv doesn't have an API to list camera devices.
         * https://github.com/opencv/opencv/issues/4269
         * While ID 0 seems to work for many cases, it doesn't always work.
         * For example, while I can successfully open ID 0 on my macbook,
         * it returns an empty frame for some reasons. The front camera
         * on the macbook seems working if I specify ID 1.
         */
        const char *camera_id_str = getenv("CAMERA_ID");
        unsigned int camera_id = 0;
        if (camera_id_str != NULL) {
                camera_id = atoi(camera_id_str);
        }
        VideoCapture cap(camera_id);
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
                // imshow("live", frame);
                // snapshot = frame.clone();
                resize(frame, sframe, Size(100, 100), -1, -1);
                imshow("small", sframe);
#if 0
                Mat gray;
                cvtColor(sframe, gray, COLOR_BGR2GRAY, 0);
                // cvtColor(sframe, gray, COLOR_BGR2RGB, 0);
                imshow("gray", gray);
#endif
                waitKey(5);
        }
}
