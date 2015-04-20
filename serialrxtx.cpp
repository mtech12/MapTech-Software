#include "serialrxtx.h"

SerialRxTx::SerialRxTx()
{
    m_frame = 0x7E; /* Framing byte */
    m_address = 0xAA; /* Random address, not really needed for a single pair */
    m_control = 0x7D; /* Control byte also escape byte */
}

QByteArray SerialRxTx::transmit(QByteArray dataToSend)
{
    /*
     * Loop over dataToSend byte by byte and check for the occurence
     * of the framing byte or the escape byte. If either is found
     * we insert an escape byte in front of it and invert bit 5
     * of the original octect.
     *
     */
    for(int i = 0; i < dataToSend.size(); i++) {
        if(dataToSend.at(i) == m_frame || dataToSend.at(i) == m_control) {
            dataToSend[i] = dataToSend[i] ^ 0x20;
            dataToSend.insert(i, m_control);
        }
    }

    /*
     * 16-bit CRC is computed over the address,
     * control, and dataToSend
     *
     */
    QByteArray toCRC;
    toCRC.append(m_address);
    toCRC.append(m_control);
    toCRC.append(dataToSend);
    /*
     * Compute CRC
     *
     */
    quint16 crc1 = qChecksum(toCRC.data(), toCRC.length());
    qDebug() << QString("CRC Transmit: %1").arg(crc1, 0, 16);
    unsigned char high = crc1 >> 8;
    unsigned char low =  crc1 & 0x00FF;

    /*
     * Complete frame to send.
     *
     */
    QByteArray toSend;
    toSend.append(m_frame); /* Framing byte */
    toSend.append(m_address); /* Random address */
    toSend.append(m_control); /* Control (escape) byte */
    toSend.append(dataToSend); /* data to send */
    toSend.append(high); /* High byte of 16-bit CRC */
    toSend.append(low); /* Low byte of 16-bit CRC */
    toSend.append(m_frame); /* Framing byte */

    // Loop over toSend and send byte by byte to 
    // serial port

    return toSend;
}

QByteArray SerialRxTx::receive(QByteArray recvFrame)
{
    int sizeOfRecv = recvFrame.size();
    /*
     * Extract received CRC
     *
     */
    unsigned char high = recvFrame.at(sizeOfRecv - 3);
    unsigned char low = recvFrame.at(sizeOfRecv - 2);
    quint16 crc = high << 8 | low;
    qDebug() << QString("CRC Received: %1").arg(crc, 0, 16);
    QByteArray dataRecv;
    /*
     * Extract data from recieved frame
     *
     */
    for(int i = 3; i < sizeOfRecv - 3; i++) {
        dataRecv.append(recvFrame.at(i));
    }
    /*
     * Compute CRC
     *
     */
    QByteArray toCRC;
    toCRC.append(m_address);
    toCRC.append(m_control);
    toCRC.append(dataRecv);
    quint16 crc1 = qChecksum(toCRC.data(), toCRC.length());
    qDebug() << QString("Checksum Calculated: %1").arg(crc1, 0, 16);
    /*
     * If the received CRC = the calculated CRC then
     * return the data else return NULL
     *
     */
    if(crc == crc1) {
        /*
         * Undo escaping of data
         *
         */
        qDebug() << "Checksums match!";
        qDebug() << "Data Recv Size: " << dataRecv.size();
        for(int i = 0; i < dataRecv.size(); i++) {
            if(dataRecv.at(i) == m_control) {
                dataRecv[i+1] = dataRecv[i+1] ^ 0x20;
                dataRecv.remove(i,1);
            }
        }
        qDebug() << "Done Reading File In.";
        return dataRecv;
    }
    else return NULL;
}

bool SerialRxTx::saveImage(QByteArray data, QString savePath)
{
    /*
     * Extract timestamp from data
     *
     */
    QByteArray time = data.mid(0, 17);
    qDebug() << "Date/Time: " << time.data();
    /*
     * Create filename from timestamp
     *
     */
    QString filename = savePath + time.data();
    qDebug() << "Filename: " << filename;
    data = data.right(data.size() - 17);
    /*
     * Create image from byte array
     * and save it
     *
     */
    QImage image;
    image.loadFromData(data);
    return image.save(filename, "JPEG");
}

QByteArray SerialRxTx::loadImage(QString imageToLoad)
{
    /*
     * Load image into byte array
     *
     */
    QByteArray bytes;
    QImage image;
    if(!image.load(imageToLoad)) qDebug() << "Failed to load image!";
    else {
        QBuffer buffer (&bytes);
        buffer.open (QBuffer::WriteOnly);
        image.save (&buffer, "JPEG");
    }
    /*
     * Prepend timestamp to front of image
     * Format: yyyyMMddThh:mm:ss
     *
     */
    QDateTime now = QDateTime::currentDateTime();
    bytes.prepend(now.toString("yyyyMMddThh:mm:ss").toStdString().c_str());
    return bytes;
}
