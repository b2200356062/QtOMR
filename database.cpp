#include "database.h"

Database::Database(QObject *parent)
    : QObject{parent}
{
    initDB();
}

Database::~Database()
{
    delete model;
}

void Database::initDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");

#ifdef Q_OS_ANDROID
    db.setDatabaseName("/storage/emulated/0/Documents/sonuclar.db");
#else
    db.setDatabaseName("C:\\Users\\melih\\Desktop\\sonuclar.db");
#endif
    db.open();

    QSqlQuery dbQuery;
    dbQuery.exec("CREATE TABLE IF NOT EXISTS sonuclar (ad text, soyad text, numara int not null primary key, dogru int, yanlis int, bos int, net numeric);");
}

void Database::kaydet(const QString &ad, const QString &soyad, const int &numara, const int &dogru, const int& yanlis, const int& bos, const float& net)
{
    if(db.open())
    {
        QSqlQuery query;
        query.prepare("INSERT INTO sonuclar (ad, soyad, numara, dogru, yanlis, bos, net) VALUES (:ad, :soyad, :numara, :dogru, :yanlis, :bos, :net)");
        query.bindValue(":ad", ad);
        query.bindValue(":soyad", soyad);
        query.bindValue(":numara", numara);
        query.bindValue(":dogru", dogru);
        query.bindValue(":yanlis", yanlis);
        query.bindValue(":bos", bos);
        query.bindValue(":net", net);

        if (!query.exec()){
            // aynı numaralı öğrenciyi güncellemek için:
            // INSERT or REPLACE into tablename VALUES (value1,value2 , so on );
            qDebug() << "Error inserting data:" << query.lastError().text() << "\n";
        }
        else
        {
            qDebug() << "Data inserted successfully!";
            emit kaydedildi();
        }
    }
    else
    {
        qDebug() << "HATA! DB ACIK DEĞİL\n";
    }
    db.close();
}

void Database::sonuc_istegi()
{   // sonucları döndür
    if(!db.isOpen())
    {
        db.open();
    }

    model = new QSqlTableModel(this, db);

    QString tableName = "sonuclar";

    model->setTable(tableName);

    model->select();

    if (model->lastError().isValid()) {
        qDebug() << "dbden secilemedi!" << model->lastError().text();
    } else {
        emit sonuclari_dondur(model);  // Emit the signal with the model
    }

    db.close();
}
