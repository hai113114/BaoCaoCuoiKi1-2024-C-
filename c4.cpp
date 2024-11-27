#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fstream>
#include <string>

using namespace cv;
using namespace std;
using namespace tesseract;

int main() {
    // Đường dẫn ảnh và tệp văn bản
    string image_path = "D:/Download/binary_image.png";
    string output_text_path = "D:/VSC/ticket_full_text.txt";

    // Đọc ảnh và chuyển sang ảnh xám
    Mat image = imread(image_path, IMREAD_GRAYSCALE);

    // Tăng cường ảnh (chuyển sang ảnh nhị phân và tăng độ tương phản)
    Mat binary;
    threshold(image, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);

    // Thực hiện OCR trên ảnh nhị phân với cấu hình cải tiến
    TessBaseAPI ocr;
    if (ocr.Init(NULL, "por", OEM_LSTM_ONLY) != 0) {
        cerr << "Không thể khởi tạo Tesseract OCR!" << endl;
        return 1;
    }

    ocr.SetPageSegMode(PSM_SINGLE_BLOCK);
    ocr.SetImage(binary.data, binary.cols, binary.rows, 1, binary.step);

    // Nhận diện văn bản
    string full_text = ocr.GetUTF8Text();

    // Lưu văn bản nhận dạng vào tệp
    ofstream outFile(output_text_path);
    if (outFile.is_open()) {
        outFile << full_text;
    } else {
        cerr << "Không thể mở tệp để ghi!" << endl;
        return 1;
    }

    return 0;
}
