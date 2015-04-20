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
    progVer = "V.013";
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

    //
    // Initially we copy the current files to the previous file
    // directory so that we have something to compare to the first
    // time we sync the FIPS files
    //
    qDebug() << "Starting to copy...";
    //copyProc = new QProcess(this);
    QString oldDir = fipsDir + "old/";
    QString cmd = QString("cp %1*.dat %2").arg(fipsDir).arg(oldDir);
    system(cmd.toStdString().c_str());
    //copyParms << QString("%1*.dat").arg(fipsDir) << oldDir;
    //copyProc->start("cp", copyParms);
    //copyProc->waitForFinished(-1);
    //copyProc->terminate ();
    qDebug() << "Done copying...";

    for(int i = 0; i < 20; i++) {
        sockets[i] = new QTcpSocket(this);
    }
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
    QString msg = ts+","+gid+","+data+","+rawStr+","+QString::number(boxGID)+"\n";
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
            else if(count == 13) {
                fipsInterval = line.trimmed().toInt();
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
    qDebug() << "Getting FIPS....";
    if(getFiles.size() > 0) {
        for(int i = 0; i < getFiles.size(); i++) { //Loop over each file
            QString line;
            QString newFile = fipsDir+getFiles.at(i).trimmed()+".dat";
            QString oldFile = fipsDir+"old/"+getFiles.at(i).trimmed()+".dat";
            if (log) {
                MTLOG (QString("Reading New File: %1").arg(newFile));
                MTLOG (QString("Old File: %1").arg(oldFile));
            }
            //
            // fipsDiff returns a QStringList sorted by time containing any records 
            // that were not in the previously synced file.
            // 
            qDebug() << "Getting new records...";
            QStringList newRecords = fipsDiff (newFile, oldFile);
            qDebug() << "Done getting new records...";

            //
            // We should now have a list of new records
            // Loop through them -- check pass list and if
            // appropriate pass it 
            //
            qDebug() << "New Records Size: " << newRecords.size ();
            for(int i = 0; i < newRecords.size (); i++) {
                QString record = newRecords.at (i).trimmed (); 
                QStringList recordFields = record.trimmed ().split (",");
                qDebug() << "Record Fields Size: " << recordFields.size ();
                int gid = recordFields.at (1).toInt ();
                int node = recordFields.at (4).toInt ();
                qDebug() << "GID: " << gid;
                qDebug() << "Node: " << node;
                bool filter = inFilter (node, gid);
                qDebug() << "Filter: " << filter;
                if(filter == true) {
                    if(log) MTLOG(QString("Record passed: %1").arg(record));
                    ui->CloudBrowser->append("<font color=\"green\">" + record + "</font>");
                    cloudCnt++;
                    if(cloudCnt > 500) {
                        ui->CloudBrowser->clear();
                        cloudCnt = 0;
                    }
                    //
                    // The send list is a QStringList containing
                    // raw data bytes, sorted by time
                    //
                    sendList.append(recordFields.at(3));
                    sendRecord(record);
                }
                else {
                    qDebug() << "Record Not Passed: " << record;
                    //if(log) MTLOG(QString("Record NOT passed: %1 (%2)").arg(record).arg(getFiles.at(i).trimmed()));
                    qDebug() << "Appending to cloud browser";
                    ui->CloudBrowser->append("<font color=\"red\">" + record + "</font>");
                    qDebug() << "Incrementing cloud count";
                    cloudCnt++;
                    if(cloudCnt > 500) {
                        ui->CloudBrowser->clear();
                        cloudCnt = 0;
                    }
                }
            }
        }
    }

    //
    // Copy the current files to the previous file directory
    // so we can keep track of
    qDebug() << "Starting to copy...";
    copyProc->start("cp", copyParms);
    copyProc->waitForFinished(-1);
    copyProc->terminate ();
    qDebug() << "Done copying...";

    sendFips ();
    qDebug() << "Done getting FIPS....";
}

QStringList MT500::fipsDiff (QString newFile, QString oldFile) 
{
    QStringList newRecords;
    //QMap <QDateTime, QString> sortedRecords;

    qDebug() << "Diffing files....";
    // Read in the entire contents of the old file
    QStringList contents;
    QFile file1 (oldFile);
    qDebug() << "Reading " << oldFile;
    if (file1.open (QIODevice::ReadOnly) == true) {
        QTextStream in (&file1);
        while(!in.atEnd()) contents << in.readLine ().trimmed ();
        file1.close ();
    }
    qDebug() << "Done reading " << oldFile;
    
    //QStringList ret;
    qDebug() << "Reading " << newFile;
    QFile file2 (newFile);
    if (file2.open (QIODevice::ReadOnly) == true) {
        QTextStream in (&file2);
        while (!in.atEnd()) {
            QString line = in.readLine();
            //
            // Search old file for record if its not
            // there we're assuming its new
            //
            if(contents.contains(line.trimmed ()) == false) {
                // Put new records in a map for sorting...QMaps are sored automatically by key
                // so we make the key the time and we get a time sorted map....yay!
                //QStringList toks = line.split (",");
                //QDateTime recordTime = QDateTime::fromString (toks[0].trimmed (), "MM/dd/yyyy hh:mm:ss");
                //sortedRecords.insert (recordTime, line.trimmed ());
                qDebug() << "Adding " << line.trimmed () << " to new records...";
                newRecords << line.trimmed ();
            }
        }
        file2.close ();
    }
    qDebug() << "Done reading " << newFile;
   

    //
    // Regurgitate the sorted map and place all
    // record strings (values) into a QStringList
    // to return
    //
    /*QMap<QDateTime, QString>::iterator it;
    for(it = sortedRecords.begin (); it != sortedRecords.end (); ++it) {
        newRecords.append (it.value ());
    }
    newRecords.removeDuplicates ();

    //
    // This section gets rid of all repeats due to repeaters
    // ie. If we get 3 of the same record 1 sec apart this should
    // eliminate all but 1
    // 
    QHash<int, QDateTime> repeaterFilter;
    QStringList finalPass;
    for(int i = 0; i < newRecords.size(); i++) {
        QStringList toks = newRecords.at(i).trimmed().split(",");
        int gid = toks.at(1).toInt ();
        QDateTime recordTime = QDateTime::fromString (toks.at(0).trimmed(), "MM/dd/yyyy hh:mm:ss");
        if(repeaterFilter.contains (gid) == false) { // No records from this gid
            finalPass.append (newRecords.at (i).trimmed ());
            repeaterFilter.insert (gid, recordTime);
        }
        else { // We do have records for this gid we need to make a time comparison and see if we want them
            QDateTime prev = repeaterFilter.value (gid); // Timestamp for prev record
            if(recordTime >= prev.addSecs (60)) { // If at least 1 min has passed then we go ahead pass it through
                repeaterFilter.insert (gid, recordTime);
                finalPass.append (newRecords.at (i).trimmed ());
            }
        }
    }*/
    //QStringList toks = newFile.split("/");
    //QString file = toks.at( toks.size() - 1);
    //toks = file.split(".");
    //QString cmd = QString("diff %1 %2 | sed -n 's/^> \(.*\)/\1/p' > diff_%3.txt").arg(oldFile).arg(newFile).arg(toks.at(0).trimmed());
    //system(cmd.toStdString().c_str());
    //QStringList parms;
    //QString grep = "\'s\/^> \\(.*\\)\/\\1\/p'";
    //if(log) MTLOG(QString("Grep: %1").arg(grep));
    //parms << oldFile << newFile; // << "|" << "sed" << "-n" << grep;
    /*QProcess p;
    p.setProcessChannelMode (QProcess::MergedChannels);
    p.start("diff", parms);
    p.waitForFinished (-1);
    QString out = p.readAllStandardOutput ();
    QStringList toks = out.split("\n");
    QStringList ret;
    for(int i = 0; i < toks.size (); i++) {
        MTLOG(toks[i]);
        if(toks[i].trimmed().contains ("<") == true) {
            ret << toks[i].remove("<").trimmed();
        }
    }*/

    /*if(newRecords.size() > 0) {
        for(int i = 0; i < newRecords.size(); i++) MTLOG(ret[i]);
    }*/

    /*QFile diff ("diff.txt");
    if (diff.open (QIODevice::ReadOnly) == true) {
        QTextStream in (&diff);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if(log) MTLOG (QString("\tAppending Record: %1").arg(line));
            newRecords.append (line.trimmed ());
        }
    }*/
    qDebug() << "Done diffing files.";
    return newRecords;
}

void MT500::sendRecord(QString line)
{
    qDebug() << "Sending Record to Database";

    QByteArray bytesToSend;
    QStringList fields;
    QString rawStr;
    QString msg;

    if(line.contains(','))
    {
       // qDebug() << "Line contains ','";
        QString emess = line;
        emess.replace(',',' ');
        bytesToSend = encode(emess);
            rawStr = QString(bytesToSend);
            msg = "NOT FILTERED,"+line+",END\n";
    }
    else{
      //   qDebug() << "Line contains ' '";
        fields = line.split(" ");
        QString ts = fields.at(2).trimmed()+" "+fields.at(3).trimmed();
        QString gid = fields.at(1).trimmed();
        QString data = fields.at(4).trimmed();
        bytesToSend = encode(line);
        rawStr = QString(bytesToSend);
        msg = "NOT FILTERED,"+ts+","+gid+","+data+","+rawStr+","+QString::number(boxGID)+",END\n";
    }

    QTcpSocket *send= new QTcpSocket;
    send->connectToHost(QHostAddress("72.66.190.194"), 3154);
    if(send->waitForConnected(1000)) {
        send->write(msg);
        send->disconnectFromHost();
        qDebug() << "Sent Record";
    }
    qDebug() << "Function End";
}

void MT500::sendFips()
{
    QByteArray bytesToSend;
    bool emory;
    //QStringList sorted = sortFips();
    for(int i = 0; i < sendList.size(); i++) {
        QString raw = sendList.at(i);
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
        bytesToSend[0] = byte1.trimmed().toUInt(&emory, 16);
        bytesToSend[1] = byte2.trimmed().toUInt(&emory, 16);
        bytesToSend[2] = byte3.trimmed().toUInt(&emory, 16);
        bytesToSend[3] = byte4.trimmed().toUInt(&emory, 16);
        QString hexString = "";
        for(int i = 0; i < 4; i++) {
            hexString += QString("%1").arg(bytesToSend.at(i), 0, 16);
        }
        for(int y = 0; y < 6; y++) {
            if(ipArray[y].dataType == "RAW") { //<-- Probably needs to be handled differently
                QString ip = ipArray[y].ip;
                int port = ipArray[y].port;
                sockets[y]->connectToHost(QHostAddress(ip), port);
                for(int i = 0; i < 4; i++) {
                    sockets[y]->putch(bytesToSend[i]);
                }
                sockets[y]->close();
            }
        }
        sendIFLOWS(bytesToSend);
    }
    sendList. clear ();
}

QStringList MT500::sortFips() //Sorts fips records by time before pushing to IFLOWS
{
    /*QString raw = "adfd";
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
        oldest = QDateTime::currentDateTime().addMonths(1);
    }
    sendList.clear();
    return sorted;*/
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
    ba.clear();
}

void MT500::checkResetFIPS()
{
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
    bytesToSend[0] = byte1.trimmed().toUInt(&emory, 16);
    bytesToSend[1] = byte2.trimmed().toUInt(&emory, 16);
    bytesToSend[2] = byte3.trimmed().toUInt(&emory, 16);
    bytesToSend[3] = byte4.trimmed().toUInt(&emory, 16);
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
