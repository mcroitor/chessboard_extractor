#ifndef _BOARD_DETECTOR_H_
#define _BOARD_DETECTOR_H_

#include <vector>
#include <string>
#include <map>

#include <opencv2/core.hpp>

#include "functions.h"

typedef cv::Mat Board;
typedef std::vector<cv::Rect> rects_t;
typedef std::vector<std::vector<cv::Point>> contours_t;

class BoardDetector
{
public:
    BoardDetector();
    std::vector<Board> scanPage(std::string pathToPageImage);
    static contours_t getContours(const cv::Mat &mat);
    static rects_t getBoundingRects(const contours_t &contours);
    static rects_t removeNestedRects(const rects_t &rects);
};

#endif