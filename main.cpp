#include <iostream>

#include "qrcode.hpp"

#include <opencv2/opencv.hpp>
#include <quirc.h>
#include <algorithm>
#include <numeric>

const char* keys =
    "{ help  h | | Print help message. }"
    "{ test  t | | Run tests. }"
    "{ input i | | Path to input image or video. Skip to grab frames from a camera. }";

bool verifyVertical(int center_x, int center_y, const cv::Mat& img, int* top, int* bottom);

bool verifyDiagonal(int center_x, int center_y, const cv::Mat& img);

cv::Mat extract(const cv::Mat& img, const cv::Point& topLeft,
                const cv::Point& topRight, const cv::Point& bottomLeft);

int main(int argc, char** argv)
{   //                                                      _
    /////////////////////////////////////////////////      (_)>
    // Parse a command line.                              \(/)
    ///////////////////////////////////////////////////    ^ ^
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("QR codes detector");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    if (parser.has("test"))
    {
        return runTests() ? 0 : 1;
    }

    //
    // Open an input file or a camera stream.
    //
    cv::VideoCapture cap;
    if (parser.has("input"))
        cap.open(parser.get<std::string>("input"));
    else
        cap.open(0);

    cv::namedWindow("Markers", cv::WINDOW_NORMAL);
    cv::namedWindow("QR code", cv::WINDOW_NORMAL);
    cv::Mat img, gray, bin, mask;
    while (cv::waitKey(1) < 0)
    {
        // Read an image.
        cap >> img;
        if (img.empty())
        {
            img = cv::imread(parser.get<std::string>("input"));
            if (img.empty())
            {
                cv::waitKey();
                break;
            }
        }

        // TODO: Convert BGR image to grayscale.
        //          __
        //     __  (  )_
        //    (  )(     )
        //   (___(_______)
        //     /  /  /  /
        //   /  /  /  /
        bgr2gray(img, gray);

        // TODO: Convert grayscale image to black-and-white.
        gray2bin(gray, bin);

        std::string msg = decode(bin, img, mask);
        cv::imshow("Markers", img);
        cv::imshow("Black-and-white image", bin);
        if (!mask.empty())
            cv::imshow("QR code", mask);
        if (!msg.empty())
            std::cout << "Message: " << msg << std::endl;
    }
    return 0;
}

void bgr2gray(const cv::Mat& src, cv::Mat& dst)
{
    cv::cvtColor(src, dst, CV_BGR2GRAY);
}

void gray2bin(const cv::Mat& src, cv::Mat& dst, uint8_t thresh)
{
    cv::threshold(src, dst, thresh, 255, CV_THRESH_BINARY);
}

void countPixels(const uint8_t* row, int length, std::vector<int>& counts,
                 std::vector<int>& xs) {
    int cur_val = 0;
    auto cur = row;
    auto first = row;
    auto last = row + length;
    for(;;) {
        cur = std::find(cur, last, cur_val);
        if (cur==last) break;
        xs.push_back(static_cast<int>(cur - first));
        cur_val ^= 255;
    }
    for(auto i = 1; i<xs.size(); i++) {
        counts.push_back(xs[i] - xs[i-1]);
    }
    if(xs.size()) counts.push_back(length - *xs.rbegin());
}
bool checkRatios(const int* counts)
{
    static constexpr double eps = 1.5;
    auto el = {counts[0], counts[1], counts[3], counts[4]};
    double min = *std::min_element(el.begin(), el.end());
    double max = *std::max_element(el.begin(), el.end());
    double mean = static_cast<double>(std::accumulate(el.begin(), el.end(), 0)) / 4;
    double mid = counts[2];
    double a = mid / mean;
    if ( a > 3 * eps || a < 3 / eps) return false;
    return max / min <= eps;
}

void computeCenters(const std::vector<cv::Rect>& rects, std::vector<cv::Point>& centers) {
    std::vector<char> used(rects.size(), false);
    std::vector<cv::Rect> intersections;

    for (int i = 0; i < rects.size(); i++) {
        if (used[i]) continue;
        used[i] = true;
        intersections.push_back(rects[i]);
        for (int j = i+1; j < rects.size(); j++) {
            if (used[j]) continue;
            if ((*intersections.rbegin() & rects[j]).area()) {
                *intersections.rbegin() &= rects[j];
                used[j] = true;
            }
        }
    }

    for (const auto &i : intersections) {
        centers.push_back(cv::Point(i.x + i.width/2, i.y + i.height/2));
    }
}

void sortMarkers(const std::vector<cv::Point>& centers, cv::Point& topLeft,
                 cv::Point& topRight, cv::Point& bottomLeft)
{
    std::vector<cv::Point> vectors = {centers[1] - centers[2], centers[0] - centers[2], centers[1] - centers[0]};
    auto copyCenters(centers);
    topLeft = centers[std::distance(vectors.begin(), std::max_element(vectors.begin(), vectors.end(),
                                                              [](cv::Point &a, cv::Point &b) {
                                                                return a.dot(a) < b.dot(b);
                                                              }))];

    copyCenters.erase(std::remove_if(copyCenters.begin(), copyCenters.end(), [&topLeft](cv::Point &center){
      return center == topLeft;
    }));

    if((copyCenters[0] - topLeft).cross(copyCenters[1] - topLeft) < 0) {
        bottomLeft = copyCenters[0];
        topRight = copyCenters[1];
    } else {
        bottomLeft = copyCenters[1];
        topRight = copyCenters[0];
    }
}

std::string decode(const cv::Mat& bin, cv::Mat& img, cv::Mat& mask)
{
    // Parse an every row to find desired ratios.
    std::vector<cv::Rect> markersCandidates;
    for (int y = 0; y < bin.rows; ++y)
    {
        std::vector<int> counts;  // Numbers of sequent black & white pixels.
        std::vector<int> xs;      // Indices of first pixels of an every group.
        countPixels(bin.ptr<uint8_t>(y), bin.cols, counts, xs);

        if (counts.size() < 5)
            continue;

        CV_Assert(xs.size() == counts.size());
        xs.push_back(bin.cols);  // For simplification.
        for (int i = 0; i < counts.size() - 5; i += 2)
        {
            // Compare ratios. Try to find 1:1:3:1:1
            int top, bottom, center_x = (xs[i] + xs[i + 5]) / 2;
            if (checkRatios(&counts[i]) &&
                verifyVertical(center_x, y, bin, &top, &bottom) &&
                verifyDiagonal(center_x, y, bin))
            {
                cv::line(img, cv::Point(xs[i], y), cv::Point(xs[i + 5], y), cv::Scalar(0, 255, 0));
                cv::line(img, cv::Point(center_x, top), cv::Point(center_x, bottom), cv::Scalar(0, 255, 0));

                cv::Rect candidate;
                candidate.x = xs[i];
                candidate.y = top;
                candidate.width = xs[i + 5] - xs[i];
                candidate.height = bottom - top + 1;

                markersCandidates.push_back(candidate);

                CV_Assert(bin.at<uint8_t>(y, xs[i]) == 0);
                CV_Assert(bin.at<uint8_t>(y, xs[i + 5] - 1) == 0);
                CV_Assert(bin.at<uint8_t>(top, center_x) == 0);
                CV_Assert(bin.at<uint8_t>(bottom, center_x) == 0);
            }
        }
    }

    // Estimates centers of each marker.
    std::vector<cv::Point> centers;
    computeCenters(markersCandidates, centers);
    if (centers.size() != 3)
        return "";

    if (!centers.empty())
    {
        // Identify each marker location.
        cv::Point topLeft, topRight, bottomLeft;
        sortMarkers(centers, topLeft, topRight, bottomLeft);

        // Draw markers.
        cv::circle(img, topRight, 5, cv::Vec3b(255, 0, 0), CV_FILLED);
        cv::circle(img, topLeft, 5, cv::Vec3b(255, 0, 255), CV_FILLED);
        cv::circle(img, bottomLeft, 5, cv::Vec3b(0, 0, 255), CV_FILLED);

        // Extract a qr code.
        mask = extract(bin, topLeft, topRight, bottomLeft);
    }

    // 001000000101101100001011011110001101000101110010110111000100110101000
    // Decoding
    // 011010000001110110000010001111011000010000001011011000010110111100011
    quirc_code qCode;
    memset(&qCode, 0, sizeof(qCode));

    qCode.size = 21;
    for (int y = 0; y < 21; ++y)
    {
        for (int x = 0; x < 21; ++x)
        {
            int p = y * 21 + x;
            qCode.cell_bitmap[p >> 3] |= mask.at<uint8_t>(y, x) ? 0 : (1 << (p & 7));
        }
    }

    quirc_data qData;
    quirc_decode_error_t errorCode = quirc_decode(&qCode, &qData);
    std::string msg = "";
    if (errorCode == 0)
    {
        for (int i = 0; i < qData.payload_len; ++i)
        {
            msg += qData.payload[i];
        }
    }
    return msg;
}

bool verifyVertical(int center_x, int center_y, const cv::Mat& img, int* top, int* bottom)
{
    std::vector<int> counts(5, 0);
    int y, x = center_x, numPixels = 0;
    for (y = center_y; y >= 0 && img.at<uint8_t>(y, x) == 0; --y, counts[2] += 1, ++numPixels) {}
    for (; y >= 0 && img.at<uint8_t>(y, x) == 255; --y, counts[1] += 1, ++numPixels) {}
    for (; y >= 0 && img.at<uint8_t>(y, x) == 0; --y, counts[0] += 1, ++numPixels) {}
    *top = y + 1;
    for (y = center_y + 1; y < img.rows && img.at<uint8_t>(y, x) == 0; ++y, counts[2] += 1, ++numPixels) {}
    for (; y < img.rows && img.at<uint8_t>(y, x) == 255; ++y, counts[3] += 1, ++numPixels) {}
    for (; y < img.rows && img.at<uint8_t>(y, x) == 0; ++y, counts[4] += 1, ++numPixels) {}
    *bottom = y - 1;

    return checkRatios(&counts[0]);
}

bool verifyDiagonal(int center_x, int center_y, const cv::Mat& img)
{
    std::vector<int> counts(5, 0);
    int y, x = center_x, numPixels = 0;
    for (y = center_y; y >= 0 && x >= 0 && img.at<uint8_t>(y, x) == 0; --y, --x, counts[2] += 1, ++numPixels) {}
    for (; y >= 0 && x >= 0 && img.at<uint8_t>(y, x) == 255; --y, --x, counts[1] += 1, ++numPixels) {}
    for (; y >= 0 && x >= 0 && img.at<uint8_t>(y, x) == 0; --y, --x, counts[0] += 1, ++numPixels) {}
    for (y = center_y + 1, x = center_x + 1; y < img.rows && x < img.cols && img.at<uint8_t>(y, x) == 0; ++y, ++x, counts[2] += 1, ++numPixels) {}
    for (; y < img.rows && x < img.cols && img.at<uint8_t>(y, x) == 255; ++y, ++x, counts[3] += 1, ++numPixels) {}
    for (; y < img.rows && x < img.cols && img.at<uint8_t>(y, x) == 0; ++y, ++x, counts[4] += 1, ++numPixels) {}

    return checkRatios(&counts[0]);
}

cv::Mat extract(const cv::Mat& img, const cv::Point& topLeft,
                const cv::Point& topRight, const cv::Point& bottomLeft)
{
    cv::Point2f bottomRight = bottomLeft + topRight - topLeft;

    std::vector<cv::Point2f> srcPoints(4), dstPoints(4);
    srcPoints[0] = topRight;     dstPoints[0] = cv::Point2f(17, 3);
    srcPoints[1] = topLeft;      dstPoints[1] = cv::Point2f(3, 3);
    srcPoints[2] = bottomLeft;   dstPoints[2] = cv::Point2f(3, 17);
    srcPoints[3] = bottomRight;  dstPoints[3] = cv::Point2f(17, 17);
    cv::Mat m = cv::findHomography(srcPoints, dstPoints);

    cv::Mat res(21, 21, CV_8UC1);
    cv::warpPerspective(img, res, m, res.size(), cv::INTER_NEAREST);
    return res;
}
