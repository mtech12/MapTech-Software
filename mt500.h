#ifndef MT500_H
#define MT500_H

#include <QMainWindow>
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
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
#include <QProcess>

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
    int countFile(QString);
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
    void checkResetFIPS();
    void sendRawStrtoIflows(QString);
    void sendFips();
    QStringList sortFips();
    bool inFilter(int node, int gid);
    QStringList fipsDiff (QString, QString); 
    void sendRecord(QString);

public slots:
    void getData();
    void sendHB();
    void getFips();
    void readData();
    void ipError(int);
    void resetPorts();
    void clrBuf();

private slots:
    void on_addButton_clicked();
    void on_delButton_clicked();
    void on_testButton_clicked();
    void on_clrButton_clicked();

    void on_reconfigureButton_clicked();

    void on_runModeComboBox_currentIndexChanged(int index);

private:
    Ui::MT500 *ui;
    int initialCount, heartbeatInterval, msgCount, fipsInterval, byteCount,boxGID,recordCnt, byteCnt, ipRetries, socketCtr, cloudCnt;
    qint32 outBaud, inBaud;
    QTimer *pollTimer, *hbTimer, *fipsTimer, *resetTimer, *validMsgTimer;
    QTcpSocket *sockets[20], *test;
    ipStruct ipArray[6];
    QStringList getFiles;
    QString fipsNo, ipConfig, fipsDir, COMin, COMout, currentIP, progVer, HWaddr;
    QHash<QString, QMap<QDateTime, QString> > fipsCount;
    SerialPort *inPort, *outPort;
    QByteArray msg, ba;
    bool initial, log;
    QHash<int, QDateTime> fipsFilter; //GID, Last Record
    //QHash<int, QString> sendList; //Index, Time:Raw Data
    QStringList sendList, copyParms;
    QHash<QString, QString> filterList; //Node, GID
    QProcess *copyProc;
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
