#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

cv::Mat preprocess(const cv::Mat& image) {
    cv::Mat gray, binary;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    return binary;
}

std::vector<cv::Rect> find_text_regions(const cv::Mat& binary_image) {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 5));
    cv::Mat dilated;
    cv::dilate(binary_image, dilated, kernel, cv::Point(-1, -1), 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> regions;
    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        int x = rect.x, y = rect.y, w = rect.width, h = rect.height;
        if (100 < w && w < 2000 && 30 < h && h < 200) {
            regions.push_back(rect);
        }
    }

    std::sort(regions.begin(), regions.end(), [](const cv::Rect& a, const cv::Rect& b) {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
        });

    return regions;
}

void crop_and_display(const cv::Mat& binary_image, const std::vector<cv::Rect>& regions, int target_segments = 5) {
    int count = 0;
    for (const auto& rect : regions) {
        if (count >= target_segments) break;

        cv::Mat cropped = binary_image(rect);
        cv::bitwise_not(cropped, cropped);

        int text_area = cv::countNonZero(cropped);
        if (text_area > 500) {
            cv::Mat bordered, resized;
            cv::copyMakeBorder(cropped, bordered, 10, 10, 10, 10, cv::BORDER_CONSTANT, cv::Scalar(0));
            cv::resize(bordered, resized, cv::Size(), 2, 2, cv::INTER_LINEAR);
            cv::imshow("Cropped text " + std::to_string(count + 1), resized);
            count++;
        }
    }

    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main() {
    cv::Mat image = cv::imread("D:/Download/Chapter_10/ticket.png");
    if (image.empty()) {
        std::cout << "Không tìm thấy ảnh, hãy kiểm tra lại đường dẫn." << std::endl;
        return -1;
    }

    cv::Mat binary_image = preprocess(image);
    std::vector<cv::Rect> text_regions = find_text_regions(binary_image);
    crop_and_display(binary_image, text_regions, 5);

    return 0;
}

