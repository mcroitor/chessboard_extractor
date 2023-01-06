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

bool has_fields(const cv::Rect &rect, const cv::Mat &part)
{
    size_t field_size = rect.width / 8;
    size_t field_gap = 10; // stoi(options["gap"]) / 8;

    contours_t contours = BoardDetector::getContours(part);
    // create bounders
    rects_t rects = BoardDetector::getBoundingRects(contours);
    auto endIter = std::remove_if(
        rects.begin(),
        rects.end(),
        [field_size, field_gap](const cv::Rect &rect)
        {
            // return !aprox_equal(rect.width, field_size, field_gap);
            return !aprox_square(rect, field_gap) ||
                   !aprox_equal(rect.width, field_size, field_gap);
        });
    rects.erase(endIter, rects.end());

    endIter = std::unique(
        rects.begin(),
        rects.end(),
        [field_gap](const cv::Rect &left, const cv::Rect &right)
        {
            return aprox_same_rect(left, right, field_gap);
        });
    rects.erase(endIter, rects.end());

    // filter contours: is_square, size is aprox average / 8
    // if total contours == 64, is ok
    std::cout << "[debug] + + fields = " << rects.size() << std::endl;
    std::cout << "[debug] board size = " << rect.width << std::endl;
    std::cout << "[debug] field size = " << rects[0].width << std::endl;
    return true; // rects.size() == 64;
}

bool is_board(const cv::Rect &rect, const cv::Mat &part)
{
    int min_board_size = std::stoi(options["min_board_size"]);
    return aprox_square(rect) && rect.width > min_board_size && has_fields(rect, part);
}

BoardDetector::BoardDetector() {}

contours_t BoardDetector::getContours(const cv::Mat &mat)
{
    /// Find contours
    cv::Mat mat_prepared;
    cv::cvtColor(mat, mat_prepared, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(mat_prepared, mat_prepared, cv::Size(5, 5), 0);

    cv::Mat canny_output;
    cv::Canny(mat_prepared, canny_output, std::stoi(options["treshold"]), std::stoi(options["treshold"]) * 2);

    contours_t contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

rects_t BoardDetector::getBoundingRects(const contours_t &contours)
{
    rects_t result;
    for (auto contour : contours)
    {
        std::vector<cv::Point> contour_poly;
        approxPolyDP(contour, contour_poly, 3, true);
        cv::Rect bounding = cv::boundingRect(contour_poly);
        result.push_back(bounding);
    }
    return result;
}

std::vector<Board> BoardDetector::scanPage(std::string pathToPageImage)
{
    std::vector<Board> result;
    // load page image
    cv::Mat page = cv::imread(pathToPageImage);

    contours_t contours = BoardDetector::getContours(page);
    rects_t boards = BoardDetector::getBoundingRects(contours);
    // remove small pieces
    auto endIter = std::remove_if(
        boards.begin(),
        boards.end(),
        [](cv::Rect rect)
        { 
            return rect.width < stoi(options["min_board_size"]); 
            });
    boards.erase(endIter, boards.end());

    // remove non-square bounds
    endIter = std::remove_if(
        boards.begin(),
        boards.end(),
        [](cv::Rect rect)
        { 
            return !aprox_square(rect); 
            });
    boards.erase(endIter, boards.end());


//    // remove non-board
//    endIter = std::remove_if(
//        boards.begin(),
//        boards.end(),
//        [page](cv::Rect rect)
//        { return !is_board(rect, page(rect)); });
//    boards.erase(endIter, boards.end());

    // remove nested
    boards = BoardDetector::removeNestedRects(boards);

    // remove same boundings
    endIter = std::unique(
        boards.begin(),
        boards.end(),
        [](const cv::Rect &left, const cv::Rect &right)
        {
            return aprox_same_rect(left, right);
        });
    boards.erase(endIter, boards.end());


    // extract diagrams from page
    for (auto board : boards)
    {
        result.push_back(page(board));
    }

    return result;
}

bool contains(const cv::Rect &outer, const cv::Rect &inner)
{
    return outer.x < inner.x &&
           outer.y < inner.y &&
           outer.x + outer.width > inner.x + inner.width &&
           outer.y + outer.height > inner.y + inner.height;
}

rects_t BoardDetector::removeNestedRects(const rects_t &rects)
{
    rects_t result(rects);
    auto endIter = std::unique(result.begin(), result.end(), contains);
    result.erase(endIter, result.end());
    return result;
}