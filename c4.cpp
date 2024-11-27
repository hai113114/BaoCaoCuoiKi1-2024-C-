#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fstream>
#include <string>

using namespace cv;
using namespace std;
using namespace tesseract;

int main() {
    string image_path = "D:/Download/binary_image.png";
    string output_text_path = "D:/VSC/ticket_full_text.txt";
    Mat image = imread(image_path, IMREAD_GRAYSCALE);
    Mat binary;
    threshold(image, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
    TessBaseAPI ocr;
    if (ocr.Init(NULL, "por", OEM_LSTM_ONLY) != 0) {
        cerr << "Không thể khởi tạo Tesseract OCR!" << endl;
        return 1;
    }

    ocr.SetPageSegMode(PSM_SINGLE_BLOCK);
    ocr.SetImage(binary.data, binary.cols, binary.rows, 1, binary.step);
    string full_text = ocr.GetUTF8Text();

    ofstream outFile(output_text_path);
    if (outFile.is_open()) {
        outFile << full_text;
    } else {
        cerr << "Không thể mở tệp để ghi!" << endl;
        return 1;
    }

    return 0;
}
