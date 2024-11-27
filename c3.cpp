#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;
using namespace tesseract;

Mat binarize(const Mat& input_image) {
    Mat gray_image, binary_image;
    cvtColor(input_image, gray_image, COLOR_BGR2GRAY);
    threshold(gray_image, binary_image, 0, 255, THRESH_BINARY | THRESH_OTSU);

    int white = countNonZero(binary_image);
    int black = binary_image.total() - white;

    return (white < black) ? binary_image : ~binary_image;
}

vector<RotatedRect> find_text_areas(const Mat& input_image) {
    Mat dilated;
    Mat kernel = getStructuringElement(MORPH_CROSS, Size(3, 3));
    dilate(input_image, dilated, kernel, Point(-1, -1), 5);
    imshow("Dilated", dilated);
    waitKey(0);

    vector<vector<Point>> contours;
    findContours(dilated, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<RotatedRect> areas;
    for (const auto& contour : contours) {
        RotatedRect box = minAreaRect(contour);

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

Mat deskew_and_crop(const Mat& input_image, const RotatedRect& box) {
    float angle = box.angle;
    Size2f size = box.size;

    if (angle < -45.0) {
        angle += 90.0;
        swap(size.width, size.height);
    }

    Mat transform = getRotationMatrix2D(box.center, angle, 1.0);
    Mat rotated;
    warpAffine(input_image, rotated, transform, input_image.size(), INTER_CUBIC);

    Mat cropped;
    getRectSubPix(rotated, size, box.center, cropped);
    copyMakeBorder(cropped, cropped, 10, 10, 10, 10, BORDER_CONSTANT, Scalar(0, 0, 0));
    return cropped;
}

string identify_text(const Mat& input_image, const string& language = "eng") {
    TessBaseAPI tess;
    if (tess.Init(NULL, language.c_str(), OEM_LSTM_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        return "";
    }

    tess.SetPageSegMode(PSM_SINGLE_BLOCK);
    tess.SetImage(input_image.data, input_image.cols, input_image.rows, 3, input_image.step);

    string text = tess.GetUTF8Text();
    scout << "Text:" << endl << text << endl;

    tess.End();
    return text;
}

int main() {
    Mat ticket = binarize(imread("D:/Download/Chapter_10/ticket.png"));
    vector<RotatedRect> regions = find_text_areas(ticket);

    ofstream file("ticket_full_text.txt");
    if (!file.is_open()) {
        cerr << "Could not open file for writing." << endl;
        return 1;
    }

    for (const auto& region : regions) {
        Mat cropped = deskew_and_crop(ticket, region);
        string text = identify_text(cropped, "por");
        file << text << "\n";
    }

    file.close();
    return 0;
}

