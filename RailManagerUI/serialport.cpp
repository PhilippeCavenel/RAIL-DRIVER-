#include "serialport.h"

SerialPortSender::SerialPortSender(QObject *parent) :
    QThread(parent)
{
}

SerialPortSender::~SerialPortSender()
{
    m_quit = true;
    m_cond.wakeOne();
    wait();
}

void SerialPortSender::send(const QString &portName, int waitTimeout, const QString &request)
{
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    m_request.push_back(request);
    if (!isRunning()) {
        start();
    }
    else m_cond.wakeOne();
}

void SerialPortSender::run()
{
    // OPEN
    if (m_portName.isEmpty()) {
        emit error(tr("No port name specified"));
        return;
    }
    QSerialPort serial;
    serial.setPortName(m_portName);
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
         emit error(tr("Can't open %1, error code %2").arg(m_portName).arg(serial.error()));
         return;
    }

    // WRITE
    while (!m_quit) {
        // write request
        while (!m_request.empty()) {
            const QByteArray requestData =m_request.takeFirst().toUtf8();
            serial.write(requestData);
            qDebug() << requestData;
            if (!serial.waitForBytesWritten(m_waitTimeout)) {
                emit timeout(tr("Wait write request timeout %1")
                                 .arg(QTime::currentTime().toString()));
            }
            // read response
            if (serial.waitForReadyRead(m_waitTimeout)) {
                QByteArray responseData = serial.readAll();
                qDebug() << "serial read";
                while (serial.waitForReadyRead(m_waitTimeout)) responseData += serial.readAll();
                const QString response = QString::fromUtf8(responseData);
                emit this->response(response);
             } else {
                 emit timeout(tr("Wait read response timeout %1").arg(QTime::currentTime().toString()));
                 break;
            }
        }
       m_cond.wait(&m_mutex);
    }
}

