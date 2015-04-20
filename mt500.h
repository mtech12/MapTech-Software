#ifndef MT500_H
#define MT500_H

#include <QMainWindow>
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
#include <QVariant>
#include <QTimer>
#include <Q3Socket>
#include <Q3SocketDevice>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QHash>
#include <QtAddOnSerialPort/serialport.h>
#include <QtAddOnSerialPort/serialportinfo.h>
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>

#include "databasemodule.h"

QT_USE_NAMESPACE_SERIALPORT

namespace Ui {
    class MT500;
}

struct ipStruct {
    QString ip;
    int port;
    QString dataType;
};

class MT500 : public QMainWindow {
    Q_OBJECT
public:
    MT500(QWidget *parent = 0);
    ~MT500();

protected:
    void changeEvent(QEvent *e);
    void getIPs();
    QByteArray encode(QString message);
    void sendRaw(QString);
    void sendBase(QString, bool);
    void sendIFLOWS(QByteArray);
    void getConfig();
    void getFipsCounts();
    void setupRS232(QString, QString);
    QString decode(QByteArray);
    void testConnection();
    void sendRawStrtoIflows(QString);
    bool inFilter(int node, int gid);

public slots:
    void slot_sendHB();
    void slot_readData();
    void slot_ipError(int);
    void slot_resetPorts();
    void slot_clrBuf();
    void on_addButton_clicked();
    void on_delButton_clicked();
    void on_testButton_clicked();
    void on_clrButton_clicked();
    void on_reconfigureButton_clicked();
    void on_runModeComboBox_currentIndexChanged(int index);
    void slot_dataReceived();

private:
    Ui::MT500 *ui;
    int m_initialCount, m_heartbeatInterval, m_msgCount, m_fipsInterval, m_byteCount,m_boxGID,m_recordCnt, m_socketCtr, m_cloudCnt;
    qint32 m_outBaud, m_inBaud;
    QTimer *m_hbTimer, *m_resetTimer, *m_validMsgTimer;
    QTcpSocket *m_sockets[20];
    ipStruct m_ipArray[6];
    QStringList m_getFiles;
    QString m_fipsNo, m_ipConfig, m_fipsDir, m_COMin, m_COMout, m_currentIP, m_progVer, m_HWaddr;
    SerialPort *m_inPort, *m_outPort;
    QByteArray m_msg, m_ba;
    bool m_initial, m_log;
    QHash<int, QDateTime> m_fipsFilter; //GID, Last Record
    QHash<int, QString> m_sendList; //Index, Time:Raw Data
    QHash<QString, QString> m_filterList; //Node, GID
    DatabaseModule * m_db;
    QTcpServer * m_server;
};

class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

#endif // MT500_H
