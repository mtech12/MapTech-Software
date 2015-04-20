#include "mt500.h"
#include "ui_mt500.h"
#include "logger.h"

MT500::MT500(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MT500)
{
    m_log = true;
    if(m_log) MTLOG("MT500 Object Constructed");
    ui->setupUi(this);
    //Set program version here /////////
    m_progVer = "V.012";
    ////////////////////////////////////
    m_db = new DatabaseModule("localhost", "root", "m@ptech", this);
    if(m_log) MTLOG("DatabaseModule Object Constructed");

    //Setup receive server /////////////
    m_server = new QTcpServer();
    m_server->listen(QHostAddress::Any, 3154);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slot_dataReceived()));
    ////////////////////////////////////

    ui->versionLbl->setText("Program Version: " + m_progVer);
    ui->rxLabel->setText("Messages Transmitted: 0");
    m_initial = false;
    ui->ipTable->setColumnWidth(0, 120);
    getConfig();
    m_byteCount = 0;
    m_recordCnt = 0;
    m_cloudCnt = 0;
    setupRS232(m_COMin, m_COMout);
    m_resetTimer = new QTimer(this);
    connect(m_resetTimer, SIGNAL(timeout()), this, SLOT(slot_resetPorts()));
    m_validMsgTimer = new QTimer(this);
    connect(m_validMsgTimer, SIGNAL(timeout()), this, SLOT(slot_clrBuf()));
    m_hbTimer = new QTimer(this);
    connect(m_hbTimer, SIGNAL(timeout()), this, SLOT(slot_sendHB()));
    m_msgCount = 0; //Does not include heartbeat messages
    getIPs();
    for(int i = 0; i < 20; i++) {
        m_sockets[i] = new QTcpSocket(this);
    }
    if(m_log) MTLOG(QString("HB Interval: %1 min").arg(m_heartbeatInterval));
    if(m_log) MTLOG(QString("Fips Retrieval Interval: %1 min").arg(m_fipsInterval));
    m_hbTimer->start(m_heartbeatInterval*1000*60);
    m_resetTimer->start(3600000 * 3); //Reset COM ports every 3 hours
    slot_sendHB();
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

void MT500::getIPs()
{
    for(int i = 0; i < 6; i++) m_ipArray[i].ip = "";
    ui->delSelect->clear();
    for(int col = 0; col < 3; col++) {
        for(int row = 0; row < 6; row++) {
            ui->ipTable->setItem(row, col, new QTableWidgetItem(""));
        }
    }
    QString line;
    QStringList fields;
    int count = 0;
    QFile file(m_ipConfig);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count > 9 && count <= 15) { //Skip header && only allow 6 entries
                fields = line.split('\t');
                m_ipArray[count-10].ip = fields.at(0).trimmed();
                m_ipArray[count-10].port = fields.at(1).trimmed().toInt();
                m_ipArray[count-10].dataType = fields.at(2).trimmed();
                ui->ipTable->setItem(count-10, 0, new QTableWidgetItem(m_ipArray[count-10].ip));
                ui->ipTable->setItem(count-10, 1, new QTableWidgetItem(QString::number(m_ipArray[count-10].port)));
                ui->ipTable->setItem(count-10, 2, new QTableWidgetItem(m_ipArray[count-10].dataType));
                ui->delSelect->addItem(m_ipArray[count-10].ip);
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
    QStringList fields = gid.split(" ");
    int gid_int = fields.at(1).trimmed().toInt();
    int data_int = fields.at(4).trimmed().toInt();
    unsigned int byte1 = (gid_int & 0x0000003F) | 0x00000040;
    returnVal.append(QString::number(byte1, 16));
    unsigned int byte2 = ((gid_int >> 6) & 0x0000003F) | 0x00000040;
    returnVal.append(QString::number(byte2, 16));
    unsigned int byte3 = ((gid_int >> 12) & 0x00000001) | ((data_int << 1) & 0x0000003E) | 0x000000C0;
    returnVal.append(QString::number(byte3, 16));
    unsigned int byte4 = ((data_int >> 5) & 0x0000003F) | 0x000000C0;
    returnVal.append(QString::number(byte4, 16));
    returnVal.append("C0");
    returnVal.append("03");
    return returnVal;
}

void MT500::sendRaw(QString line)  //<--- Probably needs to be handled differently
{
    QByteArray bytesToSend;
    for(int y = 0; y < 6; y++) {
        if(m_ipArray[y].ip != "") {
            if(m_ipArray[y].dataType == "RAW" && !m_ipArray[y].ip.isEmpty()) {
                QString ip = m_ipArray[y].ip;
                int port = m_ipArray[y].port;
                m_sockets[y]->connectToHost(QHostAddress(ip), port);
                bytesToSend = encode(line);
                for(int i = 0; i < 6; i++) {
                    m_sockets[y]->putch(bytesToSend[i]);
                }
                m_sockets[y]->close();
                m_msgCount++;
            }
        }
    }
    ui->rxLabel->setText("Messages Transmitted: "+QString::number(m_msgCount));
}

void MT500::sendBase(QString line, bool toAppend)
{
    QString color = "red";
    m_socketCtr++;
    if(m_socketCtr == 20) m_socketCtr = 0;
    if(m_log) MTLOG(QString("Sending record to servers: %1").arg(line));
    QByteArray bytesToSend;
    QStringList fields;
    QString rawStr;
    fields = line.split(" ");
    QString ts = fields.at(2).trimmed()+" "+fields.at(3).trimmed();
    QString gid = fields.at(1).trimmed();
    QString data = fields.at(4).trimmed();
    bytesToSend = encode(line);
    rawStr = QString(bytesToSend);
    QString msg = ts+","+gid+","+data+","+rawStr+","+QString::number(m_boxGID)+"\n";
    for(int y = 0; y < 6; y++) {
        if(m_ipArray[y].dataType == "BASE" && !m_ipArray[y].ip.isEmpty()) {
            QTcpSocket *send= new QTcpSocket;
            send->connectToHost(QHostAddress(m_ipArray[y].ip), m_ipArray[y].port);
            if(send->waitForConnected(1000)) {
                send->write(msg);
                send->disconnectFromHost();
                m_msgCount++;
                color = "green";
            }
        }
    }
    if(toAppend) ui->DataBrowser->append("<font color="+color+">"+line+"</font>");
    ui->rxLabel->setText("Messages Transmitted: "+QString::number(m_msgCount));
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
            if(count == 4) m_heartbeatInterval = line.trimmed().toInt(); //Get hb interval
            else if(count == 7) { //Get interest list
                if(!line.trimmed().isEmpty()) {
                    m_getFiles = line.trimmed().split(",");
                }
            }
            else if(count == 13) {
                m_fipsInterval = line.trimmed().toInt();
                //qDebug() << fipsInterval;
                if(m_fipsInterval < 5) m_fipsInterval = 5;
            }
            else if(count == 16) { //COM in
                QStringList comIN;
                comIN = line.trimmed().split(":");
                m_COMin = comIN.at(0);
                m_inBaud = comIN.at(1).toInt();
            }
            else if(count == 19) { //COM out
                QStringList comOUT;
                comOUT = line.trimmed().split(":");
                m_COMout = comOUT.at(0);
                m_outBaud = comOUT.at(1).toInt();
            }
            else if(count == 22) m_ipConfig = line.trimmed(); //IP Config
            else if(count == 25) m_fipsDir = line.trimmed(); //Fips Directory
            else if(count == 31) {
                int begin = 0;
                int end = 0;
                QStringList pairs;
                QStringList range;
                QString temp;
                QString filterStr = line.trimmed();
                QStringList filterArr = filterStr.split(",");
                for(int i = 0; i < filterArr.size(); i++) {
                    temp = filterArr.at(i);
                    pairs = temp.trimmed().split(":");
					if(pairs.size() > 1) {
						if(pairs.at(1).contains("-")) {
							range = pairs.at(1).trimmed().split("-");
							begin = range.at(0).trimmed().toInt();
							end = range.at(1).trimmed().toInt();
							for(int i = begin; i <= end; i++) {
                                m_filterList.insertMulti(pairs.at(0).trimmed(), QString::number(i));
							}
						}
                        else m_filterList.insertMulti(pairs.at(0).trimmed(),pairs.at(1).trimmed());
					}
                }
            }
        }
    }
    else if(m_log) MTLOG(QString("Failed to open file %1").arg(filename));
    ///////////////////////////////
    count = 0;
    QFile localeInfo("/home/administrator/Desktop/netID.dat");
    if (localeInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&localeInfo);
        while (!in.atEnd()) {
            line = in.readLine();
            count++;
            if(count == 1)  m_boxGID = line.trimmed().toInt();
            else if(count == 2) m_fipsNo = line.trimmed();
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
            if(count == 1) m_HWaddr = line.trimmed();
        }
    }
    ui->gidLabel->setText("Network ID: "+QString::number(m_boxGID));
    ui->inCOMLabel->setText("Incoming COM Port: "+m_COMin+" @ "+QString::number(m_inBaud)+"bps (8N1)");
    ui->outCOMLabel->setText("Outgoing COM Port: "+m_COMout+ " @ "+QString::number(m_outBaud)+"bps (8N1)");
}

void MT500::slot_sendHB() {
    QString msg;
    QDateTime now = QDateTime::currentDateTime();
    msg = m_fipsNo+","+now.toString("MM/dd/yyyy hh:mm:ss")+","+QString::number(m_msgCount)+","+QString::number(m_boxGID)+","+m_progVer+","+m_HWaddr;
    if(m_log) MTLOG(QString("Hearbeat message to send: %1").arg(msg));
    for(int y = 0; y < 6; y++) {
        if(m_ipArray[y].dataType == "BASE" && !m_ipArray[y].ip.isEmpty()) {
            QTcpSocket *send= new QTcpSocket;
            send->connectToHost(QHostAddress(m_ipArray[y].ip), m_ipArray[y].port);
            if(send->waitForConnected(2000)) {
                send->write(msg);
                send->disconnectFromHost();
            }
            else if(m_log) MTLOG(QString("Failed to connect to %1").arg(m_ipArray[y].ip));

        }
    }
    m_hbTimer->start(m_heartbeatInterval*1000*60);
}

void MT500::setupRS232(QString in, QString out)
{
    m_inPort = new SerialPort(this);
    m_inPort->setPort(in);
    if (m_inPort->open(QIODevice::ReadWrite)) {
        m_inPort->setRate(m_inBaud);
        m_inPort->setDataBits(SerialPort::Data8);
        m_inPort->setParity(SerialPort::NoParity);
        m_inPort->setStopBits(SerialPort::OneStop);
        m_inPort->setFlowControl(SerialPort::NoFlowControl);

        if(m_log) MTLOG(QString("Incoming Port Name: %1").arg(m_inPort->portName()));
        if(m_log) MTLOG(QString("Incoming Port Rate: %1").arg(m_inPort->rate()));
        if(m_log) MTLOG(QString("Incoming Port Data Bits: %1").arg(m_inPort->dataBits()));
        if(m_log) MTLOG(QString("Incoming Port Parity: %1").arg(m_inPort->parity()));
        if(m_log) MTLOG(QString("Incoming Port Stop Bits: %1").arg(m_inPort->stopBits()));
        if(m_log) MTLOG(QString("Incoming Port Flow Control: %1").arg(m_inPort->flowControl()));
    }
    else if(m_log) MTLOG(QString("Failed to open incoming port %1.").arg(in));

    connect(m_inPort, SIGNAL(readyRead()), this, SLOT(slot_readData()));

    m_outPort = new SerialPort(this);
    m_outPort->setPort(out);
    if (m_outPort->open(QIODevice::ReadWrite)) {
        m_outPort->setRate(m_outBaud);
        m_outPort->setDataBits(SerialPort::Data8);
        m_outPort->setParity(SerialPort::NoParity);
        m_outPort->setStopBits(SerialPort::OneStop);
        m_outPort->setFlowControl(SerialPort::NoFlowControl);

        if(m_log) MTLOG(QString("Outgoing Port Name: %1").arg(m_outPort->portName()));
        if(m_log) MTLOG(QString("Outgoing Port Rate: %1").arg(m_outPort->rate()));
        if(m_log) MTLOG(QString("Outgoing Port Data Bits: %1").arg(m_outPort->dataBits()));
        if(m_log) MTLOG(QString("Out Port Parity: %1").arg(m_outPort->parity()));
        if(m_log) MTLOG(QString("Out Port Stop Bits: %1").arg(m_outPort->stopBits()));
        if(m_log) MTLOG(QString("Out Port Flow Control: %1").arg(m_outPort->flowControl()));
    }
    else if(m_log) MTLOG(QString("Failed to open outgoing port %1.").arg(out));
}

void MT500::slot_readData()
{
    m_validMsgTimer->start(1000);
    QString line;
    m_ba = m_inPort->readAll();
    m_msg[m_byteCount] = m_ba[0];
    m_byteCount++;
    if(m_byteCount == 4) {
        m_validMsgTimer->stop();
        line = decode(m_msg);
        if(line == "NA") {
            m_validMsgTimer->start(1000);
            m_byteCount--;
            QByteArray temp;
            temp[0] = m_msg[1];
            temp[1] = m_msg[2];
            temp[2] = m_msg[3];
            m_msg = temp;
        }
        else {
            m_byteCount = 0;
            //ui->DataBrowser->append(line.trimmed());
            m_recordCnt++;
            if(m_recordCnt > 500){
                ui->DataBrowser->clear();
                m_recordCnt =0;
            }
            QStringList fields = line.trimmed().split(" ");
            int gid = fields.at(1).trimmed().toInt();
            if(inFilter(m_boxGID, gid)) {
                sendRaw(line.trimmed());
                sendIFLOWS(m_msg);
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
    msg.append(0xC0);
    msg.append(0x03);
    m_outPort->write(msg);
}

void MT500::on_addButton_clicked()
{
    QString ip = ui->ipLineEdit->text().trimmed();
    QString port = ui->portLineEdit->text().trimmed();
    QString dataType = ui->dataSelect->currentText().trimmed();
    if(dataType == "Processed") dataType = "PROC";
    else if(dataType == "Base Station") dataType = "BASE";
    else if(dataType == "Raw") dataType = "RAW";;
    QFile file(m_ipConfig);
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
    m_initial = true;
    testConnection();
}

void MT500::slot_ipError(int error)
{
    switch(error) {
    case 0:
        ui->DataBrowser->append("<font color=red>"+m_currentIP+": Connection Refused</font>");
        break;
    case 1:
        ui->DataBrowser->append("<font color=red>"+m_currentIP+": Host Not Found</font>");
        break;
    case 2:
        ui->DataBrowser->append("<font color=red>"+m_currentIP+": Socket Read Error</font>");
        break;
    }
}

void MT500::testConnection()
{
    for(int i = 0; i < 6; i++) {
        if(!m_ipArray[i].ip.isEmpty()){
            QString msg = "[" + QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss") + "] ";
            QTcpSocket * test = new QTcpSocket;
            test->connectToHost(QHostAddress(m_ipArray[i].ip), m_ipArray[i].port);
            if(test->waitForConnected(2000)) {
                msg += m_ipArray[i].ip + ": Test Message Sent";
                ui->testBrowser->append("<font color=green>" + msg + "</font>");
                test->abort();
            }
            else {
                msg += m_ipArray[i].ip + ": Failed to Connect";
                ui->testBrowser->append("<font color=red>" + msg + "</font>");
            }
        }
    }
    if(!m_initial) {
        QString fips = m_fipsNo.trimmed().remove("VAZ");
        QString msgToSend = "A 100 " + QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss") + " " + QString::number(m_boxGID);
        sendBase(msgToSend, true);
        sendRaw(msgToSend);
        sendRawStrtoIflows(QString(encode(msgToSend)));
    }
    else m_initial = false;
}


void MT500::on_delButton_clicked()
{
    QFile file(m_ipConfig);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "File format:\n\n";
        out << "[IP ADDRESS]    [PORT]  [TYPE OF DATA]\n\n";
        out << "[IP ADDRESS] --- Destination IP in the form of xxx.xxx.xxx.xxx\n[PORT] --- Destination port\n[TYPE OF DATA] --- Options: RAW or PROC or BASE\n\n------DO NOT MODIFY ANYTHING ABOVE THIS LINE---------\n";
        for(int i = 0; i < 6; i++) {
            if(!m_ipArray[i].ip.isEmpty() && m_ipArray[i].ip != ui->delSelect->currentText().trimmed()) {
                out << m_ipArray[i].ip + "\t" + QString::number(m_ipArray[i].port) + "\t" + m_ipArray[i].dataType + "\n";
            }
        }
    }
    file.close();
    ui->delLabel->setText("<font color=red>Port Deleted</font>");
    getIPs();
    m_initial = true;
    testConnection();
}


void MT500::on_testButton_clicked()
{
    testConnection();
}

void MT500::slot_resetPorts()
{
    if(m_log) MTLOG("Resetting COM Ports...");
    m_inPort->close();
    m_outPort->close();
    setupRS232(m_COMin, m_COMout);
}


void MT500::on_clrButton_clicked()
{
    ui->DataBrowser->clear();
}

void MT500::slot_clrBuf() {
    m_validMsgTimer->stop();
    m_byteCount = 0;
    //qDebug() << "Cleared buffer.";
    m_ba.clear();
}

void MT500::on_reconfigureButton_clicked()
{
    ui->updateLbl->setText("<font color=red>Updated</font>");
    m_inPort->close();
    m_outPort->close();
    getConfig();
    setupRS232(m_COMin, m_COMout);

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

    QList<QString> wildCard = m_filterList.values("*");
    if(wildCard.contains(QString::number(gid)) || wildCard.contains("*")) rValue = true;

    if(!rValue) {
        QList<QString> vals = m_filterList.values(QString::number(node));
        if(vals.contains(QString::number(gid)) || vals.contains("*")) rValue = true;
    }

    return rValue;
}

void MT500::on_runModeComboBox_currentIndexChanged(int index)
{
    if(index == 0) m_log = false;
    else m_log = true;
}

// Here we want to put the received record into a dbase AND
// write to a Fips files for legacy support...also we want
// to push it serially to a computer connected to the MT500
void MT500::slot_dataReceived()
{
    MTLOG ("Data Received!");
    QString recordDir = "/home/administrator/Desktop/fipsFiles/";
    QDir dir;
    dir.mkpath(recordDir); //Ensure directory exists

    QString heartbeatDir = recordDir.append("heartbeat/");
    dir.mkpath(heartbeatDir); //Ensure directory exists

    QTcpSocket* newConnection = m_server->nextPendingConnection();
    newConnection->waitForReadyRead();
    QString record = newConnection->readAll().data();
    QStringList recordFields = record.split(",");
    QDateTime recordTime = QDateTime::currentDateTime();
    QString dateStr = recordTime.toString("MM/dd/yyyy HH:mm:ss");
    QString recordStr, filename;
    if(recordFields[0].contains("VAZ") == true) { // Heartbeat
        m_db->insertHeartbeat(record); // Insert heartbeat into dbase
        recordStr = QString("%1,%2,%3,%4,%5,%6\n").arg(recordFields[0].trimmed()).arg(dateStr)
                .arg(recordFields[2].trimmed()).arg(recordFields[3].trimmed())
                .arg(recordFields[4].trimmed()).arg(recordFields[5].trimmed());
        filename = QString("%1%2HB.dat").arg(heartbeatDir).arg(recordFields[0]);
    }
    else { // Actual data record
        if(m_log) MTLOG(QString("New record: %1").arg(record));
        int gid = recordFields[1].trimmed().toInt();
        int node = recordFields[4].trimmed().toInt();
        QString raw = recordFields[3].trimmed();
        QString code = m_db->getCountyCode(gid);
        recordStr = QString("%1,%2,%3,%4,%5\n").arg(dateStr).arg(gid).arg(recordFields[2].trimmed())
                .arg(recordFields[3].trimmed()).arg(node);
        if(inFilter (node,gid) == true) {
            if(m_log) MTLOG(QString("Record passed: %1 (%2)").arg(recordStr).arg(code));
            ui->CloudBrowser->append("<font color=\"green\">" + recordStr + "</font>");
            m_cloudCnt++;
            if(m_cloudCnt > 500) {
                ui->CloudBrowser->clear();
                m_cloudCnt = 0;
            }
            if(m_fipsFilter.contains(gid) == false) {
                m_fipsFilter.insert(gid, recordTime); //Update fips filter
                sendRawStrtoIflows(raw);
            }
            else {
                QDateTime prevTime = m_fipsFilter.value(gid); //Get previous time value
                if(recordTime.addSecs(60) < prevTime) { //Only concerned about it if enough time has passed
                    m_fipsFilter.insert(gid, recordTime); //Update fips filter
                    sendRawStrtoIflows(raw);
                }
            }
        }
        else {
            if(m_log) MTLOG(QString("Record NOT passed: %1 (%2)").arg(recordStr).arg(code));
            ui->CloudBrowser->append("<font color=\"red\">" + recordStr + "</font>");
            m_cloudCnt++;
            if(m_cloudCnt > 500) {
                ui->CloudBrowser->clear();
                m_cloudCnt = 0;
            }
        }
        if(m_log) MTLOG(QString("Record to Insert: %1").arg(record));
        m_db->insertRecord(record); // Insert record into dbase
        filename = QString("%1%2.dat").arg(recordDir).arg(code);
    }

    //Write to fips files OR heartbeat files
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << recordStr;
    }
    else MTLOG("Failed to write to Fips File...");
    file.close();
}
