#ifndef GALLERY_H
#define GALLERY_H

#include <QDialog>
#include <QFileDialog>

#include <QObject>
#include <QPixmap>
#include <QThread>


class Gallery : public QDialog
{
    Q_OBJECT

public:
    explicit Gallery(QWidget *parent = nullptr);
    ~Gallery();

    void chooseFromGallery();

    QPixmap getChosenPhotoPix();
    void setChosenPhotoPix(QPixmap chosenPhoto);

    QImage getChosenPhotoImg() const;
    void setChosenPhotoImg(QImage newChosenPhotoImg);

signals:
    void startProcessing();
    void photoChosen(const QPixmap &pixmap);

private:
    QImage chosenPhotoImg;
    QPixmap chosenPhotoPix;
};

#endif // GALLERY_H
