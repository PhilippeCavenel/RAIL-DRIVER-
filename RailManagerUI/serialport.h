#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include <QList>


class SerialPortSender : public QThread
{
    Q_OBJECT

public:
    explicit SerialPortSender(QObject *parent = nullptr);
    ~SerialPortSender();

    void send(const QString &portName, int waitTimeout, const QString &request);

signals:
    void response(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);

private:
    void run() override;

    QString m_portName;
    QList<QString> m_request;
    int m_waitTimeout = 0;
    QMutex m_mutex;
    QWaitCondition m_cond;
    bool m_quit = false;
};

#endif // SERIALPORT_H
