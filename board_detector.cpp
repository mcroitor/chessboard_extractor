#include <algorithm>
#include <iostream>
#include <map>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "board_detector.h"
#include "functions.h"

extern keymap options;

int option_int(const std::string &key, int defaultValue)
{
    auto it = options.find(key);
    if (it == options.end() || it->second.empty())
    {
        return defaultValue;
    }

    try
    {
        return std::stoi(it->second);
    }
    catch (...)
    {
        return defaultValue;
    }
}

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
    size_t diff = left > right ? left - right : right - left;
    return diff < eps;
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

contours_t BoardDetector::getContours(const cv::Mat& mat)
{
    if (mat.empty())
    {
        return {};
    }

    /// Find contours
    cv::Mat mat_prepared;
    size_t blur_size = option_int("blur_size", 11);
    if (blur_size == 0)
    {
        blur_size = 1;
    }
    // Median and Gaussian blur require odd kernel size.
    if (blur_size % 2 == 0)
    {
        ++blur_size;
    }

    cv::cvtColor(mat, mat_prepared, cv::COLOR_BGR2GRAY);
    if (options["blur_gaussian"] == "1")
        cv::GaussianBlur(mat_prepared, mat_prepared, cv::Size(blur_size, blur_size), 0);
    if (options["blur_median"] == "1")
        cv::medianBlur(mat_prepared, mat_prepared, blur_size);
    if (options["blur_standard"] == "1")
        cv::blur(mat_prepared, mat_prepared, cv::Size(blur_size, blur_size));

    cv::Mat canny_output;
    size_t treshold = option_int("treshold", 100);
    cv::Canny(mat_prepared, canny_output, treshold, treshold * 2, 5);

    // Optional edge-gap closing helps connect broken board frames on low-quality scans.
    if (option_int("close_gaps", 1) != 0)
    {
        int kernelSize = option_int("morph_kernel_size", 5);
        if (kernelSize < 1)
        {
            kernelSize = 1;
        }
        if (kernelSize % 2 == 0)
        {
            ++kernelSize;
        }

        int iterations = option_int("morph_iterations", 1);
        if (iterations < 1)
        {
            iterations = 1;
        }

        cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(kernelSize, kernelSize));
        cv::morphologyEx(
            canny_output,
            canny_output,
            cv::MORPH_CLOSE,
            kernel,
            cv::Point(-1, -1),
            iterations);
    }

    contours_t contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

rects_t BoardDetector::getBoundingRects(const contours_t& contours)
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
    if (page.empty())
    {
        std::cerr << "[error] can not read page image: " << pathToPageImage << std::endl;
        return {};
    }

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
    if (page.empty())
    {
        std::cerr << "[error] can not read page image: " << pathToPageImage << std::endl;
        return result;
    }

    auto boards = BoardDetector::getBoardContours(pathToPageImage);
    cv::Rect imageBounds(0, 0, page.cols, page.rows);

    // extract diagrams from page
    for (auto board : boards)
    {
        cv::Rect clipped = board & imageBounds;
        if (clipped.area() > 0)
        {
            result.push_back(page(clipped));
        }
    }

    return result;
}

bool contains(const cv::Rect& outer, const cv::Rect& inner)
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

rects_t BoardDetector::removeNestedRects(const rects_t& rects)
{
    rects_t result;
    result.reserve(rects.size());

    for (size_t i = 0; i < rects.size(); ++i)
    {
        bool isNested = false;
        for (size_t j = 0; j < rects.size(); ++j)
        {
            if (i == j)
            {
                continue;
            }
            if (contains(rects[j], rects[i]))
            {
                isNested = true;
                break;
            }
        }
        if (!isNested)
        {
            result.push_back(rects[i]);
        }
    }

    auto endIter = std::unique(result.begin(), result.end());
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::removeSmallRects(const rects_t& rects)
{
    rects_t result(rects);
    auto endIter = std::remove_if(
        result.begin(),
        result.end(),
        [](cv::Rect rect)
        {
            return rect.width < stoi(options["min_board_size"]) ||
                rect.width > stoi(options["max_board_size"]) ||
                rect.height < stoi(options["min_board_size"]) ||
                rect.height > stoi(options["max_board_size"]);
        });
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::removeNonSquares(const rects_t& rects)
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

rects_t BoardDetector::unique(const rects_t& rects, size_t same_gap)
{
    rects_t result(rects);
    auto endIter = std::unique(
        result.begin(),
        result.end(),
        [same_gap](const cv::Rect& left, const cv::Rect& right)
        {
            return left == right; // aprox_same_rect(left, right, same_gap);
        });
    result.erase(endIter, result.end());
    return result;
}

rects_t BoardDetector::optimize(const rects_t& rects)
{
    rects_t result;
    int mergeGap = option_int("merge_gap", 8);
    if (mergeGap < 0)
    {
        mergeGap = 0;
    }

    for (auto x : rects)
    {
        auto rect = x;
        bool changed = false;
        do
        {
            changed = false;
            for (auto y : rects)
            {
                cv::Rect expanded = rect + cv::Size(mergeGap, mergeGap);
                expanded -= cv::Point(mergeGap / 2, mergeGap / 2);

                if ((expanded & y).area() != 0)
                {
                    cv::Rect merged = rect | y;
                    if (merged != rect)
                    {
                        rect = merged;
                        changed = true;
                    }
                }
            }
        }
        while (changed);

        if (std::find(result.begin(), result.end(), rect) == result.end())
        {
            result.push_back(rect);
        }
    }
    return result;
}