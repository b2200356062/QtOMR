#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QDebug>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QTableView>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    void initDB();

signals:
    void kaydedildi();
    void sonuclari_dondur(QSqlTableModel *table);

public slots:
    void kaydet(const QString &ad, const QString &soyad, const int &numara, const int &dogru, const int& yanlis, const int& bos, const float& net);
    void sonuc_istegi();

private:
    QSqlDatabase db;
    QSqlTableModel *model = nullptr;
};

#endif // DATABASE_H
