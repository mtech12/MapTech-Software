#ifndef DATABASEMODULE_H
#define DATABASEMODULE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <QDebug>

class DatabaseModule : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseModule (const QString & host, const QString & username, const QString & password, QObject *parent = 0);
    bool insertRecord (const QString & record);
    bool insertHeartbeat (const QString & heartbeat);
    QString getCountyCode (const int & gid);
    QString getCountyName (const QString & countyCode);
    
signals:
    
public slots:

private:

    QSqlDatabase m_db;
    
};

#endif // DATABASEMODULE_H
