#include "chatserver.h"
#include "idatabase.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{

}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    ServerWorker *worker = new ServerWorker(this);
    if(!worker->setSocketDescriptor(socketDescriptor)) {
        worker->deleteLater();
        return;
    }
    connect(worker, &ServerWorker::logMessage, this, &ChatServer::logMessage);
    connect(worker, &ServerWorker::jsonReceived, this, &ChatServer::jsonReceived);
    connect(worker, &ServerWorker::disconnectedFromClient, this,
            std::bind(&ChatServer::userDisconnected,this,worker));
    m_clients.append(worker);
    //emit logMessage("新的用户连接上了");
}

void ChatServer::broadcast(const QJsonObject &message)
{
    for(ServerWorker *worker: m_clients) {
        worker->sendJson(message);
    }
}

void ChatServer::unicast(const QJsonObject &message, const QString receiver, ServerWorker *exclude) {
    exclude->sendJson(message);
    for(ServerWorker *worker: m_clients) {
        if(worker->userName() == receiver) worker->sendJson(message);
    }
}

void ChatServer::stopServer()
{
    close();
}

void ChatServer::jsonReceived(ServerWorker *sender, const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");
    if(typeVal.isNull() || !typeVal.isString())
        return;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)== 0) {
        const QJsonValue textVal= docObj.value("text");
        if(textVal.isNull() || !textVal.isString())
            return;
        const QString text=textVal.toString().trimmed();
        if(text.isEmpty())
            return;
        QJsonObject message;
        message["type"] = "message";
        message["text"] = text;
        message["sender"] = sender->userName();

        int status = IDatabase::getInstance().getStatus(sender->userName());
        if(status != 1) return;
        broadcast(message);
    } else if(typeVal.toString().compare("login", Qt::CaseInsensitive)==0) {
        const QJsonValue usernameVal = docObj.value("text");
        if(usernameVal.isNull() || !usernameVal.isString())
            return;

        QJsonObject statusMessage;
        statusMessage["type"]="status";
        for(ServerWorker *worker:m_clients){
            if(worker->userName() == usernameVal.toString()) {
                statusMessage["status"] = "用户已登录";
                sender->sendJson(statusMessage);
                return;
            }
        }

        const QJsonValue passwordVal = docObj.value("password");
        if(passwordVal.isNull() || !passwordVal.isString())
            return;

        QString status = IDatabase::getInstance().userLogin(usernameVal.toString(),
                                                            passwordVal.toString());
        statusMessage["status"] = status;
        sender->sendJson(statusMessage);
        if(status != "LoginOk") return;
        emit logMessage("新的用户连接上了");


        sender->setUserName(usernameVal.toString());
        QJsonObject connectedMessage;
        connectedMessage["type"]="newuser";
        connectedMessage["username"]=usernameVal.toString();
        broadcast(connectedMessage);

        QJsonObject userListMessage;
        userListMessage["type"]= "userlist";
        QJsonArray userlist;
        for(ServerWorker *worker:m_clients){
            if(worker == sender)
                userlist.append(worker->userName()+"*");
            else
                userlist.append(worker->userName());
        }
        userListMessage["userlist"]=userlist;
        sender->sendJson(userListMessage);
    } else if(typeVal.toString().compare("unicast",Qt::CaseInsensitive)== 0) {
        const QJsonValue textVal= docObj.value("text");
        if(textVal.isNull() || !textVal.isString())
            return;

        const QString text = textVal.toString().trimmed();
        if(text.isEmpty())
            return;

        const QJsonValue receiverVal = docObj.value("receiver").toString();
        if(receiverVal.isNull() || !receiverVal.isString())
            return;

        QJsonObject message;
        message["type"] = "unicast";
        message["text"] = text;
        message["sender"] = sender->userName();
        message["receiver"] = receiverVal.toString();
        unicast(message,receiverVal.toString(),sender);
    } else if(typeVal.toString().compare("opt",Qt::CaseInsensitive)== 0) {
        const QJsonValue statusVal= docObj.value("text");
        const QString userName = docObj.value("userName").toString();
        if(statusVal.toString() == "2") IDatabase::getInstance().silence(userName);
        else if(statusVal.toString() == "1") IDatabase::getInstance().resume(userName);
        else if(statusVal.toString() == "0") {
            QJsonObject banMessage;
            banMessage["type"] = "ban";
            for(ServerWorker *worker : m_clients) {
                if(worker->userName() == userName) worker->sendJson(banMessage);
            }
        }
    }else if(typeVal.toString().compare("register",Qt::CaseInsensitive)== 0) {
        const QJsonValue userNameVal = docObj.value("text");
        if(userNameVal.isNull() || !userNameVal.isString())
            return;

        const QJsonValue passwordVal = docObj.value("password");
        if(passwordVal.isNull() || !passwordVal.isString())
            return;
        IDatabase::getInstance().reg(userNameVal.toString(),passwordVal.toString());
    }
}

void ChatServer::userDisconnected(ServerWorker *sender)
{
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if(!userName.isEmpty()){
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"]="userdisconnected",disconnectedMessage["username"]= userName;
        broadcast(disconnectedMessage);
        emit logMessage(userName + "disconnected");
    }
    sender->deleteLater();
}
