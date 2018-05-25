#include <stdint.h>
#include <vector>

#include <opencv2/opencv.hpp>

// Compute number of sequent black or white pixels.
// @param[in]  row    Pointer to a row of pixels.
// @param[in]  length Number of elements.
// @param[out] counts Number of sequent pixels starts from the first white one.
// @param[out] xs     Indices of the initial pixel in each group.
void countPixels(const uint8_t* row, int length, std::vector<int>& counts,
                 std::vector<int>& xs);

// Check if blocks of pixels has ratios 1:1:3:1:1 (black-white-black-white-black)
// |x|x|x|x|x|x|x|  <- 1          NO
// |x| | | | | |x|  <- 1:5:1      NO
// |x| |x|x|x| |x|  <- 1:1:3:1:1  YES
// |x| |x|x|x| |x|  <- 1:1:3:1:1  YES
// |x| |x|x|x| |x|  <- 1:1:3:1:1  YES
// |x| | | | | |x|  <- 1:5:1      NO
// |x|x|x|x|x|x|x|  <- 1          NO
// @param[in] counts Pointer to data with at least 5 elements
bool checkRatios(const int* counts);

// Compute centers which are intersections of separate groups of rectangles.
// @param[in]  rects   Rectangles
// @param[out] centers Output intersections
void computeCenters(const std::vector<cv::Rect>& rects, std::vector<cv::Point>& centers);

// Considering that three points are QR code markers, determine their positions.
// [topLeft] ...... [topRight]
// ... . . ... . . .. . . .. .
// . . . . . . .. . . .. . . .
// [bottomLeft] .. .  . . ....
// @param[in] centers Three detected markers
// @param[out] topLeft    Top-Left marker location
// @param[out] topRight   Top-Right marker location
// @param[out] bottomLeft Bottom-Left marker location
void sortMarkers(const std::vector<cv::Point>& centers, cv::Point& topLeft,
                 cv::Point& topRight, cv::Point& bottomLeft);

bool runTests();
