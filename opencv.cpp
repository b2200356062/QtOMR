#include "opencv.h"

#include <QFileDialog>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

// #include <tesseract/baseapi.h>
// #include <leptonica/allheaders.h>

using namespace cv;
using namespace std;

OpenCV::OpenCV(QWidget *parent)
    : QWidget(parent)
{}

OpenCV::~OpenCV() {}

std::vector<cv::Point> centers;

// HELPER FUNCTIONS
cv::Mat QImage2Mat(const QImage &inImage, bool inCloneImageData = true) {
    switch (inImage.format()) {
    // 8-bit, 4 channel
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied: {
        cv::Mat mat(inImage.height(), inImage.width(),
                    CV_8UC4,
                    const_cast<uchar*>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        return (inCloneImageData ? mat.clone() : mat);
    }

        // 8-bit, 3 channel
    case QImage::Format_RGB32: {
        cv::Mat mat(inImage.height(), inImage.width(),
                    CV_8UC4,
                    const_cast<uchar*>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        cv::Mat matRGB;
        cv::cvtColor(mat, matRGB, cv::COLOR_BGRA2BGR);
        return (inCloneImageData ? matRGB.clone() : matRGB);
    }

    case QImage::Format_RGB888: {
        cv::Mat mat(inImage.height(), inImage.width(),
                    CV_8UC3,
                    const_cast<uchar*>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        return (inCloneImageData ? mat.clone() : mat);
    }

    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied: {
        cv::Mat mat(inImage.height(), inImage.width(),
                    CV_8UC4,
                    const_cast<uchar*>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        return (inCloneImageData ? mat.clone() : mat);
    }

        // 8-bit, 1 channel
    case QImage::Format_Indexed8: {
        cv::Mat mat(inImage.height(), inImage.width(),
                    CV_8UC1,
                    const_cast<uchar*>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        return (inCloneImageData ? mat.clone() : mat);
    }

    default:
        qWarning() << "QImage format not handled in switch:" << inImage.format();
        break;
    }

    return cv::Mat();
}

QImage matToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return image.copy();
    }
    else if (mat.type() == CV_8UC3)
    {   // premultiplied eklenmesi gerekebilir
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped().copy();
    }
    else if (mat.type() == CV_8UC4)
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qWarning() << "Unsupported cv::Mat type";
        return QImage();
    }
}

struct Point2fComparator
{
    bool operator()(const Point2f& a, const Point2f& b) const {
        return (a.x < b.x) || (a.x == b.x && a.y < b.y);
    }
};

Point2f computeIntersect(Vec4i a, Vec4i b)
{
    Point2f pt;
    float denom = (float)((a[0] - a[2]) * (b[1] - b[3]) - (a[1] - a[3]) * (b[0] - b[2]));
    pt.x = ((a[0] * a[3] - a[1] * a[2]) * (b[0] - b[2]) - (a[0] - a[2]) * (b[0] * b[3] - b[1] * b[2])) / denom;
    pt.y = ((a[0] * a[3] - a[1] * a[2]) * (b[1] - b[3]) - (a[1] - a[3]) * (b[0] * b[3] - b[1] * b[2])) / denom;
    return pt;
}

cv::Mat rotateImage(const cv::Mat &image, double angle, int width, int height)
{
    cv::Point2f center(image.cols / 2.0, image.rows / 2.0);

    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);

    cv::Rect2f bbox = cv::RotatedRect(center, image.size(), angle).boundingRect2f();

    rotationMatrix.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rotationMatrix.at<double>(1, 2) += bbox.height / 2.0 - center.y;

    cv::Mat rotatedImage;
    cv::warpAffine(image, rotatedImage, rotationMatrix, bbox.size());

    cv::Mat resizedImage;
    cv::resize(rotatedImage, resizedImage, cv::Size(width, height));

    return rotatedImage;
}

std::vector<cv::Mat> splitImage( cv::Mat & image, int M, int N, int part)
{
    int width  = 52; //image.cols / M;
    int height = 113; // image.rows / N;

    std::vector<cv::Mat> result;

    for( int i = 0; i < N; ++i )
    {
        for( int j = 0; j < M; ++j )
        {
            cv::Rect roi( width * j, height * i, width, height );
            result.push_back( image( roi ) );

            // highlight çemberlerinin merkezlerini bulmak
            Point center;
            if(part == 1)
            {
                center.x =  (roi.br().x + 60 + roi.tl().x + 60) * 0.5;
                center.y = (roi.br().y + 130 + roi.tl().y + 130) * 0.5;
            }
            else
            {
                center.x =  (roi.br().x + 445  + roi.tl().x + 445) * 0.5;
                center.y = (roi.br().y + 130 + roi.tl().y + 130) * 0.5;
            }

            centers.push_back(center);
        }
    }
    return result;
}

auto analyzeAnswers(std::vector<cv::Mat> answers, int sira)
{
    std::map<int, string> choices = {
         {0, "Bos"},
         {1, "A"},
         {2, "B"},
         {3, "C"},
         {4, "D"},
         {5, "E"},
         {6, "Hatali"},
         };

    std::map<int, string> answermap;
    std::map<int, vector<int>> soru;

    // soru vectorunu olusturma
    for( size_t i = 0; i < answers.size(); ++i )
    {
        int nonzeroes = cv::countNonZero(answers[i]);
        soru[(i / 5)].push_back(nonzeroes);
        //qDebug() << "nonzero " << i/5 << " " << nonzeroes << "\n";
    }

    for(size_t i = 0; i < soru.size(); i++)
    {
        std::string cevap;

        // seçilen şık
        int maxnonzero = *std::max_element(soru[i].begin(), soru[i].end());

        // threshold
        auto minimum_value = 2500;

        // aynı soruda birden fazla şık işaretlenmisse
        auto count = std::count_if(soru[i].begin(), soru[i].end(),[&](auto const& val){ return val >= minimum_value; });
        if(count > 1)
        {
            cevap = choices.at(6);
        }
        else
        {
            if(maxnonzero > 2500){
                // secilen şıkkın indexi
                int choice = std::distance(soru[i].begin(), std::max_element(soru[i].begin(), soru[i].end()));

                if(choice % 5 == 0){
                    cevap = choices.at(1);
                }
                else if(choice % 5 == 1){
                    cevap = choices.at(2);
                }
                else if(choice % 5 == 2){
                    cevap = choices.at(3);
                }
                else if(choice % 5 == 3){
                    cevap = choices.at(4);
                }
                else if(choice % 5 == 4){
                    cevap = choices.at(5);
                }
            }
            else  // boş cevap
            {
                cevap = choices.at(0);
            }
        }
        // 0. soru --> 1. soru
        // 2. kısım ise soru indexi 11den başlar
        if(sira == 1)
        {
            answermap.insert({ i+11 , cevap});
        }
        else
        {
            answermap.insert({ i+1 , cevap});
        }
    }
    return answermap;
}

auto readOptic(const cv::Mat &img)
{
    std::map<int, string> answermap;
    std::map<int, string> answermap2;

    cv::Mat firstpart;
    cv::Mat secondpart;

    // hardcoded
    cv::Rect cropfirstpart (60, 130, 270, 1130);
    cv::Rect cropsecondpart (445, 130, 270, 1130);

    firstpart = img(cropfirstpart);
    secondpart = img(cropsecondpart);

#ifdef Q_OS_WIN
    //imshow("firspart", firstpart);
    //imshow("secondpart", secondpart);
#endif

    std::vector<cv::Mat> firstanswers = splitImage(firstpart, 5 , 10, 1);
    std::vector<cv::Mat> secondanswers = splitImage(secondpart, 5 , 10, 2);

    answermap = analyzeAnswers(firstanswers, 0);
    answermap2 = analyzeAnswers(secondanswers, 1);

    answermap.insert(answermap2.begin(), answermap2.end());

    return answermap;
}

void readCredentials();

// sinyal verip slot olarak run calıstırılabilir
void OpenCV::run(const QImage &imgNew, int page)
{
    int widthImg = 720;
    int heightImg = 1280;

    cv::Mat img = QImage2Mat(imgNew);

    if (img.empty()) {
        qDebug() << "img is empty\n";
        return;
    }

    // preprocess
    Mat grayimg;
    cvtColor(img, grayimg, cv::COLOR_BGR2GRAY);

    // Mat scaledimg;
    // cv::resize(grayimg, scaledimg, Size(widthImg, heightImg));

    Mat blurredimg;
    GaussianBlur(grayimg, blurredimg, Size(7, 7), 5);

    Mat threshimg;
    adaptiveThreshold(blurredimg, threshimg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);

    Mat morphimg;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(threshimg, morphimg, MORPH_CLOSE, kernel);

    Mat canniedimg;
    Canny(morphimg, canniedimg, 200, 300);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(canniedimg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Mat drawing = Mat::zeros(canniedimg.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++) {
        drawContours(drawing, contours, (int)i, Scalar(0, 255, 0), 1);
    }

    vector<Point> largestQuad;
    double maxArea = 0;

    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > 2000) {
            double peri = arcLength(contour, true);
            vector<Point> approxed;

            double epsilon = 0.02 * peri;
            approxPolyDP(contour, approxed, epsilon, true);

            if (approxed.size() == 4 && isContourConvex(approxed)) {
                double aspectRatio = fabs(contourArea(approxed)) / (boundingRect(approxed).width * boundingRect(approxed).height);
                if (aspectRatio > 0.5 && aspectRatio < 2.0 && area > maxArea) {
                    largestQuad = approxed;
                    maxArea = area;
                }
            }
        }
    }

    if (largestQuad.empty()) {
        emit errorNotify();
        return;
    }

    auto sortCorners = [](vector<Point>& corners) {
        sort(corners.begin(), corners.end(), [](const Point& a, const Point& b) { return a.y < b.y; });
        if (corners[0].x > corners[1].x) swap(corners[0], corners[1]);
        if (corners[2].x < corners[3].x) swap(corners[2], corners[3]);
    };

    sortCorners(largestQuad);

    Point2f src[4];
    Point2f dst[4];

    for (int i = 0; i < 4; i++) {
        src[i] = largestQuad[i];
    }

    dst[0] = Point2f(0, 0);
    dst[1] = Point2f(widthImg, 0);
    dst[2] = Point2f(widthImg, heightImg);
    dst[3] = Point2f(0, heightImg);

    Mat perspectiveMat = getPerspectiveTransform(src, dst);
    Mat perspectiveimg;

    warpPerspective(grayimg, perspectiveimg, perspectiveMat, Size(widthImg, heightImg));

    Mat thresh2;
    threshold(perspectiveimg, thresh2, 150, 255, THRESH_BINARY_INV + THRESH_OTSU);

    std::map<int, char> cevapdogrumu;
    std::map<char, char> neolmaliydi;

    // cevap kağıdını kaydet
    if(page == 1 || page == 2)
    {
        cevapKagidi = readOptic(thresh2);
    }
    else
    {
        // sınavın cevaplarını kaydet
        answers = readOptic(thresh2);
        int j = 1;

        for(const auto &i : answers)
        {
            if(i.second == cevapKagidi[j] && i.second != "Bos" && i.second != "Hatali")
            {
                dogru++;
                cevapdogrumu[j] = 'd';
            }
            else if(i.second == "Bos")
            {
                bos++;
                cevapdogrumu[j] = 'b';
            }
            else if(i.second == "Hatali")
            {
                yanlis++;
                cevapdogrumu[j] = 'h';
            }
            else
            {
                yanlis++;
                cevapdogrumu[j] = 'y';
            }
            j++;
        }
    }

    net = dogru - (yanlis * 0.25);

    this->setDogru(dogru);
    this->setYanlis(yanlis);
    this->setBos(bos);
    this->setNet(net);

    cv::cvtColor(perspectiveimg, perspectiveimg, cv::COLOR_GRAY2BGR);  // BLUE GREEN RED

    // görsel olarak yanlış ve doğruları gösterme
    int k = 0;
    for(size_t i = 1; i < answers.size()+1; i++) // index 1den baslıyor çünkü key 1den baslıyor
    {
        if(cevapdogrumu[i] == 'd')
        {
            if(answers[i] == "A"){
                cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,255,0), 3);
            }
            if(answers[i] == "B"){
                cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
            }
            if(answers[i] == "C"){
                cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
            }
            if(answers[i] == "D"){
                cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,255,0), 3);
            }
            if(answers[i] == "E"){
                cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,255,0), 3);
            }
        }

        else if(cevapdogrumu[i] == 'y')
        {
            if(answers[i] == "A")
            {
                // yanlışın pozisyonu
                cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,0,255), 3);

                if(cevapKagidi[i] == "B"){
                    cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "C"){
                    cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "D"){
                    cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "E"){
                    cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,255,0), 3);
                }
            }
            if(answers[i] == "B"){

                cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,0,255), 3);

                if(cevapKagidi[i] == "A"){
                    cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "C"){
                    cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "D"){
                    cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25,Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "E"){
                    cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25,Scalar(0,255,0), 3);
                }
            }
            if(answers[i] == "C"){

                cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,0,255), 3);

                if(cevapKagidi[i] == "A"){
                    cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25,Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "B"){
                    cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "D"){
                    cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "E"){
                    cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,255,0), 3);
                }
            }
            if(answers[i] == "D"){

                cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,0,255), 3);

                if(cevapKagidi[i] == "A"){
                    cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "B"){
                    cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "C"){
                    cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "E"){
                    cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,255,0), 3);
                }
            }
            if(answers[i] == "E"){

                cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,0,255), 3);

                if(cevapKagidi[i] == "A"){
                    cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "B"){
                    cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "C"){
                    cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
                }
                else if(cevapKagidi[i] == "D"){
                    cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,255,0), 3);
                }
            }
        }
        else   // birden fazla işaretlenen şık veya boş
        {
            if(cevapKagidi[i] == "A"){
                cv::circle(perspectiveimg, Point(centers.at(k).x, centers.at(k).y ),25, Scalar(0,255,0), 3);
            }
            else if(cevapKagidi[i] == "B"){
                cv::circle(perspectiveimg, Point(centers.at(k+1).x, centers.at(k+1).y ),25, Scalar(0,255,0), 3);
            }
            else if(cevapKagidi[i] == "C"){
                cv::circle(perspectiveimg, Point(centers.at(k+2).x, centers.at(k+2).y ),25, Scalar(0,255,0), 3);
            }
            else if(cevapKagidi[i] == "D"){
                cv::circle(perspectiveimg, Point(centers.at(k+3).x, centers.at(k+3).y ),25, Scalar(0,255,0), 3);
            }
            else if(cevapKagidi[i] == "E"){
                cv::circle(perspectiveimg, Point(centers.at(k+4).x, centers.at(k+4).y ),25, Scalar(0,255,0), 3);
            }
        }
        k+=5; // row atlaması için
    }

    QImage finalImg;
    finalImg = matToQImage(perspectiveimg);

    // işaretlenmiş sonuçların ve cevap analizinin mainwindowda gözükmesi
    emit resultsReady(finalImg, dogru, yanlis, bos, net);

    readCredentials();
}

void readCredentials()
{
    // tesseract undefined error hatası - ne yaptıysam linkleyemedim
    // std::string text;
    // cv::Mat im = cv::imread("C:\\Users\\melih\\Desktop\\isimli0.jpg", cv::IMREAD_COLOR);
    // tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    // api->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
    // api->SetPageSegMode(tesseract::PSM_AUTO);
    // api->SetImage(im.data, im.cols, im.rows, 3, im.step);
    // text = std::string(api->GetUTF8Text());
    // qDebug() << text;
    // api->End();
}

// getterlar ve setterlar
int OpenCV::getYanlis() const
{
    return yanlis;
}

void OpenCV::setYanlis(int newYanlis)
{
    yanlis = newYanlis;
}

int OpenCV::getDogru() const
{
    return dogru;
}

void OpenCV::setDogru(int newDogru)
{
    dogru = newDogru;
}

float OpenCV::getNet() const
{
    return net;
}

void OpenCV::setNet(float newNet)
{
    net = newNet;
}
int OpenCV::getBos() const
{
    return bos;
}
void OpenCV::setBos(int newBos)
{
    bos = newBos;
}
