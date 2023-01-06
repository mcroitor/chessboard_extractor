#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "board_detector.h"
#include "property.h"
#include "functions.h"

namespace fs = std::filesystem;

keymap options = {
    {"input", "."},
    {"output", "."},
    {"format", "png"},
    {"treshold", "100"},
    {"blur_size", "11"},
    {"contour_delta", "5"},
    {"gap", "50"},
    {"min_board_size", "200"},
    {"max_board_size", "1000"},
    {"remove_nested", "1"},
    {"remove_non_squares", "1"},
    {"remove_same", "1"}};

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
        auto result = detector.scanPage(image.string());
        for (int i = 0; i < result.size(); ++i)
        {
            auto path = fs::path(outputDir);
            path.append(image.filename().string() + "_brd_" + std::to_string(i) + "." + options["format"]);
            cv::imwrite(path.string(), result[i]);
        }
        std::cout << "diagrams detected: " << result.size() << std::endl;
    }

    return 0;
}
