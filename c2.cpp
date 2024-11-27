#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;

Mat preprocess(const Mat& image) {
    Mat gray, binary;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    return binary;
}

vector<Rect> find_text_regions(const Mat& binary_image) {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(15, 5));
    Mat dilated;
    dilate(binary_image, dilated, kernel, Point(-1, -1), 2);

vector<vector<Point>> contours;
    findContours(dilated, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
// Mảng lưu trữ các khu vực chứa văn bản
vector<Rect> regions;
    for (const auto& contour : contours) {
        Rect rect = boundingRect(contour);
        int x = rect.x, y = rect.y, w = rect.width, h = rect.height;
        if (100 < w && w < 2000 && 30 < h && h < 200) {
            regions.push_back(rect);
        }
    }

   sort(regions.begin(), regions.end(), [](const Rect& a, const Rect& b) {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
        });

    return regions;
}

void crop_and_display(const Mat& binary_image, const vector<Rect>& regions, int target_segments = 5) {
    int count = 0;
    for (const auto& rect : regions) {
        if (count >= target_segments) break;

        Mat cropped = binary_image(rect);
        bitwise_not(cropped, cropped);

        int text_area = countNonZero(cropped);
        if (text_area > 500) {
            Mat bordered, resized;
            copyMakeBorder(cropped, bordered, 10, 10, 10, 10, BORDER_CONSTANT, Scalar(0));
            resize(bordered, resized, Size(), 2, 2, INTER_LINEAR);
            imshow("Cropped text " + to_string(count + 1), resized);
            count++;
        }
    }

    waitKey(0);
    destroyAllWindows();
}

int main() {
    Mat image = imread("D:/Download/Chapter_10/ticket.png");
    if (image.empty()) {
        cout << "Không tìm thấy ảnh, hãy kiểm tra lại đường dẫn." << std::endl;
        return -1;
    }

    Mat binary_image = preprocess(image);
    vector<Rect> text_regions = find_text_regions(binary_image);
    crop_and_display(binary_image, text_regions, 5);

    return 0;
}

