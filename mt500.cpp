#include "mt500.h"
#include "ui_mt500.h"
#include "logger.h"

MT500::MT500(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MT500)
{
    log = true;
    if(log) MTLOG("MT500 Object Constructed");
    ui->setupUi(this);
    //Set program version here /////////
    progVer = "V.011";
    ////////////////////////////////////
    ui->versionLbl->setText("Program Version: " + progVer);
    ui->rxLabel->setText("Messages Transmitted: 0");
    initial = false;
    ui->ipTable->setColumnWidth(0, 120);
    getConfig();
    byteCount = 0;
    recordCnt = 0;
    cloudCnt = 0;
    setupRS232(COMin, COMout);
    initialCount = countFile("/home/administrator/Desktop/data.log");
    resetTimer = new QTimer(this);
    connect(resetTimer, SIGNAL(timeout()), this, SLOT(resetPorts()));
    validMsgTimer = new QTimer(this);
    connect(validMsgTimer, SIGNAL(timeout()), this, SLOT(clrBuf()));
    hbTimer = new QTimer(this);
    connect(hbTimer, SIGNAL(timeout()), this, SLOT(sendHB()));
    fipsTimer = new QTimer(this);
    connect(fipsTimer, SIGNAL(timeout()), this, SLOT(getFips()));
    msgCount = 0; //Does not include heartbeat messages
    getIPs();
    getFipsCounts();
    for(int i = 0; i < 20; i++) {
        sockets[i] = new QTcpSocket(this);
    }
    //testConnection();
    if(log) MTLOG(QString("HB Interval: %1 min").arg(heartbeatInterval));
    if(log) MTLOG(QString("Fips Retrieval Interval: %1 min").arg(fipsInterval));
    hbTimer->start(heartbeatInterval*1000*60);
    fipsTimer->start(fipsInterval*60*1000);
    resetTimer->start(3600000 * 3); //Reset COM ports every 3 hours
    sendHB();
}

MT500::~MT500()
{
    delete ui;
}

void MT500::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MT500::getData()
{
    QString line;
    int count = 0;
    QFile file("/home/administrator/Desktop/data.log");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count > initialCount) {
                //ui->DataBrowser->append(line);
                recordCnt++;
                if(recordCnt > 500){
                    ui->DataBrowser->clear();
                    recordCnt =0;
                }
                initialCount = count;
                if(line.length() > 0) {
                    sendRaw(line.trimmed());
                    sendBase(line.trimmed(), true);
                    QByteArray msgToSend;
                    msgToSend = encode(line.trimmed());
                    sendIFLOWS(msgToSend);
                }
            }
        }
    }
    pollTimer->start(500);
}

int MT500::countFile(QString filename)
{
    QString line;
    int count = 0;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
        }
    }
    return count;
}

void MT500::getIPs()
{
    for(int i = 0; i < 6; i++) ipArray[i].ip = "";
    ui->delSelect->clear();
    for(int col = 0; col < 3; col++) {
        for(int row = 0; row < 6; row++) {
            ui->ipTable->setItem(row, col, new QTableWidgetItem(""));
        }
    }
    QString line;
    QStringList fields;
    int count = 0;
    QFile file(ipConfig);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count > 9 && count <= 15) { //Skip header && only allow 6 entries
                fields = line.split('\t');
                ipArray[count-10].ip = fields.at(0).trimmed();
                ipArray[count-10].port = fields.at(1).trimmed().toInt();
                ipArray[count-10].dataType = fields.at(2).trimmed();
                ui->ipTable->setItem(count-10, 0, new QTableWidgetItem(ipArray[count-10].ip));
                ui->ipTable->setItem(count-10, 1, new QTableWidgetItem(QString::number(ipArray[count-10].port)));
                ui->ipTable->setItem(count-10, 2, new QTableWidgetItem(ipArray[count-10].dataType));
                ui->delSelect->addItem(ipArray[count-10].ip);
            }
        }
    }
}

QByteArray MT500::encode(QString message)
{
    //*******GET GID**********//
    //Expecting ie. A 0189 08/27/1989 08:00:00 2047
    QByteArray returnVal;
    returnVal.clear();
    QString gid = message;
    //qDebug() << gid;
    QStringList fields = gid.split(" ");
    int gid_int = fields.at(1).trimmed().toInt();
    int data_int = fields.at(4).trimmed().toInt();
    //qDebug() << data_int << gid_int;
    unsigned int byte1 = (gid_int & 0x0000003F) | 0x00000040;
    //qDebug() << "Byte 1: " + QString::number(byte1, 16);
    returnVal.append(QString::number(byte1, 16));
    unsigned int byte2 = ((gid_int >> 6) & 0x0000003F) | 0x00000040;
    //qDebug() << "Byte 2: " + QString::number(byte2, 16);
    returnVal.append(QString::number(byte2, 16));
    unsigned int byte3 = ((gid_int >> 12) & 0x00000001) | ((data_int << 1) & 0x0000003E) | 0x000000C0;
    //qDebug() << "Byte 3: " + QString::number(byte3, 16);
    returnVal.append(QString::number(byte3, 16));
    unsigned int byte4 = ((data_int >> 5) & 0x0000003F) | 0x000000C0;
    //qDebug() << "Byte 4: " + QString::number(byte4, 16);
    returnVal.append(QString::number(byte4, 16));
    returnVal.append("C0");
    returnVal.append("03");
    return returnVal;
}

void MT500::sendRaw(QString line)  //<--- Probably needs to be handled differently
{
    QByteArray bytesToSend;
    for(int y = 0; y < 6; y++) {
        if(ipArray[y].ip != "") {
            if(ipArray[y].dataType == "RAW" && !ipArray[y].ip.isEmpty()) {
                QString ip = ipArray[y].ip;
                int port = ipArray[y].port;
                sockets[y]->connectToHost(QHostAddress(ip), port);
                bytesToSend = encode(line);
                for(int i = 0; i < 6; i++) {
                    sockets[y]->putch(bytesToSend[i]);
                }
                sockets[y]->close();
                msgCount++;
            }
        }
    }
    ui->rxLabel->setText("Messages Transmitted: "+QString::number(msgCount));
}

void MT500::sendBase(QString line, bool toAppend)
{
    QString color = "red";
    socketCtr++;
    if(socketCtr == 20) socketCtr = 0;
    if(log) MTLOG(QString("Sending record to servers: %1").arg(line));
    QByteArray bytesToSend;
    QStringList fields;
    QString rawStr;
    fields = line.split(" ");
    QString ts = fields.at(2).trimmed()+" "+fields.at(3).trimmed();
    QString gid = fields.at(1).trimmed();
    QString data = fields.at(4).trimmed();
    bytesToSend = encode(line);
    rawStr = QString(bytesToSend);
    //qDebug() << "Byte To Send: " << rawStr;
    QString msg = ts+","+gid+","+data+","+rawStr+","+QString::number(boxGID)+"\n";
    //qDebug() << "Message to Send: " << msg;
    for(int y = 0; y < 6; y++) {
        if(ipArray[y].dataType == "BASE" && !ipArray[y].ip.isEmpty()) {
            QTcpSocket *send= new QTcpSocket;
            send->connectToHost(QHostAddress(ipArray[y].ip), ipArray[y].port);
            if(send->waitForConnected(1000)) {
                send->write(msg);
                send->disconnectFromHost();
                msgCount++;
                color = "green";
            }
        }
    }
    if(toAppend) ui->DataBrowser->append("<font color="+color+">"+line+"</font>");
    ui->rxLabel->setText("Messages Transmitted: "+QString::number(msgCount));
}

void MT500::getConfig()
{
    QString line;
    int count = 0;
    QString filename = "/home/administrator/Desktop/MT500/config.dat";
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count == 4) heartbeatInterval = line.trimmed().toInt(); //Get hb interval
            else if(count == 7) { //Get interest list
                if(!line.trimmed().isEmpty()) {
                    getFiles = line.trimmed().split(",");
                }
            }
            //else if(count == 10) fipsNo = line.trimmed(); //Get fips #
            else if(count == 13) {
                fipsInterval = line.trimmed().toInt();
                //qDebug() << fipsInterval;
                //if(fipsInterval < 5) fipsInterval = 5;
            }
            else if(count == 16) { //COM in
                QStringList comIN;
                comIN = line.trimmed().split(":");
                COMin = comIN.at(0);
                inBaud = comIN.at(1).toInt();
            }
            else if(count == 19) { //COM out
                QStringList comOUT;
                comOUT = line.trimmed().split(":");
                COMout = comOUT.at(0);
                outBaud = comOUT.at(1).toInt();
            }
            else if(count == 22) ipConfig = line.trimmed(); //IP Config
            else if(count == 25) fipsDir = line.trimmed(); //Fips Directory
            //else if(count == 28) boxGID = line.trimmed().toInt();
            else if(count == 31) {
                //qDebug() << "here";
                int begin = 0;
                int end = 0;
                QStringList pairs;
                QStringList range;
                QString temp;
                QString filterStr = line.trimmed();
                //if(log) MTLOG(QString("Pass List String: %1").arg(filterStr));
                QStringList filterArr = filterStr.split(",");
                for(int i = 0; i < filterArr.size(); i++) {
                    temp = filterArr.at(i);
                    pairs = temp.trimmed().split(":");
                    if(pairs.size() > 1) {
                        //if(log) MTLOG(QString("Filter Pair: %1:%2").arg(pairs.at(0)).arg(pairs.at(1)));
                        //qDebug() << pairs.at(0) << ":" << pairs.at(1);
                        if(pairs.at(1).contains("-")) {
                            range = pairs.at(1).trimmed().split("-");
                            begin = range.at(0).trimmed().toInt();
                            end = range.at(1).trimmed().toInt();
                            for(int i = begin; i <= end; i++) {
                                filterList.insertMulti(pairs.at(0).trimmed(), QString::number(i));
                            }
                        }
                        else filterList.insertMulti(pairs.at(0).trimmed(),pairs.at(1).trimmed());
                    }
                }
            }
        }
    }
    else if(log) MTLOG(QString("Failed to open file %1").arg(filename));
    ///////////////////////////////
    count = 0;
    QFile localeInfo("/home/administrator/Desktop/netID.dat");
    if (localeInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&localeInfo);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count == 1)  boxGID = line.trimmed().toInt();
            else if(count == 2) fipsNo = line.trimmed();
        }
    }
    ////////////////////////////////////
    count = 0;
    QFile mac("/home/administrator/Desktop/mac.dat");
    if (mac.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&mac);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count == 1) HWaddr = line.trimmed();
        }
    }
    ui->gidLabel->setText("Network ID: "+QString::number(boxGID));
    ui->inCOMLabel->setText("Incoming COM Port: "+COMin+" @ "+QString::number(inBaud)+"bps (8N1)");
    ui->outCOMLabel->setText("Outgoing COM Port: "+COMout+ " @ "+QString::number(outBaud)+"bps (8N1)");
}

void MT500::sendHB() {
    QString msg;
    QDateTime now = QDateTime::currentDateTime();
    msg = fipsNo+","+now.toString("MM/dd/yyyy hh:mm:ss")+","+QString::number(msgCount)+","+QString::number(boxGID)+","+progVer+","+HWaddr;
    if(log) MTLOG(QString("Hearbeat message to send: %1").arg(msg));
    for(int y = 0; y < 6; y++) {
        if(ipArray[y].dataType == "BASE" && !ipArray[y].ip.isEmpty()) {
            QTcpSocket *send= new QTcpSocket;
            send->connectToHost(QHostAddress(ipArray[y].ip), ipArray[y].port);
            if(send->waitForConnected(2000)) {
                send->write(msg);
                send->disconnectFromHost();
            }
            else if(log) MTLOG(QString("Failed to connect to %1").arg(ipArray[y].ip));

        }
    }
    hbTimer->start(heartbeatInterval*1000*60);
}

void MT500::getFips() {
    int listCounter = 0;
    //fipsFilter.clear();
    //checkResetFIPS();
    if(log) MTLOG("Getting fips files...");
    if(getFiles.size() > 0) {
        for(int i = 0; i < getFiles.size(); i++) {
            QString line;
            //int count = 0;
            QString filename = fipsDir+getFiles.at(i).trimmed()+".dat";
            QFile file(filename);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMap<QDateTime, QString> tempSorter;
                MTLOG(QString("Reading file %1").arg(filename));
                QTextStream in(&file);
                while (!in.atEnd()) {
                    line = in.readLine();
                    QStringList fields2 = line.split(",");
                    QString dateTime = fields2.at(0);
                    QDateTime newDate = QDateTime::fromString(dateTime.trimmed(), "MM/dd/yyyy HH:mm:ss");
                    tempSorter.insertMulti(newDate, line);
                }
                file.close();
                //Loop over sorted file and check for shit
                QMap<QDateTime, QString>::iterator it;
                for(it = tempSorter.begin(); it != tempSorter.end(); ++it) {
                    QStringList recordFields = it.value().split(",");
                    QDateTime dateTime = it.key();
                    QString record = it.value();
                    QMap<QDateTime, QString> recordInfo = fipsCount.value(getFiles.at(i));
                    QMap<QDateTime, QString>::iterator ri = recordInfo.begin();
                    QDateTime oldTime = ri.key();
                    QString oldRecord = ri.value();
                    MTLOG(QString("Old Record: %1 --- New Record: %2").arg(oldRecord).arg(record));
                    if((dateTime >= oldTime) && (record != oldRecord)) {   // If new record
                        if(log) MTLOG(QString("New record: %1").arg(record));
                        QMap<QDateTime, QString> tempMap;
                        tempMap.insert(dateTime, record);
                        fipsCount.insert(getFiles.at(i).trimmed(), tempMap);
                        int gid = recordFields.at(1).trimmed().toInt();
                        int node = recordFields.at(4).trimmed().toInt();
                        if(inFilter(node, gid) == true) {
                            if(log) MTLOG(QString("Record is in pass list: %1 (%2)").arg(record).arg(getFiles.at(i).trimmed()));
                            bool inFipsFilter = fipsFilter.contains(gid);
                            MTLOG (QString("Is %1 in fipFilter? %2").arg(gid).arg(inFipsFilter ? "true" : "false"));
                            if(inFipsFilter == false) {
                                MTLOG (QString("Inserting %1 into fipsFilter!").arg(gid));
                                fipsFilter.insert(gid, dateTime); //Update fips filter
                                sendList.insert(listCounter, dateTime.toString()+"$"+recordFields.at(3).trimmed());
                                listCounter++;
                                MTLOG (QString("Record is passed! %1").arg(record));
                                ui->CloudBrowser->append("<font color=\"green\">" + record + "</font>");
                                cloudCnt++;
                                if(cloudCnt > 500) {
                                    ui->CloudBrowser->clear();
                                    cloudCnt = 0;
                                }
                            }
                            else {
                                QDateTime prevTime = fipsFilter.value(gid); //Get previous time value
                                MTLOG (QString("Time comparison for GID %1: prevTime: %2 --- dateTime: %3").arg(gid).arg(prevTime.toString()).arg(dateTime.toString()));
                                //
                                // dateTime is the timestamp of the record that we're dealing with now
                                // prevTime is the timestamp of the last record with this same GID
                                //
                                // Here we are adding 60 secs to the prevTime and comparing that to dateTime to ensure
                                // that 60 secs has passed, this filters out repeats of the same GID
                                //
                                // Example:
                                // The last time we received a record from 1865 was 15:30:30
                                // prevTime = 15:30:30
                                // It is now 15:30:45 and we have just received another record from 1865
                                // dateTime = 15:30:45
                                // We add 60 secs to prevTime
                                //     prevTime = 15:31:30
                                // Is dateTime > prevTime? (15:30:45 > 15:31:30)
                                //     No! So we don't pass it...
                                //
                                if(dateTime > prevTime.addSecs(60)) { //Only concerned about it if enough time has passed
                                    fipsFilter.insert(gid, dateTime); //Update fips filter
                                    sendList.insert(listCounter, dateTime.toString()+"$"+recordFields.at(3).trimmed());//add to pass list
                                    listCounter++;
                                    MTLOG (QString("Record is passed! %1").arg(record));
                                    ui->CloudBrowser->append("<font color=\"green\">" + record + "</font>");
                                    cloudCnt++;
                                    if(cloudCnt > 500) {
                                        ui->CloudBrowser->clear();
                                        cloudCnt = 0;
                                    }
                                }
                                else MTLOG(QString("Record failed time comparison! %1").arg(record));
                            }
                        }
                        else {
                            if(log) MTLOG(QString("Record NOT in pass list: %1 (%2)").arg(record).arg(getFiles.at(i).trimmed()));
                            ui->CloudBrowser->append("<font color=\"red\">" + record + "</font>");
                            cloudCnt++;
                            if(cloudCnt > 500) {
                                ui->CloudBrowser->clear();
                                cloudCnt = 0;
                            }
                        }
                    }
                }
            }
            else if(log) MTLOG(QString("Error opening file %1").arg(filename));
        }
        sendFips();//Process and send all data in send list
    }
    MTLOG ("Done getting fips!");
}

void MT500::sendFips()
{
    QByteArray bytesToSend;
    bool emory;
    QStringList sorted = sortFips();
    for(int i = 0; i < sorted.size(); i++) {
        QString raw = sorted.at(i);
        QString temp = raw;
        QString byte1 = raw.remove(2,10);
        raw = temp;
        QString byte2 = raw.remove(0, 2);
        byte2 = byte2.remove(2,8);
        raw = temp;
        QString byte3 = raw.remove(0,4);
        byte3 = byte3.remove(2,6);
        raw = temp;
        QString byte4 = raw.remove(0,6);
        byte4 = byte4.remove(2,4);
        raw = temp;
        QString byte5 = raw.remove(0,8);
        byte5 = byte5.remove(2,2);
        raw = temp;
        QString byte6 = raw.remove(0,10);
        //qDebug() << byte1 << byte2 << byte3 << byte4 << byte5 << byte6;
        bytesToSend[0] = byte1.trimmed().toUInt(&emory, 16);
        bytesToSend[1] = byte2.trimmed().toUInt(&emory, 16);
        bytesToSend[2] = byte3.trimmed().toUInt(&emory, 16);
        bytesToSend[3] = byte4.trimmed().toUInt(&emory, 16);
        bytesToSend[4] = byte5.trimmed().toUInt(&emory, 16);
        bytesToSend[5] = byte6.trimmed().toUInt(&emory, 16);
        for(int y = 0; y < 6; y++) {
            if(ipArray[y].dataType == "RAW") { //<-- Probably needs to be handled differently
                QString ip = ipArray[y].ip;
                int port = ipArray[y].port;
                sockets[y]->connectToHost(QHostAddress(ip), port);
                for(int i = 0; i < 6; i++) {
                    sockets[y]->putch(bytesToSend[i]);
                }
                sockets[y]->close();
            }
        }
        //qDebug() << "Fips string: " << QString(bytesToSend.toHex());
        sendIFLOWS(bytesToSend);
    }
}

QStringList MT500::sortFips() //Sorts fips records by time before pushing to IFLOWS
{
    QString raw = "adfd";
    int key = 999;
    QStringList sorted;
    QDateTime oldest = QDateTime::currentDateTime().addMonths(1);
    QHash<int, QString>::iterator q;
    int numRecords = sendList.size();
    while(sorted.size() < numRecords) {
        for(q = sendList.begin(); q != sendList.end(); q++) {
            QString value = q.value();
            QStringList fields = value.split("$");
            QDateTime recTime = QDateTime::fromString(fields.at(0).trimmed(), "ddd MMM d HH:mm:ss yyyy");
            if(recTime == oldest || sendList.size() == 1) { //If 2 records occurred at the same time
                oldest = recTime;
                raw = fields.at(1).trimmed();
                key = q.key();
                break;
            }
            else if(recTime < oldest) { //If record time is older than current oldest
                oldest = recTime;
                raw = fields.at(1).trimmed();
                key = q.key();
            }
        }
        sorted.push_back(raw);
        sendList.remove(key);
        //qDebug() << "Removed " << key;
        oldest = QDateTime::currentDateTime().addMonths(1);
        //qDebug() << oldest.toString();
        /*QHash<int, QString>::iterator y;
        for(y = sendList.begin(); y != sendList.end(); y++) {
            qDebug() << y.key() << ":" << y.value();
        }*/
        //qDebug() << sendList.size();
    }
    //for(int v = 0; v < sorted.size(); v++) qDebug() << sorted.at(v);
    sendList.clear();
    return sorted;
}

void MT500::getFipsCounts()
{
    for(int i = 0; i < getFiles.size(); i++) {
        QString line;
        QMap<QDateTime, QString> tempSorter;
        QString filename = fipsDir+getFiles.at(i).trimmed()+".dat";
        if(log) MTLOG(QString("Getting newest record in %1").arg(filename));
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            while (!in.atEnd()) {
                line = in.readLine();
                QStringList fields = line.split(",");
                QString dateTime = fields.at(0);
                QDateTime newDate = QDateTime::fromString(dateTime.trimmed(), "MM/dd/yyyy HH:mm:ss");
                tempSorter.insertMulti(newDate, line);
            }
            QMap<QDateTime, QString>::iterator it;
            QDateTime newestRecord;
            int ctr = 0;
            for(it = tempSorter.begin(); it != tempSorter.end(); ++it) {
                ctr++;
                if(ctr == tempSorter.size()) {
                    newestRecord = it.key();
                    QMap<QDateTime, QString> tempMap;
                    tempMap.insert(newestRecord, it.value());
                    fipsCount.insert(getFiles.at(i).trimmed(), tempMap);
                }
            }
            if(log) MTLOG(QString("Newest Record for %1: %2").arg(filename).arg(newestRecord.toString()));
        }
        else {
            if(log) MTLOG(QString("Error opening file %1").arg(filename));
            QMap<QDateTime, QString> tempMap;
            tempMap.insert(QDateTime::fromString("01/01/1970 00:00:00", "MM/dd/yyyy HH:mm:ss"), "Empty Placeholder");
            fipsCount.insert(getFiles.at(i).trimmed(), tempMap);
        }
        //QHash<QString, QDateTime>::const_iterator i = fipsCount.constBegin();
        //while (i != fipsCount.constEnd()) {
        //    qDebug() << i.key() << ": " << i.value();
        //    ++i;
        //}
    }
}

void MT500::setupRS232(QString in, QString out)
{
    inPort = new SerialPort(this);
    inPort->setPort(in);
    if (inPort->open(QIODevice::ReadWrite)) {
        inPort->setRate(inBaud);
        inPort->setDataBits(SerialPort::Data8);
        inPort->setParity(SerialPort::NoParity);
        inPort->setStopBits(SerialPort::OneStop);
        inPort->setFlowControl(SerialPort::NoFlowControl);

        if(log) MTLOG(QString("Incoming Port Name: %1").arg(inPort->portName()));
        if(log) MTLOG(QString("Incoming Port Rate: %1").arg(inPort->rate()));
        if(log) MTLOG(QString("Incoming Port Data Bits: %1").arg(inPort->dataBits()));
        if(log) MTLOG(QString("Incoming Port Parity: %1").arg(inPort->parity()));
        if(log) MTLOG(QString("Incoming Port Stop Bits: %1").arg(inPort->stopBits()));
        if(log) MTLOG(QString("Incoming Port Flow Control: %1").arg(inPort->flowControl()));
    }
    else if(log) MTLOG(QString("Failed to open incoming port %1.").arg(in));

    connect(inPort, SIGNAL(readyRead()), this, SLOT(readData()));

    outPort = new SerialPort(this);
    outPort->setPort(out);
    if (outPort->open(QIODevice::ReadWrite)) {
        outPort->setRate(outBaud);
        outPort->setDataBits(SerialPort::Data8);
        outPort->setParity(SerialPort::NoParity);
        outPort->setStopBits(SerialPort::OneStop);
        outPort->setFlowControl(SerialPort::NoFlowControl);

        if(log) MTLOG(QString("Outgoing Port Name: %1").arg(outPort->portName()));
        if(log) MTLOG(QString("Outgoing Port Rate: %1").arg(outPort->rate()));
        if(log) MTLOG(QString("Outgoing Port Data Bits: %1").arg(outPort->dataBits()));
        if(log) MTLOG(QString("Out Port Parity: %1").arg(outPort->parity()));
        if(log) MTLOG(QString("Out Port Stop Bits: %1").arg(outPort->stopBits()));
        if(log) MTLOG(QString("Out Port Flow Control: %1").arg(outPort->flowControl()));
    }
    else if(log) MTLOG(QString("Failed to open outgoing port %1.").arg(out));
}

void MT500::readData()
{
    validMsgTimer->start(1000);
    QString line;
    ba = inPort->readAll();
    //qDebug() << "Byte Read: " + QString::number(ba[0], 16);
    msg[byteCount] = ba[0];
    byteCount++;
    if(byteCount == 4) {
        validMsgTimer->stop();
        line = decode(msg);
        if(line == "NA") {
            validMsgTimer->start(1000);
            byteCount--;
            QByteArray temp;
            temp[0] = msg[1];
            temp[1] = msg[2];
            temp[2] = msg[3];
            msg = temp;
        }
        else {
            byteCount = 0;
            //ui->DataBrowser->append(line.trimmed());
            recordCnt++;
            if(recordCnt > 500){
                ui->DataBrowser->clear();
                recordCnt =0;
            }
            QStringList fields = line.trimmed().split(" ");
            int gid = fields.at(1).trimmed().toInt();
            if(inFilter(boxGID, gid)) {
                sendRaw(line.trimmed());
                sendIFLOWS(msg);
                sendBase(line.trimmed(), true);
            }
            else {
                ui->DataBrowser->append("<font color=blue>"+line.trimmed()+"</font>");
                sendBase(line.trimmed(), false);
            }

        }
    }
}

QString MT500::decode(QByteArray data)
{
    //qDebug() << "Msg Buffer: " + QString::number(data[0], 16) + " " + QString::number(data[1], 16) + " " + QString::number(data[2], 16) + " " + QString::number(data[3], 16);
    int address, event;
    address = data[0] & 0x3F;
    address = ((data[1] & 0x3F) << 6) | address;
    address = ((data[2] & 0x1) << 12) | address;
    event = (data[2] & 0x3F) >> 1;
    event = ((data[3]& 0x3F) << 5) | event;
    QString addStr = QString::number(address);
    if(addStr.toInt() < 1000) addStr = "0"+addStr;
    if(address < 100 || address > 8191 || event < 0 || event > 2047) return "NA";
    else return "A "+addStr+" "+QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss")+" "+QString::number(event);
}

void MT500::sendIFLOWS(QByteArray msg)
{    
    //qDebug() << "Sending IFLOWS";
    //for(int i = 0; i < 4; i++) qDebug() << msg.at(i);
    msg.append(0xC0);
    msg.append(0x03);
    outPort->write(msg);
}

void MT500::on_addButton_clicked()
{
    QString ip = ui->ipLineEdit->text().trimmed();
    QString port = ui->portLineEdit->text().trimmed();
    QString dataType = ui->dataSelect->currentText().trimmed();
    if(dataType == "Processed") dataType = "PROC";
    else if(dataType == "Base Station") dataType = "BASE";
    else if(dataType == "Raw") dataType = "RAW";;
    QFile file(ipConfig);
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << ip << "\t" << port << "\t" << dataType << "\n";
    }
    ui->addLabel->setText("<font color=red>Port Added</font>");
    ui->ipLineEdit->clear();
    ui->portLineEdit->clear();
    ui->dataSelect->setItemText(0,"Raw");
    getIPs();
    initial = true;
    testConnection();
}

void MT500::ipError(int error)
{
    switch(error) {
    case 0:
        ui->DataBrowser->append("<font color=red>"+currentIP+": Connection Refused</font>");
        break;
    case 1:
        ui->DataBrowser->append("<font color=red>"+currentIP+": Host Not Found</font>");
        break;
    case 2:
        ui->DataBrowser->append("<font color=red>"+currentIP+": Socket Read Error</font>");
        break;
    }
}

void MT500::testConnection()
{
    for(int i = 0; i < 6; i++) {
        if(!ipArray[i].ip.isEmpty()){
            QString msg = "[" + QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss") + "] ";
            QTcpSocket * test = new QTcpSocket;
            test->connectToHost(QHostAddress(ipArray[i].ip), ipArray[i].port);
            if(test->waitForConnected(2000)) {
                msg += ipArray[i].ip + ": Test Message Sent";
                ui->testBrowser->append("<font color=green>" + msg + "</font>");
                test->abort();
            }
            else {
                msg += ipArray[i].ip + ": Failed to Connect";
                ui->testBrowser->append("<font color=red>" + msg + "</font>");
            }
        }
    }
    if(!initial) {
        QString fips = fipsNo.trimmed().remove("VAZ");
        QString msgToSend = "A 100 " + QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss") + " " + QString::number(boxGID);
        sendBase(msgToSend, true);
        sendRaw(msgToSend);
        sendRawStrtoIflows(QString(encode(msgToSend)));
    }
    else initial = false;
}


void MT500::on_delButton_clicked()
{
    QFile file(ipConfig);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "File format:\n\n";
        out << "[IP ADDRESS]    [PORT]  [TYPE OF DATA]\n\n";
        out << "[IP ADDRESS] --- Destination IP in the form of xxx.xxx.xxx.xxx\n[PORT] --- Destination port\n[TYPE OF DATA] --- Options: RAW or PROC or BASE\n\n------DO NOT MODIFY ANYTHING ABOVE THIS LINE---------\n";
        for(int i = 0; i < 6; i++) {
            if(!ipArray[i].ip.isEmpty() && ipArray[i].ip != ui->delSelect->currentText().trimmed()) {
                out << ipArray[i].ip + "\t" + QString::number(ipArray[i].port) + "\t" + ipArray[i].dataType + "\n";
            }
        }
    }
    file.close();
    ui->delLabel->setText("<font color=red>Port Deleted</font>");
    getIPs();
    initial = true;
    testConnection();
}


void MT500::on_testButton_clicked()
{
    testConnection();
}

void MT500::resetPorts()
{
    if(log) MTLOG("Resetting COM Ports...");
    inPort->close();
    outPort->close();
    setupRS232(COMin, COMout);
}


void MT500::on_clrButton_clicked()
{
    ui->DataBrowser->clear();
}

void MT500::clrBuf() {
    validMsgTimer->stop();
    byteCount = 0;
    //qDebug() << "Cleared buffer.";
    ba.clear();
}

void MT500::checkResetFIPS()
{
    /*if(getFiles.size() > 0) {
        for(int i = 0; i < getFiles.size(); i++) {
            QString line;
            int count = 0;
            QFile file(fipsDir+getFiles.at(i).trimmed()+".dat");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    line = in.readLine();
                    count++;
                }
                if(count < fipsCount.value(getFiles.at(i).trimmed())) fipsCount.insert(getFiles.at(i).trimmed(), 0);
            }
            else qDebug() << "Error opening file";
        }
    }*/
}

void MT500::on_reconfigureButton_clicked()
{
    ui->updateLbl->setText("<font color=red>Updated</font>");
    inPort->close();
    outPort->close();
    getConfig();
    setupRS232(COMin, COMout);

}

void MT500::sendRawStrtoIflows(QString rawStr)
{
    QByteArray bytesToSend;
    bool emory;
    QString raw = rawStr;
    QString temp = raw;
    QString byte1 = raw.remove(2,10);
    raw = temp;
    QString byte2 = raw.remove(0, 2);
    byte2 = byte2.remove(2,8);
    raw = temp;
    QString byte3 = raw.remove(0,4);
    byte3 = byte3.remove(2,6);
    raw = temp;
    QString byte4 = raw.remove(0,6);
    byte4 = byte4.remove(2,4);
    raw = temp;
    QString byte5 = raw.remove(0,8);
    byte5 = byte5.remove(2,2);
    raw = temp;
    QString byte6 = raw.remove(0,10);
    //qDebug() << byte1 << byte2 << byte3 << byte4 << byte5 << byte6;
    bytesToSend[0] = byte1.trimmed().toUInt(&emory, 16);
    bytesToSend[1] = byte2.trimmed().toUInt(&emory, 16);
    bytesToSend[2] = byte3.trimmed().toUInt(&emory, 16);
    bytesToSend[3] = byte4.trimmed().toUInt(&emory, 16);
    bytesToSend[4] = byte5.trimmed().toUInt(&emory, 16);
    bytesToSend[5] = byte6.trimmed().toUInt(&emory, 16);
    sendIFLOWS(bytesToSend);
}

bool MT500::inFilter(int node, int gid)
{
    bool rValue = false;

    QList<QString> wildCard = filterList.values("*");
    if(wildCard.contains(QString::number(gid)) || wildCard.contains("*")) rValue = true;

    if(!rValue) {
        QList<QString> vals = filterList.values(QString::number(node));
        if(vals.contains(QString::number(gid)) || vals.contains("*")) rValue = true;
    }

    return rValue;
}

void MT500::on_runModeComboBox_currentIndexChanged(int index)
{
    if(index == 0) log = false;
    else log = true;
}
