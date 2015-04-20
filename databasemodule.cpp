#include "databasemodule.h"
//#include "logger.h"

DatabaseModule::DatabaseModule(const QString & host, const QString & username, const QString & password, QObject *parent) :
    QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    //m_db->addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setDatabaseName("iflows");
    m_db.setUserName(username);
    m_db.setPassword(password);
    bool ok = m_db.open();
    if(ok == false) {
        //MTLOG("Failed to connect to database...");
    }
    else {
        //MTLOG("Successfully connected to database...");
    }
}

bool DatabaseModule::insertRecord(const QString &record)
{
    // ie. 04/05/2013 11:03:17,1355,306,4b55e4c9C003,124
    QStringList fields = record.split(",");
    QString gid = fields[1];
    QString countyCode = getCountyCode(gid.toInt());
    QString countyName = getCountyName(countyCode);
    //We're gonna timestamp the records server side
    QDateTime now = QDateTime::currentDateTime();
    QString dateStr = now.toString("yyyy-MM-dd hh:mm:ss");
    QString data = fields[2];
    QString raw = fields[3];
    QString netId = fields[4];

    //Insert into database
    QString query = QString("INSERT INTO data VALUES(DEFAULT, '%1', '%2', %3, '%4', %5, '%6', %7)")
            .arg(countyCode).arg(countyName).arg(gid).arg(dateStr).arg(data).arg(raw).arg(netId);
    qDebug() << query;
    QSqlQuery q;
    q.prepare(query);
    return q.exec();
}

bool DatabaseModule::insertHeartbeat(const QString &heartbeat)
{
    // ie. VAZ014,08/12/2013 21:28:21,962,117,V.010,00:22:4d:7c:a5:f4
    QStringList fields = heartbeat.split(",");
    QString countyCode = fields[0];
    //We're gonna timestamp the records server side
    QDateTime now = QDateTime::currentDateTime();
    QString dateStr = now.toString("yyyy-MM-dd hh:mm:ss");
    QString numRecords = fields[2];
    QString netId = fields[3];
    QString version = fields[4];
    QString HWaddr = fields[5];


    //Insert into database
    QString query = QString("INSERT INTO heartbeats VALUES(DEFAULT, '%1', '%2', %3, %4, '%5', '%6');")
            .arg(countyCode).arg(dateStr).arg(numRecords).arg(netId).arg(version).arg(HWaddr);
    QSqlQuery q;
    q.prepare(query);
    return q.exec();
}

QString DatabaseModule::getCountyCode(const int &gid)
{
    QString query = QString("SELECT county_code FROM counties WHERE gid = %1;").arg(gid);
    QSqlQuery q(query);
    q.next();
    return q.value(0).toString();
}

QString DatabaseModule::getCountyName(const QString &countyCode)
{
    QString query = QString("SELECT county_name FROM counties WHERE county_code = '%1';").arg(countyCode);
    QSqlQuery q(query);
    q.next();
    return q.value(0).toString();
}
