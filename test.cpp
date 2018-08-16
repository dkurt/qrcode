#include "qrcode.hpp"

#include <iostream>

#define RUN_TEST(name) \
    std::cout << "run test " << #name << std::endl; \
    try { \
        name(); \
        std::cout << "OK"<< std::endl; \
    } catch (const std::exception& ex) { \
        std::cout << ex.what() << std::endl; \
        std::cout << "FAILED"<< std::endl; \
        passed = false; \
    } \
    std::cout << std::endl;

#define CHECK_EQ(a, b) \
    if (a != b) { \
        std::stringstream ss; \
        ss << __FILE__ << ":" << __LINE__ << " Expects " << #a << " == " << #b << " (" << a << " != " << b << ")"; \
        CV_Error(cv::Error::StsAssert, ss.str()); \
    }

void test_bgr2gray()
{
    cv::Mat src = (cv::Mat_<cv::Vec3b>(2, 3) <<
                   cv::Vec3b(91, 2, 79), cv::Vec3b(179, 52, 205), cv::Vec3b(236, 8, 181),
                   cv::Vec3b(239, 26, 248), cv::Vec3b(207, 218, 45), cv::Vec3b(183, 158, 101));
    cv::Mat ref = (cv::Mat_<uint8_t>(2, 3) << 35, 112, 86, 117, 165, 144);
    cv::Mat dst;
    bgr2gray(src, dst);
    CHECK_EQ(cv::countNonZero(ref != dst), 0);
}

void test_gray2bin()
{
    cv::Mat src(14, 15, CV_8U);
    randu(src, 0, 255);
    cv::Mat dst(src.size(), src.type());

    uint8_t thresh = 127;
    gray2bin(src, dst);
    CHECK_EQ(cv::countNonZero((dst > thresh) != dst), 0);
}

void test_countPixels_1()
{
    uint8_t data[] = {255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0};

    std::vector<int> counts, xs;
    countPixels(&data[0], sizeof(data), counts, xs);

    int targetCounts[] = {1, 1, 2, 3, 2};
    int targetXs[] = {2, 3, 4, 6, 9};

    CHECK_EQ(counts.size(), 5);
    CHECK_EQ(xs.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        CHECK_EQ(counts[i], targetCounts[i]);
        CHECK_EQ(xs[i], targetXs[i]);
    }
}

void test_countPixels_2()
{
    uint8_t data[] = {0, 0, 255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0};

    std::vector<int> counts, xs;
    countPixels(&data[0], sizeof(data), counts, xs);

    int targetCounts[] = {2, 2, 1, 1, 2, 3, 2};
    int targetXs[] = {0, 2, 4, 5, 6, 8, 11};

    CHECK_EQ(counts.size(), 7);
    CHECK_EQ(xs.size(), 7);
    for (int i = 0; i < 7; ++i)
    {
        CHECK_EQ(counts[i], targetCounts[i]);
        CHECK_EQ(xs[i], targetXs[i]);
    }
}


void test_checkRatios()
{
    //       ((
    //      <(0)
    //  ..    (\)/
    //  ....  ^ ^
    {
        int counts[] = {1, 1, 3, 1, 1};
        CHECK_EQ(checkRatios(&counts[0]), true);
    }
    {
        int counts[] = {2, 2, 6, 2, 2};
        CHECK_EQ(checkRatios(&counts[0]), true);
    }
    {
        int counts[] = {3, 3, 10, 2, 3};
        CHECK_EQ(checkRatios(&counts[0]), true);
    }
    {
        int counts[] = {3, 1, 9, 3, 3};
        CHECK_EQ(checkRatios(&counts[0]), false);
    }
}

void test_computeCenters_simple_1()
{
    std::vector<cv::Rect> rects;
    rects.push_back(cv::Rect(0, 0, 3, 3));
    rects.push_back(cv::Rect(1, 1, 3, 3));

    std::vector<cv::Point> centers;
    computeCenters(rects, centers);

    CHECK_EQ(centers.size(), 1);
    CHECK_EQ(centers[0], cv::Point(2, 2));
}

void test_computeCenters_simple_2()
{
    std::vector<cv::Rect> rects;
    rects.push_back(cv::Rect(-1, -1, 3, 3));

    std::vector<cv::Point> centers;
    computeCenters(rects, centers);

    CHECK_EQ(centers.size(), 1);
    CHECK_EQ(centers[0], cv::Point(0, 0));
}

void test_computeCenters_hard()
{
    std::vector<cv::Rect> rects;
    //                      left top width height
    rects.push_back(cv::Rect(393, 185, 30, 30));
    rects.push_back(cv::Rect(393, 185, 31, 30));
    rects.push_back(cv::Rect(394, 184, 30, 31));
    rects.push_back(cv::Rect(395, 184, 30, 31));
    rects.push_back(cv::Rect(396, 184, 30, 31));
    rects.push_back(cv::Rect(394, 184, 31, 31));
    rects.push_back(cv::Rect(395, 184, 31, 31));
    rects.push_back(cv::Rect(304, 208, 31, 31));
    rects.push_back(cv::Rect(305, 208, 31, 31));
    rects.push_back(cv::Rect(306, 208, 31, 31));
    rects.push_back(cv::Rect(303, 208, 32, 31));
    rects.push_back(cv::Rect(304, 208, 32, 31));
    rects.push_back(cv::Rect(305, 208, 32, 31));
    rects.push_back(cv::Rect(306, 207, 32, 32));
    rects.push_back(cv::Rect(333, 297, 32, 31));
    rects.push_back(cv::Rect(334, 296, 32, 32));
    rects.push_back(cv::Rect(335, 296, 32, 32));

    std::vector<cv::Point> centers;
    computeCenters(rects, centers);

    CHECK_EQ(centers.size(), 3);
    CHECK_EQ(centers[0], cv::Point(409, 200));
    CHECK_EQ(centers[1], cv::Point(320, 223));
    CHECK_EQ(centers[2], cv::Point(350, 312));
}

void test_sortMarkers_1()
{
    std::vector<cv::Point> centers;
    //                         x   y
    centers.push_back(cv::Point(60, 60));
    centers.push_back(cv::Point(158, 60));
    centers.push_back(cv::Point(60, 158));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(60, 60));
    CHECK_EQ(topRight, cv::Point(158, 60));
    CHECK_EQ(bottomLeft, cv::Point(60, 158));
}

void test_sortMarkers_2()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(873, 360));
    centers.push_back(cv::Point(1347, 564));
    centers.push_back(cv::Point(663, 815));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(873, 360));
    CHECK_EQ(topRight, cv::Point(1347, 564));
    CHECK_EQ(bottomLeft, cv::Point(663, 815));
}

void test_sortMarkers_3()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(409, 200));
    centers.push_back(cv::Point(320, 223));
    centers.push_back(cv::Point(350, 312));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(320, 223));
    CHECK_EQ(topRight, cv::Point(409, 200));
    CHECK_EQ(bottomLeft, cv::Point(350, 312));
}

void test_sortMarkers_4()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(331, 226));
    centers.push_back(cv::Point(290, 309));
    centers.push_back(cv::Point(376, 346));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(290, 309));
    CHECK_EQ(topRight, cv::Point(331, 226));
    CHECK_EQ(bottomLeft, cv::Point(376, 346));
}

void test_sortMarkers_5()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(445, 199));
    centers.push_back(cv::Point(371, 257));
    centers.push_back(cv::Point(497, 274));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(445, 199));
    CHECK_EQ(topRight, cv::Point(497, 274));
    CHECK_EQ(bottomLeft, cv::Point(371, 257));
}

void test_sortMarkers_6()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(444, 270));
    centers.push_back(cv::Point(501, 344));
    centers.push_back(cv::Point(426, 397));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(501, 344));
    CHECK_EQ(topRight, cv::Point(426, 397));
    CHECK_EQ(bottomLeft, cv::Point(444, 270));
}

void test_sortMarkers_7()
{
    std::vector<cv::Point> centers;
    //                            x   y
    centers.push_back(cv::Point(308, 339));
    centers.push_back(cv::Point(434, 338));
    centers.push_back(cv::Point(371, 406));

    cv::Point topLeft, topRight, bottomLeft;
    sortMarkers(centers, topLeft, topRight, bottomLeft);

    CHECK_EQ(topLeft, cv::Point(371, 406));
    CHECK_EQ(topRight, cv::Point(308, 339));
    CHECK_EQ(bottomLeft, cv::Point(434, 338));
}

void test_decode()
{
#ifdef WIN32
    cv::Mat img = cv::imread("..\\qrcode.png");
#else
    cv::Mat img = cv::imread("../qrcode.png");
#endif
    cv::Mat gray, bin, mask;

    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, bin, 127, 255, cv::THRESH_BINARY);
    std::string msg = decode(bin, img, mask);

    CHECK_EQ(msg, "OpenCV");
}


bool runTests()
{
    bool passed = true;
    RUN_TEST(test_bgr2gray);
    RUN_TEST(test_gray2bin);
    RUN_TEST(test_countPixels_1);
    RUN_TEST(test_countPixels_2);
    RUN_TEST(test_checkRatios);
    RUN_TEST(test_computeCenters_simple_1);
    RUN_TEST(test_computeCenters_simple_2);
    RUN_TEST(test_computeCenters_hard);
    RUN_TEST(test_sortMarkers_1);
    RUN_TEST(test_sortMarkers_2);
    RUN_TEST(test_sortMarkers_3);
    RUN_TEST(test_sortMarkers_4);
    RUN_TEST(test_sortMarkers_5);
    RUN_TEST(test_sortMarkers_6);
    RUN_TEST(test_sortMarkers_7);
    RUN_TEST(test_decode);
    return passed;
}
