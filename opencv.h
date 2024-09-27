#ifndef OPENCV_H
#define OPENCV_H

#include <QWidget>
#include <QLabel>

class OpenCV : public QWidget
{
    Q_OBJECT

public:
    explicit OpenCV(QWidget *parent = nullptr);
    ~OpenCV();

    void run(const QImage &img, int page);

    int getYanlis() const;
    void setYanlis(int newYanlis);

    int getDogru() const;
    void setDogru(int newDogru);

    float getNet() const;
    void setNet(float newNet);

    int getBos() const;
    void setBos(int newBos);

signals:
    void errorNotify();
    void resultsReady(const QImage& finalImg, const int &dogru, const int &yanlis, const int &bos, const float &net);

private:
    std::map<int, std::string> answers;
    std::map<int, std::string> cevapKagidi;

    int dogru = 0;
    int yanlis = 0;
    int bos = 0;
    float net = 0;

};

#endif // OPENCV_H
