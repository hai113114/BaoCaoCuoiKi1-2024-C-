#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <fstream>
#include <vector>

cv::Mat binarize(const cv::Mat& input_image) {
    cv::Mat gray_image, binary_image;
    cv::cvtColor(input_image, gray_image, cv::COLOR_BGR2GRAY);
    cv::threshold(gray_image, binary_image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    int white = cv::countNonZero(binary_image);
    int black = binary_image.total() - white;

    return (white < black) ? binary_image : ~binary_image;
}

std::vector<cv::RotatedRect> find_text_areas(const cv::Mat& input_image) {
    cv::Mat dilated;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::dilate(input_image, dilated, kernel, cv::Point(-1, -1), 5);
    cv::imshow("Dilated", dilated);
    cv::waitKey(0);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::RotatedRect> areas;
    for (const auto& contour : contours) {
        cv::RotatedRect box = cv::minAreaRect(contour);

        if (box.size.width < 20 || box.size.height < 20) {
            continue;
        }

        float proportion = (box.size.width > box.size.height) ?
            (box.size.height / box.size.width) :
            (box.size.width / box.size.height);
        if (proportion < 2) {
            continue;
        }

        areas.push_back(box);
    }
    return areas;
}

cv::Mat deskew_and_crop(const cv::Mat& input_image, const cv::RotatedRect& box) {
    float angle = box.angle;
    cv::Size2f size = box.size;

    if (angle < -45.0) {
        angle += 90.0;
        std::swap(size.width, size.height);
    }

    cv::Mat transform = cv::getRotationMatrix2D(box.center, angle, 1.0);
    cv::Mat rotated;
    cv::warpAffine(input_image, rotated, transform, input_image.size(), cv::INTER_CUBIC);

    cv::Mat cropped;
    cv::getRectSubPix(rotated, size, box.center, cropped);
    cv::copyMakeBorder(cropped, cropped, 10, 10, 10, 10, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    return cropped;
}

std::string identify_text(const cv::Mat& input_image, const std::string& language = "eng") {
    tesseract::TessBaseAPI tess;
    if (tess.Init(NULL, language.c_str(), tesseract::OEM_LSTM_ONLY)) {
        std::cerr << "Could not initialize tesseract." << std::endl;
        return "";
    }

    tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
    tess.SetImage(input_image.data, input_image.cols, input_image.rows, 3, input_image.step);

    std::string text = tess.GetUTF8Text();
    std::cout << "Text:" << std::endl << text << std::endl;

    tess.End();
    return text;
}

int main() {
    cv::Mat ticket = binarize(cv::imread("D:/Download/Chapter_10/ticket.png"));
    std::vector<cv::RotatedRect> regions = find_text_areas(ticket);

    std::ofstream file("ticket_full_text.txt");
    if (!file.is_open()) {
        std::cerr << "Could not open file for writing." << std::endl;
        return 1;
    }

    for (const auto& region : regions) {
        cv::Mat cropped = deskew_and_crop(ticket, region);
        std::string text = identify_text(cropped, "por");
        file << text << "\n";
    }

    file.close();
    return 0;
}

