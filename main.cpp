#include <QCoreApplication>
#include <serialrxtx.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SerialRxTx s;
    /* Load image into byte array */
    QByteArray bytes = s.loadImage("/home/administrator/Desktop/truck.jpeg");
    qDebug() << "Done Loading Image";
    /* Send image using serial protocol */
    QByteArray tmp = s.transmit(bytes);
    qDebug() << "Done Transmitting";
    /* Receive image using serial protocol */
    QByteArray tmp2 = s.receive(tmp);
    qDebug() << "Done Receiving";
    /* Save received image with timestamp */
    if(tmp2 != NULL) {
        qDebug() << "Saving...";
        s.saveImage(tmp2);
    }
    
    return a.exec();
}
