#include <iostream>
#include <iomanip>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "MarkerTracker.h"
#include "PoseEstimation.h"

using namespace std;

void trackbarHandler(int pos, void* slider_value) {
    *((int*)slider_value) = pos;
}

void bw_trackbarHandler(int pos, void* slider_value) {
    *((int*)slider_value) = pos;
}

int subpixSampleSafe(const cv::Mat& pSrc, const cv::Point2f& p) {
    auto x = int(floorf(p.x));
    auto y = int(floorf(p.y));

    if (x < 0 || x >= pSrc.cols - 1 ||
        y < 0 || y >= pSrc.rows - 1)
        return 127;

    auto dx = int(256 * (p.x - floorf(p.x)));
    auto dy = int(256 * (p.y - floorf(p.y)));

    auto* i = (pSrc.data + y * pSrc.step) + x;
    int a = i[0] + ((dx * (i[1] - i[0])) >> 8);
    i += pSrc.step;
    int b = i[0] + ((dx * (i[1] - i[0])) >> 8);
    return a + ((dy * (b - a)) >> 8);
}

void MarkerTracker::init() {
    std::cout << "Startup\n";
    //cv::namedWindow(kWinName1, CV_WINDOW_AUTOSIZE);
    //cv::namedWindow(kWinName2, CV_WINDOW_AUTOSIZE);
    //cv::namedWindow(kWinName3, CV_WINDOW_AUTOSIZE);
    cv::namedWindow(kWinName4, 0);
    cvResizeWindow("Exercise 8 - Marker", 120, 120);

    int max = 255;
    int slider_value = 100;
    cv::createTrackbar("Threshold", kWinName2, &slider_value, 255, trackbarHandler, &slider_value);

    int bw_sileder_value = bw_thresh;
    cv::createTrackbar("BW Threshold", kWinName2, &slider_value, 255, bw_trackbarHandler, &bw_sileder_value);

    memStorage = cvCreateMemStorage();
}

void MarkerTracker::cleanup() {
    cvReleaseMemStorage(&memStorage);

    cv::destroyWindow(kWinName1);
    cv::destroyWindow(kWinName2);
    cv::destroyWindow(kWinName3);
    cv::destroyWindow(kWinName4);
    std::cout << "Finished\n";
}

void MarkerTracker::findMarker(cv::Mat& img_bgr, std::vector<Marker>& markers) {
    bool isFirstStripe = true;

    bool isFirstMarker = true;

    {
        cv::cvtColor(img_bgr, img_gray, CV_BGR2GRAY);
        //cv::threshold(img_gray, img_mono, 140, 255, CV_THRESH_BINARY);
        cv::adaptiveThreshold(img_gray, img_mono, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 41, 15);

        // Find Contours with old OpenCV APIs
        CvSeq* contours;
        CvMat img_mono_(img_mono);

        cvFindContours(
            &img_mono_, memStorage, &contours, sizeof(CvContour),
            CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE
        );

        for (; contours; contours = contours->h_next) {
            CvSeq* result = cvApproxPoly(
                contours, sizeof(CvContour), memStorage, CV_POLY_APPROX_DP,
                cvContourPerimeter(contours) * 0.02, 0
            );

            if (result->total != 4) {
                continue;
            }

            cv::Mat result_ = cv::cvarrToMat(result); /// API 1.X to 2.x
            cv::Rect r = cv::boundingRect(result_);
            if (r.height < 20 || r.width < 20 || r.width > img_mono.cols - 10 || r.height > img_mono.rows - 10) {
                continue;
            }

            const auto* rect = (const cv::Point*)result_.data;
            int npts = result_.rows;
            // draw the polygon
            cv::polylines(img_bgr, &rect, &npts, 1,
                true,           // draw closed contour (i.e. joint end to start)
                CV_RGB(255, 0, 0),// colour RGB ordering (here = green)
                2,              // line thickness
                CV_AA, 0);

            float lineParams[16];
            cv::Mat lineParamsMat(cv::Size(4, 4), CV_32F, lineParams); // lineParams is shared

            for (int i = 0; i < 4; ++i) {
                cv::circle(img_bgr, rect[i], 3, CV_RGB(0, 255, 0), -1);

                double dx = (double)(rect[(i + 1) % 4].x - rect[i].x) / 7.0;
                double dy = (double)(rect[(i + 1) % 4].y - rect[i].y) / 7.0;

                auto stripeLength = (int)(0.8 * sqrt(dx * dx + dy * dy));
                if (stripeLength < 5)
                    stripeLength = 5;

                //make stripeLength odd (because of the shift in nStop)
                stripeLength |= 1;

                //e.g. stripeLength = 5 --> from -2 to 2
                int nStop = stripeLength >> 1;
                int nStart = -nStop;

                cv::Size stripeSize;
                stripeSize.width = 3;
                stripeSize.height = stripeLength;

                cv::Point2f stripeVecX;
                cv::Point2f stripeVecY;

                //normalize vectors
                double diffLength = sqrt(dx * dx + dy * dy);
                stripeVecX.x = static_cast<float>(dx / diffLength);
                stripeVecX.y = static_cast<float>(dy / diffLength);

                stripeVecY.x = stripeVecX.y;
                stripeVecY.y = -stripeVecX.x;

                cv::Mat iplStripe(stripeSize, CV_8UC1);
                ///				IplImage* iplStripe = cvCreateImage( stripeSize, IPL_DEPTH_8U, 1 );

                                // Array for edge point centers
                cv::Point2f points[6];

                for (int j = 1; j < 7; ++j) {
                    double px = (double)rect[i].x + (double)j * dx;
                    double py = (double)rect[i].y + (double)j * dy;

                    cv::Point p;
                    p.x = (int)px;
                    p.y = (int)py;
                    cv::circle(img_bgr, p, 2, CV_RGB(0, 0, 255), -1);

                    for (int m = -1; m <= 1; ++m) {
                        for (int n = nStart; n <= nStop; ++n) {
                            cv::Point2f subPixel;

                            subPixel.x = static_cast<float>((double)p.x + ((double)m * stripeVecX.x) +
                                ((double)n * stripeVecY.x));
                            subPixel.y = static_cast<float>((double)p.y + ((double)m * stripeVecX.y) +
                                ((double)n * stripeVecY.y));

                            cv::Point p2;
                            p2.x = (int)subPixel.x;
                            p2.y = (int)subPixel.y;

                            if (isFirstStripe)
                                cv::circle(img_bgr, p2, 1, CV_RGB(255, 0, 255), -1);
                            else
                                cv::circle(img_bgr, p2, 1, CV_RGB(0, 255, 255), -1);

                            int pixel = subpixSampleSafe(img_gray, subPixel);

                            int w = m + 1; //add 1 to shift to 0..2
                            int h = n + (stripeLength >> 1); //add stripelenght>>1 to shift to 0..stripeLength

                            iplStripe.at<uchar>(h, w) = (uchar)pixel;
                            ///							*(iplStripe->imageData + h * iplStripe->widthStep  + w) =  pixel; //set pointer to correct position and safe subpixel intensity
                        }
                    }

                    //use sobel operator on stripe
                    // ( -1 , -2, -1 )
                    // (  0 ,  0,  0 )
                    // (  1 ,  2,  1 )
                    std::vector<double> sobelValues(stripeLength - 2);
                    ///					double* sobelValues = new double[stripeLength-2];
                    for (int n = 1; n < (stripeLength - 1); n++) {
                        unsigned char* stripePtr = &(iplStripe.at<uchar>(n - 1, 0));
                        ///						unsigned char* stripePtr = ( unsigned char* )( iplStripe->imageData + (n-1) * iplStripe->widthStep );
                        double r1 = -stripePtr[0] - 2 * stripePtr[1] - stripePtr[2];

                        stripePtr += 2 * iplStripe.step;
                        ///						stripePtr += 2*iplStripe->widthStep;
                        double r3 = stripePtr[0] + 2 * stripePtr[1] + stripePtr[2];
                        sobelValues[n - 1] = r1 + r3;
                    }

                    double maxVal = -1;
                    int maxIndex = 0;
                    for (int n = 0; n < stripeLength - 2; ++n) {
                        if (sobelValues[n] > maxVal) {
                            maxVal = sobelValues[n];
                            maxIndex = n;
                        }
                    }

                    double y0, y1, y2; // y0 .. y1 .. y2
                    y0 = (maxIndex <= 0) ? 0 : sobelValues[maxIndex - 1];
                    y1 = sobelValues[maxIndex];
                    y2 = (maxIndex >= stripeLength - 3) ? 0 : sobelValues[maxIndex + 1];

                    //formula for calculating the x-coordinate of the vertex of a parabola, given 3 points with equal distances
                    //(xv means the x value of the vertex, d the distance between the points):
                    //xv = x1 + (d / 2) * (y2 - y0)/(2*y1 - y0 - y2)

                    double pos = (y2 - y0) / (4 * y1 - 2 * y0 -
                        2 * y2); //d = 1 because of the normalization and x1 will be added later

// This would be a valid check, too
//if (std::isinf(pos)) {
//	// value is infinity
//	continue;
//}

                    if (pos != pos) {
                        // value is not a number
                        continue;
                    }

                    cv::Point2f edgeCenter; //exact point with subpixel accuracy
                    int maxIndexShift = maxIndex - (stripeLength >> 1);

                    //shift the original edgepoint accordingly
                    edgeCenter.x = static_cast<float>((double)p.x + (((double)maxIndexShift + pos) * stripeVecY.x));
                    edgeCenter.y = static_cast<float>((double)p.y + (((double)maxIndexShift + pos) * stripeVecY.y));

                    cv::Point p_tmp;
                    p_tmp.x = (int)edgeCenter.x;
                    p_tmp.y = (int)edgeCenter.y;
                    cv::circle(img_bgr, p_tmp, 1, CV_RGB(0, 0, 255), -1);

                    points[j - 1].x = edgeCenter.x;
                    points[j - 1].y = edgeCenter.y;

                    if (isFirstStripe) {
                        cv::Mat iplTmp;
                        cv::resize(iplStripe, iplTmp, cv::Size(100, 300));
                        //cv::imshow(kWinName3, iplTmp);//iplStripe );
                        isFirstStripe = false;
                    }

                } // end of loop over edge points of one edge

                // we now have the array of exact edge centers stored in "points"
                cv::Mat mat(cv::Size(1, 6), CV_32FC2, points);
                cv::fitLine(mat, lineParamsMat.col(i), CV_DIST_L2, 0, 0.01, 0.01);
                // cvFitLine stores the calculated line in lineParams in the following way:
                // vec.x, vec.y, point.x, point.y

                cv::Point p;
                p.x = (int)lineParams[8 + i] - (int)(50.0 * lineParams[i]);
                p.y = (int)lineParams[12 + i] - (int)(50.0 * lineParams[4 + i]);

                cv::Point p2;
                p2.x = (int)lineParams[8 + i] + (int)(50.0 * lineParams[i]);
                p2.y = (int)lineParams[12 + i] + (int)(50.0 * lineParams[4 + i]);

                cv::line(img_bgr, p, p2, CV_RGB(0, 255, 255), 1, 8, 0);

            } // end of loop over the 4 edges

            // so far we stored the exact line parameters and show the lines in the image
            // now we have to calculate the exact corners
            cv::Point2f corners[4];

            for (int i = 0; i < 4; ++i) {
                int j = (i + 1) % 4;
                double x0, x1, y0, y1, u0, u1, v0, v1;
                x0 = lineParams[i + 8];
                y0 = lineParams[i + 12];
                x1 = lineParams[j + 8];
                y1 = lineParams[j + 12];

                u0 = lineParams[i];
                v0 = lineParams[i + 4];
                u1 = lineParams[j];
                v1 = lineParams[j + 4];

                // (x|y) = p + s * vec
                // s = Ds / D (see cramer's rule)
                // (x|y) = p + (Ds / D) * vec
                // (x|y) = (p * D / D) + (Ds * vec / D)
                // (x|y) = (p * D + Ds * vec) / D
                // (x|y) = a / c;
                double a = x1 * u0 * v1 - y1 * u0 * u1 - x0 * u1 * v0 + y0 * u0 * u1;
                double b = -x0 * v0 * v1 + y0 * u0 * v1 + x1 * v0 * v1 - y1 * v0 * u1;
                double c = v1 * u0 - v0 * u1;

                if (fabs(c) < 0.001) //lines parallel?
                {
                    std::cout << "lines parallel" << std::endl;
                    continue;
                }

                a /= c;
                b /= c;

                //exact corner
                corners[i].x = static_cast<float>(a);
                corners[i].y = static_cast<float>(b);
                cv::Point p;
                p.x = (int)corners[i].x;
                p.y = (int)corners[i].y;

                cv::circle(img_bgr, p, 5, CV_RGB(255, 255, 0), -1);
            } //finished the calculation of the exact corners

            cv::Point2f targetCorners[4];
            targetCorners[0].x = static_cast<float>(-0.5);
            targetCorners[0].y = static_cast<float>(-0.5);
            targetCorners[1].x = 5.5;
            targetCorners[1].y = static_cast<float>(-0.5);
            targetCorners[2].x = 5.5;
            targetCorners[2].y = 5.5;
            targetCorners[3].x = static_cast<float>(-0.5);
            targetCorners[3].y = 5.5;

            //create and calculate the matrix of perspective transform
            cv::Mat projMat(cv::Size(3, 3), CV_32FC1);
            projMat = cv::getPerspectiveTransform(corners, targetCorners);
            ///			cv::warpPerspectiveQMatrix ( corners, targetCorners, projMat);

                        //create image for the marker
            //			markerSize.width  = 6;
            //			markerSize.height = 6;
            cv::Mat iplMarker(cv::Size(6, 6), CV_8UC1);
            cv::Mat iplMarker2(cv::Size(6, 6), CV_8UC1);
            //change the perspective in the marker image using the previously calculated matrix
            cv::warpPerspective(img_gray, iplMarker, projMat, cv::Size(6, 6));
            cv::adaptiveThreshold(iplMarker, iplMarker, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 3, 15);

            /* this part is used to compare the new and old functions to detect marker
            cv::warpPerspective(img_gray, iplMarker2, projMat, cv::Size(6, 6));
            cv::threshold(iplMarker2, iplMarker2, 128, 255, CV_THRESH_BINARY);
            cv::namedWindow("Marker compare", 0);
            cvResizeWindow("Marker compare", 200, 200);
            cv::imshow("Marker compare", iplMarker2);
            */
            //now we have a B/W image of a supposed Marker

            // check if border is black, 5 white pixels are allowed on the boarder
            int code = 0;
            for (int i = 0; i < 6; ++i) {
                int pixel1 = iplMarker.at<uchar>(0, i);
                int pixel2 = iplMarker.at<uchar>(5, i);
                int pixel3 = iplMarker.at<uchar>(i, 0);
                int pixel4 = iplMarker.at<uchar>(i, 5);
                if ((pixel1 > 0) || (pixel2 > 0) || (pixel3 > 0) || (pixel4 > 0)) {
                    code += pixel1 + pixel2 + pixel3 + pixel4;
                }
            }
            code -= 5 * 255;
            if (code > 0) {
                continue;
            }

            if (isFirstMarker) {
                cv::imshow(kWinName4, iplMarker);
                //isFirstMarker = false;
            }

            //copy the BW values into cP
            int cP[4][4];
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    cP[i][j] = iplMarker.at<uchar>(i + 1, j + 1);
                    cP[i][j] = (cP[i][j] == 0) ? 1 : 0; //if black then 1 else 0
                }
            }

            //save the ID of the marker
            int codes[4];
            codes[0] = codes[1] = codes[2] = codes[3] = 0;
            for (int i = 0; i < 16; i++) {
                int row = i >> 2;
                int col = i % 4;

                codes[0] <<= 1;
                codes[0] |= cP[row][col]; // 0???

                codes[1] <<= 1;
                codes[1] |= cP[3 - col][row]; // 90???

                codes[2] <<= 1;
                codes[2] |= cP[3 - row][3 - col]; // 180???

                codes[3] <<= 1;
                codes[3] |= cP[col][3 - row]; // 270???
            }

            if ((codes[0] == 0) || (codes[0] == 0xffff)) {
                continue;
            }

            //account for symmetry
            code = codes[0];
            int angle = 0;
            for (int i = 1; i < 4; ++i) {
                if (codes[i] < code) {
                    code = codes[i];
                    angle = i;
                }
            }

            printf("Found: %04x\n", code);



            //correct the order of the corners
            if (angle != 0) {
                cv::Point2f corrected_corners[4];
                for (int i = 0; i < 4; i++) corrected_corners[(i + angle) % 4] = corners[i];
                for (int i = 0; i < 4; i++) corners[i] = corrected_corners[i];
            }

            // transfer screen coords to camera coords
            for (auto& corner : corners) {
                corner.x -= img_bgr.cols * 0.5; //here you have to use your own camera resolution (x) * 0.5
                corner.y =
                    static_cast<float>(-corner.y + img_bgr.rows *
                        0.5); //here you have to use your own camera resolution (y) * 0.5
            }

            // Added in Exercise 9 - Start *****************************************************************
            Marker marker;
            marker.code = code;

            estimateSquarePose(marker.resultMatrix, (cv::Point2f*)corners, kMarkerSize);

            markers.push_back(marker);

            // Added in Exercise 9 - End *****************************************************************

            //this part is only for printing
            /*
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    std::cout << std::setw(6);
                    std::cout << std::setprecision(5);
                    std::cout << marker.resultMatrix[4 * i + j] << " ";
                }
                //std::cout << marker.resultMatrix[3];
                std::cout << "\n";
            }
            std::cout << "\n";
            float x, y, z;
            x = marker.resultMatrix[3];
            y = marker.resultMatrix[7];
            z = marker.resultMatrix[11];
            std::cout << "length: " << sqrt(x * x + y * y + z * z) << "\n";
            std::cout << "\n";
            */
        } // end of loop over contours

        //cv::imshow(kWinName1, img_gray);
        //cv::imshow(kWinName2, img_mono);

        isFirstStripe = true;

        isFirstMarker = true;

        cvClearMemStorage(memStorage);
    } // end of main loop

    cvClearMemStorage(memStorage);

    int key = cvWaitKey(10);
    if (key == 27) exit(0);

    //	glutPostRedisplay();
}