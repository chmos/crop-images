//
//  cropper.h
//  SlamView
//
//  Created by Mosong Cheng on 4/30/14.
//  Copyright (c) 2014 Mosong Cheng. All rights reserved.
//

#ifndef __SlamView__cropper__
#define __SlamView__cropper__

#include <iostream>
#include "opencv2/opencv.hpp"
#include "cv.hpp"
#include "highgui.h"

using namespace std;
using namespace cv;

    
    class Cropper{
    public:
        int kstep = 10;
        
    public:
        /** cropping the image to rid the blank part **/
        Mat crop(const Mat& img);
        
        void crop(const Mat& src, Mat& dst, cv::Rect& dstBound);
        
        
        inline void setStep(int kp){
            kstep = kp;
        }
        
        void setROI(const cv::Rect& roi){
            finalROI = roi;
            // roi = [4165 x 2207 from (465, 253)]
        }
        
        ~Cropper(){
            mask.release();
        }
        
    private:
        Mat mask;
        cv::Rect rect;
        
        cv::Rect finalROI; // the final cropping from image
        
    private:
        void getMask(const Mat& img);
        void expandROI(cv::Rect& r);
        void framing();
        
        bool lineHasBlankPixels(const Point2i& st, const Point2i& ed);
        
        bool rectHasBlankPixels(const cv::Rect& roi);
        
        bool lineHasContent(const Point2i& st, const Point2i& ed);
        
        void searchContentLine(Point2i& p1, Point2i& p2, Point2i& delta);
    };


