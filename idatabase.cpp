#include "idatabase.h"
#include<QUuid>
void IDatabase::initDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    QString aFile = "user.db";
    database.setDatabaseName(aFile);
    if (!database.open()) {
        qDebug() << "failed to open database";
    } else {
        qDebug() << "open database is ok";
    }
}

QString IDatabase::userLogin(QString userName, QString password)
{
    QSqlQuery query;
    query.prepare("select username,password from user where username=:USER");
    query.bindValue(":USER", userName);
    query.exec();
    query.first();
    if (query.first() && query.value("username").isValid()) {
        QString passwd = query.value("password").toString();
        if (passwd == password) {

            return "LoginOk";
        } else {
            return "wrong password";
        }
    } else {
        qDebug() << "no such user";
        return "wrong username";
    }
}

void IDatabase::silence(QString userName)
{
    QSqlQuery query;
    query.prepare("UPDATE user SET status = :STATUS WHERE username = :USER");
    query.bindValue(":STATUS", 2);
    query.bindValue(":USER", userName);
    if (!query.exec()) {
        qDebug() << "Failed to update status:" << query.lastError();
    } else {
        qDebug() << "Status updated successfully";
    }
}

void IDatabase::resume(QString userName)
{
    QSqlQuery query;
    query.prepare("UPDATE user SET status = :STATUS WHERE username = :USER");
    query.bindValue(":STATUS", 1);
    query.bindValue(":USER", userName);
    if (!query.exec()) {
        qDebug() << "Failed to update status:" << query.lastError();
    } else {
        qDebug() << "Status updated successfully";
    }
}

int IDatabase::getStatus(QString userName)
{
    QSqlQuery query;
    query.prepare("SELECT status FROM user WHERE username = :USER");
    query.bindValue(":USER", userName);
    if (query.exec()) {
        if (query.next()) {
            int status = query.value(0).toInt();
            return status;
        } else
            qDebug() << "No matching user found";
    }
    return 0;
}

void IDatabase::reg(QString userName, QString password)
{
    if(search(userName)) return;
    QSqlQuery query;
    query.prepare("INSERT INTO user (username, password) VALUES (:userName, :password)");

    // 绑定参数
    query.bindValue(":userName", userName);
    query.bindValue(":password", password);

    // 执行插入操作
    if (query.exec()) {
        qDebug() << "User added successfully.";
    } else {
        qDebug() << "Failed to add user: " << query.lastError().text();
    }
}

bool IDatabase::search(QString userName)
{
    QSqlQuery query;
    query.prepare("select username from user where username=:USER");
    query.bindValue(":USER", userName);
    query.exec();
    query.first();
    if (query.first() && query.value("username").isValid())
        return true;
    else
        return false;
}

IDatabase::IDatabase(QObject *parent)
    : QObject{parent}
{
    initDatabase();
}


