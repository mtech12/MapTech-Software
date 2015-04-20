#ifndef LOGGER_H
#define LOGGER_H

#include<QDateTime>
#include<QFile>
#include<QDir>

#define MTLOG(text) \
{ \
    mtlog(text, __LINE__, __func__, __DATE__, __TIME__, __FILE__); \
}

void mtlog(QString logText, int lineNumber, std::string functionName, std::string date, std::string time, std::string fileName)
{
    QString txt = QString("[%1 %2] %3:%4 (%5)\t%6\n").arg(QString(date.c_str())).arg(QString(time.c_str()))
            .arg(QString(fileName.c_str())).arg(lineNumber).arg(QString(functionName.c_str())).arg(logText);

    QString dirname = "/home/administrator/Desktop/MT500/logs/";
    QDir dir (dirname);
    dir.mkpath (dirname); //Ensure directory exists
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString filename = QString("%1%2.log").arg(dirname).arg(curDateTime.toString("yyMMdd"));
    QFile file (filename);
    if(file.open (QIODevice::WriteOnly | QIODevice::Append) == true) {
        file.write(txt.toLatin1 ().data ());
        file.close();
    }
}

#endif // LOGGER_H
