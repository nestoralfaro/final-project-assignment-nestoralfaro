#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
  // load the video
  string videoPath = "../tmp/parking/v4.mp4";
  VideoCapture cap(videoPath);
  if (!cap.isOpened()) {
    cerr << "Failed to open video" << endl;
    return -1;
  }

  Mat frame, gray, blurred, edges;
  while (true) {
    cap >> frame;

    if (frame.empty()) {
      cout << "End of video, looping..." << endl;
      cap.set(CAP_PROP_POS_FRAMES, 0);
      continue;
    }

    vector<Point> totalPoints;

    // convert to grayscale
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // apply Gaussian Blur
    GaussianBlur(gray, blurred, Size(5, 5), 1.5);

    // canny Edge Detection
    Canny(blurred, edges, 50, 150);

    // detect lines using Hough Transform
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI/180, 50, 50, 10);

    // draw detected lines
    for (size_t i = 0; i < lines.size(); i++) {
      Vec4i l = lines[i];
      Point p1(l[0], l[1]), p2(l[2], l[3]);
      totalPoints.push_back(p1);
      totalPoints.push_back(p2);
      line(frame, p1, p2, Scalar(0, 255, 0), 2, LINE_AA);
    }

    // contour detection
    vector<vector<Point>> contours;
    // findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(edges, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // filter contours and draw bounding boxes
    for (size_t i = 0; i < contours.size(); i++) {
      Rect boundRect = boundingRect(contours[i]);
      // adjust the area threshold as needed
      if (boundRect.area() > 500) {
        totalPoints.push_back(boundRect.tl());
        totalPoints.push_back(boundRect.br());
        rectangle(frame, boundRect.tl(), boundRect.br(), Scalar(0, 0, 255), 2);

        // check if the bounding box is between two lines
        for (size_t j = 0; j < lines.size(); j++) {
          Vec4i l = lines[j];
          Point linePoint1(l[0], l[1]);
          Point linePoint2(l[2], l[3]);
          // Assuming horizontal lines
          if (linePoint1.y == linePoint2.y) {
            if (boundRect.y >= min(linePoint1.y, linePoint2.y) &&
              boundRect.y + boundRect.height <= max(linePoint1.y, linePoint2.y)) {
              putText(frame, "Car Detected", boundRect.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
              break;
            }
          }
        }
      }
    }

    int rectWidth = frame.cols / 3;
    int rectHeight = frame.rows;
    for (int i = 0; i < 3; ++i) {
      Point topLeft(i * rectWidth, 0);
      Point bottomRight((i + 1) * rectWidth, rectHeight);

      rectangle(frame, topLeft, bottomRight, Scalar(255, 0, 0), 2);

      // display the count on the frame
      int count = 0;
      for (size_t i = 0; i < totalPoints.size(); i++) {
        if (totalPoints[i].x >= topLeft.x && totalPoints[i].x <= bottomRight.x && totalPoints[i].y >= topLeft.y && totalPoints[i].y <= bottomRight.y) {
          count += 1;
        }
      }
      string text = to_string(count) + (count > 25 ? "Car!" : "");
      double fontScale = 1.0;
      Point textOrg(topLeft.x + 10, topLeft.y + 50);
      putText(frame, text, textOrg, FONT_HERSHEY_SIMPLEX, fontScale, Scalar(255, 0, 0), 2);
    }

    // show the result
    imshow("gray", gray);
    imshow("blurred", blurred);
    imshow("edges", edges);
    imshow("main", frame);

    // break the loop on 'q' key press
    if (waitKey(1) == 'q') {
        break;
    }
  }

  cap.release();
  destroyAllWindows();

  return 0;
}


/*
#include <opencv2/opencv.hpp>
#include <iostream>
#include <unistd.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
  std::string videoPath = "../tmp/parking/v4.mp4";
  // string videoPath = "../tmp/parking/v1.mp4";
  VideoCapture cap(videoPath);
  if (!cap.isOpened()) {
    cerr << "failed to open video" << endl;
    return -1;
  }

  Mat frame, imgGray, imgBlur, imgThreshold;

  vector<pair<Point, Point>> rectangles = {
    { Point(25, 20), Point(280, 470) }
    , { Point(305, 20), Point(590, 470) }
    , { Point(600, 20), Point(850, 470) }
  };

  while (true) {
    // capture a new frame
    cap >> frame;
    if (frame.empty()) {
      cout << "end of video, looping..." << endl;
      cap.set(CAP_PROP_POS_FRAMES, 0);
      continue;
    }

    Scalar green(0, 255, 0), white(255, 255, 255);
    int thickness = 2;

    for (const auto& rect : rectangles) {
      // convert frame to grayscale and apply glaussian blur
      cvtColor(frame, imgGray, COLOR_BGR2GRAY);
      GaussianBlur(imgGray, imgBlur, Size(3, 3), 1);
      adaptiveThreshold(imgBlur, imgThreshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 25, 16);

      // count non-zero pixels in the thresholded image
      int count = countNonZero(imgThreshold);

      // draw rectangles
      rectangle(frame, rect.first, rect.second, green, thickness);
      rectangle(imgThreshold, rect.first, rect.second, white, thickness);

      // display the count on the frame
      string text = to_string(count);
      double fontScale = 1.0;
      Point textOrg(rect.first.x + 10, rect.first.y + 50);
      putText(frame, text, textOrg, FONT_HERSHEY_SIMPLEX, fontScale, green, thickness);
      putText(imgThreshold, text, textOrg, FONT_HERSHEY_SIMPLEX, fontScale, white, thickness);
    }

    // display frames
    imshow("video", frame);
    imshow("imgThreshold", imgThreshold);

    // break while loop on `q` key press
    if (waitKey(1) == 'q') {
      break;
    }
  }

  // release the video capture object
  cap.release();

  // close all OpenCV windows
  destroyAllWindows();

  return 0;
}
*/
