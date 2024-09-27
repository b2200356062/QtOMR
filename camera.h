#ifndef CAMERA_H
#define CAMERA_H

#include <QDialog>
#include <QMessageBox>
#include <QDir>
#include <QtMultimedia/QtMultimedia>
#include <QFileDialog>
#include <QSemaphore>


class Camera : public QDialog
{
    Q_OBJECT

public:
    explicit Camera(QWidget *parent = nullptr);
    ~Camera();

    void initCamera();
    void takePhoto();
    void imageSaved(int id, const QString &fileName);
    void processCapturedImage(int requestId, const QImage &img);

    void displayCaptureError(int, QImageCapture::Error, const QString &errorString);
    void convertImage(const QString &filePath);

    QImage getTakenPhoto();
    void setTakenPhoto(QImage photo);

    QMediaCaptureSession *getCaptureSession() const;
    void setCaptureSession(QMediaCaptureSession *newCaptureSession);

signals:
    void photoTaken();

private:
    QScopedPointer<QCamera> cam;
    QMediaCaptureSession *captureSession = nullptr;
    QScopedPointer<QImageCapture> imageCapture;
    QImage takenPhoto;
};

#endif // CAMERA_H
