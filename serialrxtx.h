#ifndef SERIALRXTX_H
#define SERIALRXTX_H

#include <QByteArray>
#include <QImage>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>

/*
 *
 * Very simple implementation of the HDLC data protocol.
 * http://en.wikipedia.org/wiki/High-Level_Data_Link_Control
 *
 * Frame:
 *    [frame 8-bits][address 8-bits][control 8-bits][data 0-n bits][CRC 16-bits][frame 8-bits]
 *
 */

#define SAVEPATH "/home/administrator/Desktop/"

class SerialRxTx
{
public:
    SerialRxTx();

    /*
     * Returns a QByteArray with computed CRC16
     * and correct framing sequence
     */
    QByteArray transmit(QByteArray dataToSend);

    /*
     * Returns data from received frame if CRC16
     * matches else returns a null QByteArray
     */
    QByteArray receive (QByteArray recvFrame);

    /*
     * Function responsible for saving image
     * from received data
     */
    bool saveImage(QByteArray data, QString savePath = SAVEPATH);

    /*
     * Loads image into a byte array ready for
     * transmitting
     */
    QByteArray loadImage(QString imageToLoad);

private:

    unsigned char m_frame, m_address, m_control;
};

#endif // SERIALRXTX_H
