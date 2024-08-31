#include <cstdint>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>

#define MANY_SPOTS 3
#define OPEN 0
#define BUSY 1
#define CREATE_SPOTS_ARRAY(type, name) type name[MANY_SPOTS]

using namespace cv;
using namespace std;
typedef unsigned short ushort;

void emailUpdate(uint16_t* spots) { 
  std::ostringstream command;
  command << "echo \"Subject: ";
  ushort openSpots = 0;
  for (ushort i = 0; i < MANY_SPOTS; ++i) {
    if (spots[i] == 0) {
      openSpots += 1;
    }
  }
  command << openSpots << " | ";
  for (ushort s = 0; s < MANY_SPOTS; ++s) {
    command << "[" << (spots[s] == 0 ? "   " : "CAR" ) << "]";
  }
  // command << "\" | msmtp -t";
  command << "\" | msmtp openspotrpi@gmail.com";

  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  std::cout << command.str().c_str() << " - " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
  // what should have been emailed
  std::ofstream file("/tmp/tmpopenspot", std::ios::app);
  if (file.is_open()) {
    file << command.str().c_str() << " - " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
    file.close();
  }

  // int result = system(command.str().c_str());
  // if (result == -1) {
  //   std:cerr << "Failed to email." << std::endl;
  // }
}

int main(int argc, char** argv) {
  // load the video
  // string videoPath = "../tmp/parking/vfast1.mp4";
  // VideoCapture cap(videoPath);
  VideoCapture cap(0);
  if (!cap.isOpened()) {
    cerr << "Failed to open video" << endl;
    return -1;
  }

  uint16_t bitPosition;
  Mat frame, gray, blurred, edges;
  CREATE_SPOTS_ARRAY(uint16_t, spots);
  CREATE_SPOTS_ARRAY(bool, flags);

  std::ofstream logfile("/tmp/tmpopenspot");
  if (logfile.is_open()) logfile.close();

  while (true) {
    cap >> frame;
    if (frame.empty()) {
      cout << "End of video, looping..." << endl;
      cap.set(CAP_PROP_POS_FRAMES, 0);
      continue;
    }

    vector<Point> totalPoints;

    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 1.5);
    // canny Edge Detection
    Canny(blurred, edges, 50, 150);
    // detect lines using Hough Transform
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI/180, 50, 50, 10);

    // draw detected lines
    for (ushort i = 0; i < lines.size(); i++) {
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
    for (ushort i = 0; i < contours.size(); i++) {
      Rect boundRect = boundingRect(contours[i]);
      totalPoints.push_back(boundRect.tl());
      totalPoints.push_back(boundRect.br());
      // rectangle(frame, boundRect.tl(), boundRect.br(), Scalar(0, 0, 255), 2);
    }

    int spotBoundaryWidth = frame.cols / MANY_SPOTS;
    int spotBoundaryHeight = frame.rows;
    for (ushort i = 0; i < MANY_SPOTS; ++i) {
      Point topLeft(i * spotBoundaryWidth, 0);
      Point bottomRight((i + 1) * spotBoundaryWidth, spotBoundaryHeight);
      // boundary rectangles
      rectangle(frame, topLeft, bottomRight, Scalar(255, 0, 0), 2);

      // display the count on the frame
      ushort count = 0;
      for (ushort i = 0; i < totalPoints.size(); i++) {
        if (totalPoints[i].x >= topLeft.x && totalPoints[i].x <= bottomRight.x && totalPoints[i].y >= topLeft.y && totalPoints[i].y <= bottomRight.y) {
          count += 1;
        }
      }
      if (count > (spotBoundaryHeight * 0.09)) {
        // turn the next bit on in a ring-like fashion
        spots[i] |= (1 << bitPosition);
      }
      else {
        // turn the next bit off in a ring-like fashion
        // spots[i] &= ~(1 << bitPosition);

        // just reset the bits instead
        // thus making the real-time car detection more demanding
        // i.e., all the bits must be turned on in a streak
        spots[i] = 0;
      }
      double fontScale = 1.0;
      Point textOrg(topLeft.x + 10, topLeft.y + 50);
      putText(frame, to_string(count), textOrg, FONT_HERSHEY_SIMPLEX, fontScale, Scalar(0, 0, 255), 2);
      // std::cout << "Bits: " << std::bitset<16>(spots[i]) << std::endl;
      if (spots[i] == 0xFFFF && flags[i] != BUSY) {
        // all bits are on: car detected!
        flags[i] = BUSY;
        // std::cout << "spot[" << i << "] = " << std::bitset<16>(spots[i]) << " busy." << std::endl;
        emailUpdate(spots);

        // for (int s = 0; s < MANY_SPOTS; ++s) {
        //   std::cout << " [" << (spots[s] == 0 ? "   " : "CAR" ) << "] ";
        // }
        // std::cout << std::endl;
      }
      if (spots[i] == 0 && flags[i] != OPEN) {
        // all bits are off: open spot!
        flags[i] = OPEN;
        // std::cout << "spot[" << i << "] = " << std::bitset<16>(spots[i]) << " open." << std::endl;
        emailUpdate(spots);

        // for (int s = 0; s < MANY_SPOTS; ++s) {
        //   std::cout << " [" << (spots[s] == 0 ? "   " : "CAR" ) << "] ";
        // }
        // std::cout << std::endl;
      }
      // putText(frame, "<car|nocar>", textOrg, FONT_HERSHEY_SIMPLEX, fontScale, Scalar(255, 0, 0), 2);
      bitPosition = (bitPosition + 1) % 16;
    }

    // show the result
    // imshow("gray", gray);
    // imshow("blurred", blurred);
    // imshow("edges", edges);
    // imshow("main", frame);

    // break the loop on 'q' key press
    if (waitKey(125) == 'q') {
        break;
    }
  }

  cap.release();
  destroyAllWindows();

  return 0;
}
