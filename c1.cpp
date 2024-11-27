#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

using namespace std;
using namespace cv;

// Hàm chuyển ảnh sang trắng đen
Mat binarize(Mat input) {
    Mat gray, binaryImage;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    threshold(gray, binaryImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
    return binaryImage;
}

// Hàm tìm các vùng văn bản
vector<RotatedRect> findTextAreas(Mat input) {
    Mat dilated;
    auto kernel = getStructuringElement(MORPH_CROSS, Size(3, 3));
    dilate(input, dilated, kernel, Point(-1, -1), 5);

    vector<vector<Point>> contours;
    findContours(dilated, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<RotatedRect> areas;
    for (const auto& contour : contours) {
        RotatedRect box = minAreaRect(contour);
        if (box.size.width < 20 || box.size.height < 20) continue;

        double proportion = max(box.size.width, box.size.height) / min(box.size.width, box.size.height);
        if (proportion < 2) continue;
        areas.push_back(box);
    }
    return areas;
}

// Hàm căn chỉnh và cắt vùng văn bản
Mat deskewAndCrop(Mat input, const RotatedRect& box) {
    double angle = box.angle;
    auto size = box.size;

    if (angle < -45.0) {
        angle += 90.0;
        swap(size.width, size.height);
    }

    Mat rotationMatrix = getRotationMatrix2D(box.center, angle, 1.0);
    Mat rotated;
    warpAffine(input, rotated, rotationMatrix, input.size(), INTER_CUBIC);

    Mat cropped;
    getRectSubPix(rotated, size, box.center, cropped);
    return cropped;
}

int main() {
    // Đọc ảnh
    Mat input = imread("D:/Download/Chapter_10/ticket.png");
    if (input.empty()) {
        cout << "Không tìm thấy file ảnh!" << endl;
        return -1;
    }
    cout << "Ảnh đã tải thành công!" << endl;

    // Chuyển trắng đen
    Mat binaryImage = binarize(input);
    imshow("Binary Image", binaryImage); // Kiểm tra ảnh trắng đen
    waitKey(0);

    // Tìm các vùng văn bản
    vector<RotatedRect> textAreas = findTextAreas(binaryImage);
    cout << "Tìm thấy " << textAreas.size() << " vùng văn bản." << endl;

    // Xử lý từng vùng văn bản
    for (size_t i = 0; i < textAreas.size(); ++i) {
        Mat cropped = deskewAndCrop(binaryImage, textAreas[i]);
        imshow("Cropped Region " + to_string(i + 1), cropped);
        waitKey(0);
    }

    return 0;
}
