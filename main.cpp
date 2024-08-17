#include <iostream>

int main(int argc, char** argv) {
  std::cout << "OpenSpot closed. Come back later" << std::endl;
  return 0;
}

/*
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstdio>

using namespace cv;
using namespace std;

// Placeholder function to check if the spot is free
bool is_spot_free(Mat spot_img);

// Function to detect parking spots
int detect_parking_spots(Mat frame) {
  // Mat gray, blurred, edges;
  // cvtColor(frame, gray, COLOR_BGR2GRAY);
  // GaussianBlur(gray, blurred, Size(15, 15), 0);
  // Canny(blurred, edges, 50, 150);
  //
  // vector<vector<Point>> contours;
  // findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  //
  int available_spots = 0;
  //
  // for (size_t i = 0; i < contours.size(); i++) {
  //   if (contourArea(contours[i]) > 500) {
  //     Rect rect = boundingRect(contours[i]);
  //     rectangle(frame, rect, Scalar(0, 255, 0), 2);
  //     if (is_spot_free(frame(rect))) {
  //       available_spots++;
  //     }
  //   }
  // }
  //
  return available_spots;
}

// Placeholder function to check if the spot is free
bool is_spot_free(Mat spot_img) {
  // Implement your logic to determine if the spot is free
  // This can be based on background subtraction, color detection, etc.
  return true;  // Placeholder return true for demo
}

int main(int argc, char** argv) {
  VideoCapture cap(0); // Open the default camera
  if (!cap.isOpened()) {
    cerr << "Error opening video stream" << endl;
    return -1;
  }

  // adjust camera settings
  cap.set(CAP_PROP_FRAME_WIDTH, 640);
  cap.set(CAP_PROP_FRAME_HEIGHT, 480);
  cap.set(CAP_PROP_BRIGHTNESS, 0.5);
  cap.set(CAP_PROP_CONTRAST, 0.5);
  cap.set(CAP_PROP_SATURATION, 0.5);
  cap.set(CAP_PROP_GAIN, 0.5);
  cap.set(CAP_PROP_EXPOSURE, -4);

  while (true) {
    Mat frame;
    cap >> frame; // Capture a frame
    if (frame.empty()) break;

    string input_filename = "parking_lot.jpg";
    imwrite(input_filename, frame); // Save the captured frame

    frame = imread(input_filename); // Load the saved frame for processing

    int available_spots = detect_parking_spots(frame);
    cout << "Available parking spots: " << available_spots << endl;

    string output_filename = "processed_parking_lot.jpg";
    imwrite(output_filename, frame); // Save the processed image

    if (remove(input_filename.c_str()) != 0) {
      perror("Error deleting the file");
    }

    waitKey(15000); // Wait for 15 seconds
  }

  cap.release();
  destroyAllWindows();
  return 0;
}
*/
