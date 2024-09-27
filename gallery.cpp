#include "gallery.h"

#ifdef Q_OS_ANDROID
    #include <QJniObject>
    #include <QJniEnvironment>
    #include <QUrl>
#endif

Gallery::Gallery(QWidget *parent)
    : QDialog(parent)
{}

Gallery::~Gallery()
{}

// pc file path
QString filePath = "C:\\Users\\melih\\Desktop\\";

// android file path
QString filePathAndroid = "/storage/emulated/0/DCIM/Camera/";

void Gallery::chooseFromGallery()
{
#ifdef Q_OS_WIN
    QString fileName = QFileDialog::getOpenFileName(this, "Bir fotoğraf seç", filePath);
#endif

#ifdef Q_OS_ANDROID
    QString fileName = QFileDialog::getOpenFileName(this, "Bir fotoğraf seç", filePathAndroid);
#endif

    if(!fileName.isEmpty())
    {
        chosenPhotoPix = QPixmap(fileName);
        chosenPhotoImg = QImage(fileName);

        this->setChosenPhotoPix(chosenPhotoPix);
        this->setChosenPhotoImg(chosenPhotoImg);

        emit photoChosen(chosenPhotoPix);
    }
}
// getterlar ve setterlar
QPixmap Gallery::getChosenPhotoPix()
{
    return chosenPhotoPix;
}

void Gallery::setChosenPhotoPix(QPixmap chosenPhoto)
{
    this->chosenPhotoPix = chosenPhoto;
}

QImage Gallery::getChosenPhotoImg() const
{
    return chosenPhotoImg;
}

void Gallery::setChosenPhotoImg(QImage newChosenPhotoImg)
{
    chosenPhotoImg = newChosenPhotoImg;
}
