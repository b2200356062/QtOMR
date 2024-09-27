#include "camera.h"

Camera::Camera(QWidget *parent)
    : QDialog(parent)

{}

Camera::~Camera()
{
    delete captureSession;
}

bool checkCameraAvailability()
{
    if (QMediaDevices::videoInputs().count() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Camera::initCamera()
{

    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission))
    {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(cameraPermission, this, &Camera::initCamera);
        return;
    case Qt::PermissionStatus::Denied:
        qWarning("kamera izni verilmedi!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }

    if(!checkCameraAvailability())
    {
        qDebug() << "kamera bulunamadı\n";
        return;
    }

    const QList<QCameraDevice> Cameras = QMediaDevices::videoInputs();

    for (const QCameraDevice &CameraDevice : Cameras) {
                                    // arka kamera                               pc
        if(CameraDevice.description() == "Rear Camera: 0" || CameraDevice.description() == "HD Webcam")
        {
            cam.reset(new QCamera(CameraDevice));
        }
    }

    captureSession = new QMediaCaptureSession();

    // set up camera
    captureSession->setCamera(cam.data());

    imageCapture.reset(new QImageCapture);

    captureSession->setImageCapture(imageCapture.get());

    connect(imageCapture.get(), &QImageCapture::imageCaptured, this, &Camera::processCapturedImage);
    // her defasında görseli kaydetmesin
    connect(imageCapture.get(), &QImageCapture::imageSaved, this, &Camera::imageSaved);
    connect(imageCapture.get(), &QImageCapture::errorOccurred, this, &Camera::displayCaptureError);

    cam->start();
}

QImage Camera::getTakenPhoto()
{
    return takenPhoto;
}

void Camera::setTakenPhoto(QImage takenPhoto)
{
    this->takenPhoto = takenPhoto;
}

QMediaCaptureSession *Camera::getCaptureSession() const
{
    return captureSession;
}

void Camera::setCaptureSession(QMediaCaptureSession *newCaptureSession)
{
    captureSession = newCaptureSession;
}

void Camera::takePhoto()
{
    // capture photo
#ifdef Q_OS_WIN
    QString saveLocation = "C:\\Users\\melih\\Desktop\\foto1.jpg";
    imageCapture->captureToFile(saveLocation);
#else
    QString saveLocation =  "/storage/emulated/0/DCIM/Camera/";
    imageCapture->captureToFile(saveLocation);
#endif

}

void Camera::processCapturedImage(int requestId, const QImage &img)
{
    Q_UNUSED(requestId);
    this->setTakenPhoto(img);
}

void Camera::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);
    emit photoTaken();
}

void Camera::displayCaptureError(int id, const QImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
}



