#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "board_detector.h"
#include "property.h"
#include "functions.h"

namespace fs = std::filesystem;

keymap options = {
    {"help", "0"},
    {"input", "."},
    {"output", "."},
    {"format", "png"},
    {"treshold", "100"},
    {"blur_size", "11"},
    {"contour_delta", "5"},
    {"gap", "50"},
    {"min_board_size", "200"},
    {"max_board_size", "1000"},
    {"thickness", "3"},
    {"remove_non_squares", "1"},
    {"blur_standard", "1"},
    {"blur_median", "1"},
    {"blur_gaussian", "1"},
};

int main(int argc, char **argv)
{
    if (fs::exists("config.ini"))
    {
        mc::properties props;
        props.load_properties("config.ini");
        for (auto &option : options)
        {
            if (props.has_property(option.first))
            {
                option.second = props.get_property(option.first);
            }
        }
    }

    build_keys(options, argc, argv);

    print_options(options);
    if (options["help"] == "1")
    {
        return 0;
    }

    fs::path inputDir = options["input"];
    fs::path outputDir = options["output"];

    if (!fs::exists(inputDir))
    {
        std::cout << "[error] input directory " << inputDir << " is missing, exit" << std::endl;
        return 0;
    }

    if (!fs::exists(outputDir))
    {
        std::cout << "[error] output directory " << outputDir << " is missing, will be create" << std::endl;
        fs::create_directory(outputDir);
    }

    BoardDetector detector;

    // read files from input directory
    std::vector<fs::path> images;
    std::string ext = std::string(".") + options["format"];

    for (const auto &entry : fs::directory_iterator(inputDir))
    {
        if (entry.path().extension().string() == ext)
        {
            images.push_back(entry.path().string());
        }
    }

    std::cout << "images: " << images.size() << std::endl;

    // for each image:
    //      extract boards
    //      save boards to output
    for (const auto &image : images)
    {
        std::cout << "parse page: " << image << std::endl;
        cv::Mat page = cv::imread(image.string());

        auto result = detector.getBoardContours(image.string());
        for (int i = 0; i < result.size(); ++i)
        {
            cv::rectangle(page, result[i], cv::Scalar(0, 0, 192), std::stoi(options["thickness"]));
            std::cout << "[info] point = ( "
                      << result[i].x << ", " << result[i].y
                      << " ), size = ( " << result[i].width << ", " << result[i].height << " )"
                      << std::endl;
        }
        auto path = fs::path(outputDir);
        path.append(image.filename().string());
        cv::imwrite(path.string(), page);
        std::cout << "diagrams detected: " << result.size() << std::endl;
    }

    return 0;
}
