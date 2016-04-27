#ifndef VECSCONTROLLER_H
#define VECSCONTROLLER_H

#include <QObject>
#include <QAbstractListModel>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include "vecsdevice.h"

class VecsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(QVariant model READ model NOTIFY devicesUpdated)
    Q_PROPERTY(bool discovering READ discovering NOTIFY stateChanged)

public:
    VecsController(QObject *parent = 0);
    ~VecsController();

    bool discovering() const;
    QString message() const;

    QVariant model() const;
    QList<VecsDevice *> devices() const;


public slots:
    void startScan();
    void setMessage(const QString &message);
    void startSession();

private slots:
    void addDevice(const QBluetoothDeviceInfo&device);
    void scanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);

signals:
    void messageChanged();
    void devicesUpdated();
    void stateChanged();

private:
    bool m_discovering;   
    QString m_message;

    QList<VecsDevice *> m_devices;
    QBluetoothDeviceDiscoveryAgent *m_agent;
};

#endif // VECSCONTROLLER_H
