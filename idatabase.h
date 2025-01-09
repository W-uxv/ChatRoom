#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QDataWidgetMapper>

class IDatabase : public QObject
{
    Q_OBJECT



public:
    static IDatabase &getInstance()
    {
        static IDatabase instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
        return instance;
    }

    QString userLogin(QString userName,QString password);
    void silence(QString userName);
    void resume(QString userName);
    int getStatus(QString userName);
    void reg(QString userName, QString password);
    bool search(QString userName);
private:
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(IDatabase const&)               = delete;
    void operator=(IDatabase const&)  = delete;

    QSqlDatabase database;

    void initDatabase();


signals:

public:

};

#endif // IDATABASE_H
