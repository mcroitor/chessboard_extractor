#include <algorithm>
#include <iostream>
#include <map>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "board_detector.h"
#include "functions.h"

extern keymap options;

/**
 * @brief calculate if (left == right) with aproximation
 *
 * @param left
 * @param right
 * @param delta
 * @return true
 * @return false
 */
bool aprox_equal(size_t left, size_t right, size_t delta = 0)
{
    size_t eps = delta ? delta : (left + right) / 20; // delta or 5% from size
    return abs((int)(left - right)) < eps;
}

/**
 * @brief calculate if rect is square with aproximation
 *
 * @param rect
 * @param delta
 * @return true
 * @return false
 */
bool aprox_square(cv::Rect rect, size_t delta = 0)
{
    return aprox_equal(rect.width, rect.height, delta);
}

/**
 * @brief calculate if 2 rects are equal with approximation
 *
 * @param rect1
 * @param rect2
 * @param delta
 * @return true
 * @return false
 */
bool aprox_same_rect(cv::Rect rect1, cv::Rect rect2, size_t delta = 0)
{
    size_t gap = delta ? delta : std::stoi(options["gap"]);
    return aprox_equal(rect1.x, rect2.x, gap) &&
           aprox_equal(rect1.y, rect2.y, gap) &&
           aprox_equal(rect1.width, rect2.width, gap) &&
           aprox_equal(rect1.height, rect2.height, gap);
}

// methods

BoardDetector::BoardDetector() {}

contours_t BoardDetector::getContours(const cv::Mat &mat)
{
    /// Find contours
    cv::Mat mat_prepared;
    size_t blur_size = std::stoi(options["blur_size"]);
    cv::cvtColor(mat, mat_prepared, cv::COLOR_BGR2GRAY);
    if (options["blur_gaussian"] == "1")
        cv::GaussianBlur(mat_prepared, mat_prepared, cv::Size(blur_size, blur_size), 0);
    if (options["blur_median"] == "1")
        cv::medianBlur(mat_prepared, mat_prepared, blur_size);
    if (options["blur_standard"] == "1")
        cv::blur(mat_prepared, mat_prepared, cv::Size(blur_size, blur_size));

    cv::Mat canny_output;
    size_t treshold = std::stoi(options["treshold"]);
    cv::Canny(mat_prepared, canny_output, treshold, treshold * 2, 5);

    contours_t contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

rects_t BoardDetector::getBoundingRects(const contours_t &contours)
{
    rects_t result;
    size_t contour_delta = std::stoi(options["contour_delta"]);

    for (auto contour : contours)
    {
        std::vector<cv::Point> contour_poly;
        approxPolyDP(contour, contour_poly, contour_delta, true);
        cv::Rect bounding = cv::boundingRect(contour_poly);
        result.push_back(bounding);
    }
    return result;
}

rects_t BoardDetector::getBoardContours(std::string pathToPageImage)
{
    // load page image
    cv::Mat page = cv::imread(pathToPageImage);

    contours_t contours = BoardDetector::getContours(page);
    rects_t boards = BoardDetector::getBoundingRects(contours);
    // remove small pieces
    boards = BoardDetector::removeSmallRects(boards);
    boards = BoardDetector::optimize(boards);

    // remove non-square bounds
    if (options["remove_non_squares"] != "0")
    {
        boards = BoardDetector::removeNonSquares(boards);
    }

    return boards;
}

std::vector<Board> BoardDetector::scanPage(std::string pathToPageImage)
{
    std::vector<Board> result;
    cv::Mat page = cv::imread(pathToPageImage);

    auto boards = BoardDetector::getBoardContours(pathToPageImage);

    // extract diagrams from page
    for (auto board : boards)
    {
        result.push_back(page(board));
    }

    return result;
}

bool contains(const cv::Rect &outer, const cv::Rect &inner)
{
    size_t gap = std::stoi(options["gap"]);
    cv::Size gap_size(gap, gap);
    cv::Point gap_point(gap / 2, gap / 2);
    auto tmp = outer + gap_size;
    tmp -= gap_point;
    return (inner & tmp) == inner;
    // return outer.x - gap < inner.x &&
    //        outer.y - gap < inner.y &&
    //        outer.x + gap + outer.width > inner.x + inner.width &&
    //        outer.y + gap + outer.height > inner.y + inner.height;
}

rects_t BoardDetector::removeNestedRects(const rects_t &rects)
{
    rects_t result(rects);
    auto endIter = std::unique(result.begin(), result.end(), contains);
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::removeSmallRects(const rects_t &rects)
{
    rects_t result(rects);
    auto endIter = std::remove_if(
        result.begin(),
        result.end(),
        [](cv::Rect rect)
        {
            return rect.width < stoi(options["min_board_size"]) ||
                   rect.width > stoi(options["max_board_size"]);
        });
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::removeNonSquares(const rects_t &rects)
{
    rects_t result(rects);
    auto endIter = std::remove_if(
        result.begin(),
        result.end(),
        [](cv::Rect rect)
        {
            size_t gap = std::stoi(options["gap"]);
            return !aprox_square(rect, gap);
        });
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::unique(const rects_t &rects, size_t same_gap)
{
    rects_t result(rects);
    auto endIter = std::unique(
        result.begin(),
        result.end(),
        [same_gap](const cv::Rect &left, const cv::Rect &right)
        {
            return left == right; // aprox_same_rect(left, right, same_gap);
        });
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::optimize(const rects_t &rects)
{
    rects_t result;
    for (auto x : rects)
    {
        auto rect = x;
        for (auto y : rects)
        {
            if ((rect & y).area() != 0)
            {
                rect |= y;
            }
        }
        if (std::find(result.begin(), result.end(), rect) == result.end())
        {
            result.push_back(rect);
        }
    }
    return result;
}