crop-images
===========

A class for cropping OpenCV images

The image to be cropped must be an independant cv::Mat. It can not
be a sub-matrix because of the way this class transverses every
pixel in the image. For fast processing, the class does not use 
Mat.at<>(). Instead, it assumes all mat elements are stored 
continuously. Thus in the case the image is a ROI of a bigger image,
it will not visit all pixels correctly.
