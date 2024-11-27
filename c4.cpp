#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fstream>
#include <string>

int main() {
    // Ðý?ng d?n ?nh t?i lên
    std::string image_path = "D:/Download/binary_image.png";
    std::string output_text_path = "D:/VSC/ticket_full_text.txt";

    // Ð?c ?nh
    cv::Mat image = cv::imread(image_path);

    // Chuy?n sang m?c xám
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Tãng cý?ng ?nh (binarize và tãng ð? týõng ph?n)
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Reload the binary image provided by the user
    std::string binary_image_path_updated = "D:/Download/binary_image.png";

    // Load the binary image
    cv::Mat binary_image_updated = cv::imread(binary_image_path_updated, cv::IMREAD_GRAYSCALE);

    // Perform OCR on the updated binary image with improved configurations
    tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
    ocr->Init(NULL, "por", tesseract::OEM_LSTM_ONLY);
    ocr->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
    ocr->SetImage(binary_image_updated.data, binary_image_updated.cols, binary_image_updated.rows, 1, binary_image_updated.step);

    std::string full_text_updated = std::string(ocr->GetUTF8Text());

    // Save the updated full text to a file
    std::string updated_output_text_path = "D:/VSC/ticket_full_text.txt";
    std::ofstream outFile(updated_output_text_path);
    if (outFile.is_open()) {
        outFile << full_text_updated;
        outFile.close();
    }

    // Clean up
    ocr->End();
    delete ocr;

    return 0;
}

