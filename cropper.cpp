//
//  cropper.cpp
//  SlamView
//
//  Created by Mosong Cheng on 4/30/14.
//  Copyright (c) 2014 Mosong Cheng. All rights reserved.
//

#include "cropper.h"

using namespace std;
using namespace cv;


/** find the max ROI in which all pixels are not blank, and
 ** then crop the image using ROI **/
Mat Cropper::crop(const cv::Mat &img){
    if(img.rows==0 || img.cols==0)return img;
    Mat f;
    crop(img, f, finalROI);
    return f;
}


void Cropper::crop(const cv::Mat &src, cv::Mat &dst, cv::Rect &dstBound)
{
    if (src.rows==0 || src.cols==0){
        dst = src.clone();
        dstBound = cv::Rect(0,0,0,0);
        return;
    }
    
    // -- Step 0: get mask
    getMask(src);
    rect = cv::Rect(0, 0, src.cols, src.rows);
    
    // -- Step 1: rough frame
    framing();
    Mat f = Mat(src, rect).clone();
    finalROI = rect;
    rect = cv::Rect(0, 0, f.cols, f.rows);
    
    //imwrite(SAT::strCombined(debugPath, "zbw20.jpg"), mask*200);
    
    // -- Step 3: start from the center of the image
    cv::Rect roi;
    Point2i pt(rect.width/2, rect.height/2);
    
    roi = cv::Rect(pt.x, pt.y, 1, 1);
    expandROI(roi);
    
    cout<<"roi = "<<roi<<endl;
    f = Mat(f, roi).clone();
    
    finalROI = cv::Rect(finalROI.x + roi.x,
                        finalROI.y + roi.y,
                        roi.width,
                        roi.height);
    
    // -- Step : finally return cropped image
    dst.release();
    dst = f.clone();
    f.release();
    dstBound = finalROI;
}



/** build the mask for cropping **/
void Cropper::getMask(const Mat& img) {
    mask.release();
    mask = Mat::ones(img.rows, img.cols, CV_8U);
    
    uchar *qm = (uchar *)mask.ptr(0,0);
    Vec3b *qi = (Vec3b *)img.ptr(0,0);
    for(int i1=0; i1<img.total(); i1++){
        if(*qi==Vec3b(0,0,0)){
            *qm = 0;
        }
        qm++; qi++;
    }
}


/** roughly get the frame of image and mask **/
void Cropper::framing() {
    Point2i pt1, pt2, delta;
    int x0, x1, y0, y1;
    
    // -- find the upper line
    pt1 = Point2i(0,0); pt2 = Point2i(rect.width-1, 0);
    delta = Point2i(0,1);
    searchContentLine(pt1, pt2, delta);
    y0 = pt1.y;
    
    // -- find the lower line
    pt1 = Point2i(0, rect.height-1); pt2 = Point2i(rect.width-1, rect.height-1);
    delta = Point2i(0,-1);
    searchContentLine(pt1, pt2, delta);
    y1 = pt1.y;
    
    // -- find the left line
    pt1 = Point2i(0,0); pt2 = Point2i(0, rect.height-1 );
    delta = Point2i(1,0);
    searchContentLine(pt1, pt2, delta);
    x0 = pt1.x;
    
    // -- find the right line
    pt1 = Point2i(rect.width-1, 0); pt2 = Point2i(rect.width-1, rect.height-1);
    delta = Point2i(-1,0);
    searchContentLine(pt1, pt2, delta);
    x1 = pt1.x;
    
    // -- get the new rect
    rect = cv::Rect(x0, y0, x1-x0+1, y1-y0+1);
    
    // -- get the new mask
    Mat tmp = Mat(mask, rect).clone();
    mask.release();
    mask = tmp.clone();
    tmp.release();
}


/** expand a roi **/
void Cropper::expandROI(cv::Rect &r) {
    bool canExpand = true;
    Point2i p1, p2;
    cv::Rect rp;
    
    while(canExpand){
        canExpand = false;
        Point2i stp = (rect.tl() - r.tl());
        stp.x /=2; stp.y /=2;
        p2 = r.br();
        while(abs(stp.x) + abs(stp.y)>=1){
            p1 = r.tl() + stp;
            rp = cv::Rect(p1, p2);
            if(!rectHasBlankPixels(rp)){
                r = rp;
                canExpand = true;
                break;
            }
            stp.x /= 2; stp.y /=2;
        }
        
        stp = (rect.br() - r.br());
        stp.x /=2; stp.y /=2;
        p1 = r.tl();
        while(abs(stp.x) + abs(stp.y)>=1){
            p2 = r.br() + stp;
            rp = cv::Rect(p1, p2);
            if(!rectHasBlankPixels(rp)){
                r = rp;
                canExpand = true;
                break;
            }
            stp.x /=2; stp.y/=2;
        }
    }
}



/** determine if a line contains any "blank" piexls.
 *  This line is either horizontal or vertical **/
bool Cropper::lineHasBlankPixels(const Point2i &st, const Point2i &ed) {
    bool verticalLine = (st.x == ed.x);
    bool horizontalLine = (st.y == ed.y);
    //cout<<"vertical = "<<verticalLine<<"; horizon = "<<horizontalLine<<endl;
    if(!verticalLine && !horizontalLine)return true; // this case is not processed
    
    Point2i pt(st);
    int kstep = 1;
    if(verticalLine){
        if (ed.y<st.y) kstep = -1;
        for(pt.y=st.y; pt.y<=ed.y; pt.y += kstep){
            if (mask.at<uchar>(pt)==0){
                return true;
            }
        }
        return false;
    }

    // -- horizontalLine
    if (ed.x < st.x) kstep = -1;
    for(pt.x=st.x; pt.x<=ed.x; pt.x += kstep){
        if (mask.at<uchar>(pt)==0){
            return true;
        }
    }
    return false;
}

/** determine if a rectangle contains "blank" pixels **/
bool Cropper::rectHasBlankPixels(const cv::Rect &roi) {
    Point2i tr(roi.x + roi.width - 1, roi.y);
    Point2i bl(roi.x, roi.y + roi.height - 1);
    Point2i br(roi.x + roi.width - 1, roi.y + roi.height - 1);
    
    if(lineHasBlankPixels(roi.tl(), tr))
        return true;
    
    if(lineHasBlankPixels(tr, br))
       return true;
    
    if(lineHasBlankPixels(bl, br))
        return true;
    
    if(lineHasBlankPixels(roi.tl(), bl))
        return true;
    
    return false;
}

/** determine if a line contains "content", i.e, "non-blank" pixels **/
bool Cropper::lineHasContent(const Point2i &st, const Point2i &ed) {
    bool verticalLine = (st.x == ed.x);
    bool horizontalLine = (st.y == ed.y);
    if(!verticalLine && !horizontalLine)return true; // this case is not processed
    
    Point2i pt(st);
    int kstep = 1;
    if(verticalLine){
        if (ed.y<st.y) kstep = -1;
        for(pt.y=st.y; pt.y<=ed.y; pt.y += kstep){
            if (mask.at<uchar>(pt)>0){
                return true;
            }
        }
        return false;
    }
    
    // -- horizontalLine
    if (ed.x < st.x) kstep = -1;
    for(pt.x=st.x; pt.x<=ed.x; pt.x += kstep){
        if (mask.at<uchar>(pt)>0){
            return true;
        }
    }
    return false;
}


/** search until no more blank line **/
void Cropper::searchContentLine(Point2i &p1, Point2i &p2, Point2i &delta) {
    Point2i p3 = p1, p4 = p2;

    if(lineHasContent(p1, p2))return; // p1-p2 is the content line
    
    int stp = kstep;
    while(stp>=1){
        p3 = p1 + delta*stp;
        p4 = p2 + delta*stp;
        if(!p3.inside(rect)){
            stp /= 2; continue;
        }
        if(lineHasContent(p3, p4)){
            stp /= 2; continue;
        }
        p1 = p3; p2 = p4;
    }
    p1 = p1 + delta;
    p2 = p2 + delta;
}
