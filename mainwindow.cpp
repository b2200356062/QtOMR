#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(new Camera(this))
    , opencv(new OpenCV(this))
    , gallery(new Gallery(this))
    , database(new Database(this))
{
    ui->setupUi(this);
    QWidget::setWindowTitle("Optik Okuyucu");

#ifdef Q_OS_ANDROID
    // opencv kütüphanesini androide yükler
    void* handle = dlopen("libopencv_java4.so", RTLD_LAZY);
    if (!handle) {
        qDebug() << "opencv kütüphanesi yüklenemedi\n: " << dlerror();
        return;
    }
#endif
    // sınav okuma butonları cevap anahtarı girilmeden görülmez
    ui->sinavgaleri->hide();
    ui->sinavkamera->hide();

    init();

    // sinyaller ve slotlar
    connect(camera, &Camera::photoTaken, this, &MainWindow::tookPhoto);
    connect(gallery, &Gallery::photoChosen, this, &MainWindow::updatePhoto);
    connect(opencv, &OpenCV::errorNotify, this, &MainWindow::openCVerror);
    connect(opencv, &OpenCV::resultsReady, this, &MainWindow::displayResults);

    connect(this, &MainWindow::sonuc_kaydet, database, &Database::kaydet);
    connect(database, &Database::kaydedildi, this, &MainWindow::kaydedildi);

    connect(this, &MainWindow::sonuclari_goster, database, &Database::sonuc_istegi);
    connect(database, &Database::sonuclari_dondur, this, &MainWindow::sonuclar_geldi);
    // sonuclar geldi ve sonuc görü bağla
}

MainWindow::~MainWindow()
{
    delete ui;
    // delete camera;
    // delete gallery;
    // delete opencv;
    // delete database;
}

#ifdef Q_OS_ANDROID

bool checkPermission() {
    // hiçbir şekilde izin verilmiyor ama çalışıyor
    QList<bool> permissions;

    auto r = QtAndroidPrivate::checkPermission("android.permission.READ_EXTERNAL_STORAGE").result();

    qDebug() <<"read external: " <<  r << "\n";

    if (r != QtAndroidPrivate::Authorized)
    {
        r = QtAndroidPrivate::requestPermission("android.permission.READ_EXTERNAL_STORAGE").result();

        if (r == QtAndroidPrivate::Denied)
            permissions.append(false);
            qDebug() << "read external izin yok\n";
    }

    r = QtAndroidPrivate::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE").result();

    qDebug() <<"write external: " <<  r << "\n";

    if (r != QtAndroidPrivate::Authorized)
    {
        r = QtAndroidPrivate::requestPermission("android.permission.WRITE_EXTERNAL_STORAGE").result();
        if (r == QtAndroidPrivate::Denied)
            permissions.append(false);
            qDebug() << "write external izin yok\n";
    }

    // auto r = QtAndroidPrivate::checkPermission("android.permission.READ_MEDIA_IMAGES").result();

    // qDebug() <<"read media: " <<  r << "\n";

    // if (r != QtAndroidPrivate::Authorized)
    // {
    //     r = QtAndroidPrivate::requestPermission("android.permission.READ_MEDIA_IMAGES").result();

    //     if (r == QtAndroidPrivate::Denied)
    //         permissions.append(false);
    //         qDebug() << "read media izin yok\n";
    // }

    qDebug() << "permission count: " << permissions.count() << "\n";
    return (permissions.count() != 3);
}

#endif
void MainWindow::init()
{

// dosya izinleri açılırken
#ifdef Q_OS_ANDROID
    if(!checkPermission())
    {
        qDebug() << "permission is not granted\n";
        abort();
        exit(0);
    }
#endif

}

void MainWindow::on_anahtargaleri_clicked()
{
    ui->sayfalar->setCurrentIndex(1);
}
void MainWindow::on_anahtarkamera_clicked()
{
    ui->sayfalar->setCurrentIndex(2);

    camera->initCamera();

    // live previewu göster
    camera->getCaptureSession()->setVideoOutput(ui->ckpreview);

}
void MainWindow::on_sinavgaleri_clicked()
{
    ui->sayfalar->setCurrentIndex(3);
}
void MainWindow::on_sinavkamera_clicked()
{
    ui->sayfalar->setCurrentIndex(4);
    camera->initCamera();
    camera->getCaptureSession()->setVideoOutput(ui->skpreview);
}

// cevap anahtarı galeriden seç
void MainWindow::on_cgsec_clicked()
{
// pcde concurrent thread çalıştırmadan dosya açabilirken androidde thread gui'yı bloklayıp ekranı kilitliyor.
#ifdef Q_OS_WIN
    gallery->chooseFromGallery();
#else
    auto future = QtConcurrent::run([this]{
        gallery->chooseFromGallery();
    });
#endif
}

// sınav kağıdı galeriden seç
void MainWindow::on_sgsec_clicked()
{
#ifdef Q_OS_WIN
    gallery->chooseFromGallery();
#else
    auto future = QtConcurrent::run([this]{
        gallery->chooseFromGallery();
    });
#endif
}

// cevap anahtarını galeriden kaydet
void MainWindow::on_cgkaydet_clicked()
{
    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #000000; }"
        "QLabel { color: white; }"
        "QPushButton { background-color: #000000; color: white; }"
        );

    if(gallery->getChosenPhotoPix().isNull())
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(" ");
        msgBox.setText("Önce fotoğraf çekiniz!");
        msgBox.exec();
    }
    else
    {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(" ");
        msgBox.setText("Kağıt Kaydedildi!");
        msgBox.exec();

        // sinyal vermeyi dene
        opencv->run(gallery->getChosenPhotoImg(), ui->sayfalar->currentIndex());

        ui->sinavgaleri->show();
        ui->sinavkamera->show();
    }
}

void MainWindow::openCVerror()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Hata!");
    msgBox.setText("Lütfen daha yakından ve dik açıyla çekiniz!");

    // Set custom stylesheet for the message box
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #000000; }"
        "QLabel { color: white; }"
        "QPushButton { background-color: #000000; color: white; }"
        );

    msgBox.exec();
}

void MainWindow::tookPhoto()
{
    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #000000; }"
        "QLabel { color: white; }"
        "QPushButton { background-color: #000000; color: white; }"
        );

    QThread::sleep(1);

    if(camera->getTakenPhoto().isNull())
    {
        msgBox.setIcon(QMessageBox::Warning);

        msgBox.setWindowTitle(" ");

        msgBox.setText("Önce fotoğraf çekiniz!");

        msgBox.exec();
    }
    else
    {
        msgBox.setIcon(QMessageBox::Information);

        msgBox.setWindowTitle(" ");

        msgBox.setText("Kağıt Kaydedildi!");

        msgBox.exec();

        if(ui->sinavgaleri->isHidden() && ui->sinavkamera->isHidden())
        {
            ui->sinavgaleri->show();
            ui->sinavkamera->show();
        }
        opencv->run(camera->getTakenPhoto(), ui->sayfalar->currentIndex());
    }
}

void MainWindow::updatePhoto(const QPixmap &pixmap)
{
    //invoke yapmadan main threadde gui işlem yapıldığı kesin olmadığı için main threadde yapılması gerek
    QMetaObject::invokeMethod(this, [this, pixmap]()
    {
        QTimer::singleShot(10, this, [this, pixmap]()
        {
            if(ui->sayfalar->currentIndex() == 1)
            {
                ui->cglabel->setPixmap(pixmap.scaled(ui->cglabel->size()));
                ui->cglabel->update();
                ui->cglabel->repaint();
                ui->cglabel->parentWidget()->update();
            }
            else
            {
                ui->sglabel->setPixmap(pixmap.scaled(ui->sglabel->size()));
                ui->sglabel->update();
                ui->sglabel->repaint();
                ui->sglabel->parentWidget()->update();
            }
        });
    }, Qt::QueuedConnection);
}


// cevap anahtarını kameradan kaydet
void MainWindow::on_ckkaydet_clicked()
{
    camera->takePhoto();
}

// galeriden sınav seç ve değerlendir
void MainWindow::on_sgdegerlendir_clicked()
{
    QMessageBox msgBox;

    msgBox.setStyleSheet(
        "QMessageBox { background-color: #000000; }"
        "QLabel { color: white; }"
        "QPushButton { background-color: #000000; color: white; }"
    );

    if(gallery->getChosenPhotoPix().isNull())
    {
        msgBox.setIcon(QMessageBox::Warning);

        msgBox.setWindowTitle(" ");

        msgBox.setText("Önce fotoğraf çekiniz!");

        msgBox.exec();
    }
    else
    {
        msgBox.setIcon(QMessageBox::Information);

        msgBox.setWindowTitle(" ");

        msgBox.setText("Kağıt Kaydedildi!");

        msgBox.exec();

        opencv->run(gallery->getChosenPhotoImg(), ui->sayfalar->currentIndex());

        QString sonuc = QString("Kişinin Sınav Sonucu:\n%1 Doğru %2 Yanlış %3 Boş %4 Net").arg(QString::number(opencv->getDogru()),
                    QString::number(opencv->getYanlis()), QString::number(opencv->getBos()), QString::number(opencv->getNet()));

        on_sggeri_clicked();

        // degerlendirme sayfası
        ui->dslabel->setAlignment(Qt::AlignHCenter);
        ui->dslabel->setText(sonuc);

        ui->sayfalar->setCurrentIndex(5);
    }
}

// database'e sonuçları göndermek için
int dogru2 = 0;
int yanlis2 = 0;
int bos2 = 0;
float net2 = 0.0;

void MainWindow::displayResults(const QImage& finalImg, const int &dogru, const int &yanlis, const int &bos, const float &net)
{
    // sonuç sayfası sadece sınav kamera değerlendirme veya sınav galeri değerlendirmede gözükecek
    if(ui->sayfalar->currentIndex() == 3 || ui->sayfalar->currentIndex() == 4)
    {
        ui->dsimglabel->setPixmap(QPixmap::fromImage(finalImg).scaled(ui->dsimglabel->size()));

        QString sonuc = QString("Kişinin Sınav Sonucu:\n%1 Doğru %2 Yanlış %3 Boş %4 Net").arg(QString::number(dogru),
                QString::number(yanlis), QString::number(bos), QString::number(net));

        // degerlendirme sayfası
        on_sggeri_clicked();

        ui->dslabel->setAlignment(Qt::AlignHCenter);
        ui->dslabel->setText(sonuc);

        ui->sayfalar->setCurrentIndex(5);
    }

    dogru2 = dogru;
    bos2 = bos;
    yanlis2 = yanlis;
    net2 = net;
}

// kameradan sınav çek ve değerlendir
void MainWindow::on_skdegerlendir_clicked()
{
    camera->takePhoto();
}

void MainWindow::on_kisikaydet_clicked()
{   // kaydetme sayfasını açar
    ui->sayfalar->setCurrentIndex(6);
}

void MainWindow::on_sonuckaydet_clicked()
{   // sonucu dbye kaydeder

    QString ad = ui->adgir->toPlainText();
    QString soyad = ui->soyadgir->toPlainText();
    QString numara = ui->numaragir->toPlainText();

    int n = numara.toInt();

    emit sonuc_kaydet(ad, soyad, n, dogru2, yanlis2, bos2, net2);
}

void MainWindow::kaydedildi()
{
    // dbden gelen bilgi
    QMessageBox msgBox;

    msgBox.setStyleSheet(
        "QMessageBox { background-color: #000000; }"
        "QLabel { color: white; }"
        "QPushButton { background-color: #000000; color: white; }"
    );

    msgBox.setIcon(QMessageBox::Information);

    msgBox.setWindowTitle("Kayıt Başarılı!");

    msgBox.setText("Kişi kaydedildi!");

    msgBox.exec();
}

void MainWindow::on_sonucgor_clicked()
{   // dbdeki diğer sonuçları gösterir
    emit sonuclari_goster();
}

void MainWindow::sonuclar_geldi(QSqlTableModel *table)
{   // db sonuçlarını göster
    ui->tablo->setModel(table);
}

// GERİ BUTONLARI
void MainWindow::on_cggeri_clicked()
{   // geriye basınca stacked widget sayfasını resetler
    ui->cglabel->clear();
    gallery->setChosenPhotoPix(QPixmap());
    ui->sayfalar->setCurrentIndex(0);
}

void MainWindow::on_ckgeri_clicked()
{
    camera->setTakenPhoto(QImage());
    ui->sayfalar->setCurrentIndex(0);
}

void MainWindow::on_sggeri_clicked()
{
    ui->sglabel->clear();
    gallery->setChosenPhotoPix(QPixmap());
    ui->sayfalar->setCurrentIndex(0);
}

void MainWindow::on_skgeri_clicked()
{
    camera->setTakenPhoto(QImage());
    ui->sayfalar->setCurrentIndex(0);
}

void MainWindow::on_dsgeri_clicked()
{
    ui->dslabel->clear();
    // tekrar optik okutunca verilerin sıfırlanması için
    opencv->setDogru(0);
    opencv->setYanlis(0);
    opencv->setNet(0.0);
    opencv->setBos(0);
    ui->sayfalar->setCurrentIndex(0);
}

void MainWindow::on_sonucgeri_clicked()
{
    ui->sayfalar->setCurrentIndex(0);
}

// çıkış
void MainWindow::on_cikis_clicked()
{
    abort();
    exit(0);
}


