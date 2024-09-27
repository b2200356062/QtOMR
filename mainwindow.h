#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCoreApplication>
#ifdef Q_OS_ANDROID
    #include <QtCore/private/qandroidextras_p.h>
    #include <dlfcn.h>
#endif
#include <QtMultimedia/QtMultimedia>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <QScreen>
#include <QDebug>
#include "opencv.h"
#include "camera.h"
#include "gallery.h"
#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void init();

signals:
    void sonuc_kaydet(const QString &ad, const QString &soyad, const int& numara, const int &dogru, const int& yanlis, const int& bos, const float& net);

    void sonuclari_goster();

    void dybn_gonder();

private slots:
    void on_anahtargaleri_clicked();

    void on_sinavkamera_clicked();

    void on_sinavgaleri_clicked();

    void on_cikis_clicked();

    void on_anahtarkamera_clicked();

    void on_ckkaydet_clicked();

    void on_sgdegerlendir_clicked();

    void on_skdegerlendir_clicked();

    void on_cgkaydet_clicked();

    void on_skgeri_clicked();

    void on_sggeri_clicked();

    void on_ckgeri_clicked();

    void on_cggeri_clicked();

    void on_cgsec_clicked();

    void on_sgsec_clicked();

    void on_dsgeri_clicked();

    void on_kisikaydet_clicked();

    void on_sonucgeri_clicked();

    void on_sonuckaydet_clicked();

    void on_sonucgor_clicked();


public slots:
    void tookPhoto();

    void updatePhoto(const QPixmap &pixmap);

    void openCVerror();

    void displayResults(const QImage &img, const int& dogru, const int& yanlis, const int& bos, const float& net);

    void kaydedildi();

    void sonuclar_geldi(QSqlTableModel *table);

private:
    Ui::MainWindow *ui;
    Camera* camera = nullptr;
    OpenCV* opencv = nullptr;
    Gallery* gallery = nullptr;
    Database* database = nullptr;

};
#endif // MAINWINDOW_H
