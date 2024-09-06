#include <csignal>
#include <ctime>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#define MANY_SPOTS 3
#define OPEN 0
#define BUSY 1
#define CREATE_SPOTS_ARRAY(type, name) type name[MANY_SPOTS]

volatile sig_atomic_t terminateFlag = 0;
typedef unsigned short ushort;
void emailUpdate(uint16_t* spots);
void daemonize();
void handleSignal(int signal);

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "<path_to_file|-d>" << std::endl;
    return 1;
  }
  cv::VideoCapture cap;
  if (std::strcmp(argv[1], "-d") == 0) {
    std::cout << "Daemonizing openspot." << std::endl;
    cap.open(0);
    std::signal(SIGTERM, handleSignal);
    daemonize();
  }
  else {
    // load the video
    std::string videoPath = argv[1];
    std::cout << "Loading video " << videoPath << std::endl;
    cap.open(videoPath);
  }
  if (!cap.isOpened()) {
    std::cerr << "Failed to open video" << std::endl;
    return -1;
  }
  openlog("openspot", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_LOCAL1);

  uint16_t bitPosition;
  cv::Mat frame, gray, blurred, edges;
  CREATE_SPOTS_ARRAY(uint16_t, spots);
  CREATE_SPOTS_ARRAY(bool, flags);

  // std::ofstream logfile("/tmp/tmpopenspot");
  // if (logfile.is_open()) logfile.close();

  while (!terminateFlag) {
    cap >> frame;
    if (frame.empty()) {
      syslog(LOG_ERR, "Frame is empty. Quitting now.");
      // std::cout << "End of video, looping..." << std::endl;
      // cap.set(cv::CAP_PROP_POS_FRAMES, 0);
      break;
    }

    std::vector<cv::Point> totalPoints;

    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);
    // canny Edge Detection
    cv::Canny(blurred, edges, 50, 150);
    // detect lines using Hough Transform
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI/180, 50, 50, 10);

    // draw detected lines
    for (ushort i = 0; i < lines.size(); i++) {
      cv::Vec4i l = lines[i];
      cv::Point p1(l[0], l[1]), p2(l[2], l[3]);
      totalPoints.push_back(p1);
      totalPoints.push_back(p2);
      line(frame, p1, p2, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    // contour detection
    std::vector< std::vector<cv::Point> > contours;
    // findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    cv::findContours(edges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // filter contours and draw bounding boxes
    for (ushort i = 0; i < contours.size(); i++) {
      cv::Rect boundRect = boundingRect(contours[i]);
      totalPoints.push_back(boundRect.tl());
      totalPoints.push_back(boundRect.br());
      // rectangle(frame, boundRect.tl(), boundRect.br(), Scalar(0, 0, 255), 2);
    }

    int spotBoundaryWidth = frame.cols / MANY_SPOTS;
    int spotBoundaryHeight = frame.rows;
    for (ushort i = 0; i < MANY_SPOTS; ++i) {
      cv::Point topLeft(i * spotBoundaryWidth, 0);
      cv::Point bottomRight((i + 1) * spotBoundaryWidth, spotBoundaryHeight);
      // boundary rectangles
      //cv::rectangle(frame, topLeft, bottomRight, Scalar(255, 0, 0), 2);

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
      cv::Point textOrg(topLeft.x + 10, topLeft.y + 50);
      // cv::putText(frame, sd::to_string(count), textOrg, FONT_HERSHEY_SIMPLEX, fontScale, Scalar(0, 0, 255), 2);

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
    // if (waitKey(125) == 'q') {
    //     break;
    // }

    // std::this_thread::sleep_for(std::chrono::seconds(3));
  }

  cap.release();
  // destroyAllWindows();

  return 0;
}

void handleSignal(int signal) {
  if (signal == SIGTERM) {
    std::cout << "Received SIGTERM, cleaning up..." << std::endl;
    syslog(LOG_INFO, "Received SIGTERM, cleaning up...");
    closelog();
    terminateFlag = 1;
  }
}

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
  syslog(LOG_INFO, command.str().c_str());

  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  std::cout << command.str().c_str() << " - " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
  // what should have been emailed
  // std::ofstream file("/tmp/tmpopenspot", std::ios::app);
  // if (file.is_open()) {
  //   file << command.str().c_str() << " - " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
  //   file.close();
  // }

  /* executing msmtp itself */
  // int result = system(command.str().c_str());
  // if (result == -1) {
  //   std:cerr << "Failed to email." << std::endl;
  // }
}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Exit the parent process.
        exit(EXIT_SUCCESS);
    }

    // Create a new session.
    if (setsid() < 0) {
        std::cerr << "Failed to create a new session" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Fork again to prevent the daemon from acquiring a terminal.
    pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Set the file mode mask.
    umask(0);

    // Redirect standard file descriptors.
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}
