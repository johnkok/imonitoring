#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ioko.hpp"

using namespace cv;
using namespace std;

int pd_init = 0;
HOGDescriptor hog;
int pd_alarm = 0;

void peopleDetectInit(void){
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
}

int peopleDetectCam(int camID){
Mat img;
FILE* f = 0;
int ret = 0;
char path[256];
size_t i, j;
vector<Rect> found, found_filtered;

	sprintf(path, "%s%s",IMAGES_PATH, "cam_0.jpg");
	img = imread(path);

        if(!img.data)
            return ret;

        hog.detectMultiScale(img, found, 0.3, Size(8,8), Size(32,32), 1.0, 1.0);
        
	ret = found.size();
	pd_alarm = ret;

	for( i = 0; i < found.size(); i++ ){
            Rect r = found[i];

            for( j = 0; j < found.size(); j++ )
                if( j != i && (r & found[j]) == r)
                    break;
            if( j == found.size() )
                found_filtered.push_back(r);
        }

//        for( i = 0; i < found_filtered.size(); i++ ){	    
//            Rect r = found_filtered[i];

//            r.x += cvRound(r.width*0.1);
//            r.width = cvRound(r.width*0.8);
//            r.y += cvRound(r.height*0.07);
//            r.height = cvRound(r.height*0.8);
//            rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
//        }

    return ret;
}
