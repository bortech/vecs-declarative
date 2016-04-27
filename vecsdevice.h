#ifndef VECSDEVICE_H
#define VECSDEVICE_H

#include <QObject>
#include <QBluetoothAddress>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QTimer>

class VecsDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString address READ address NOTIFY stateChanged)
    Q_PROPERTY(qint16 rssi READ rssi NOTIFY stateChanged)
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY stateChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(quint32 singleClickCount READ singleClickCount NOTIFY keyPressed)
    Q_PROPERTY(quint32 doubleClickCount READ doubleClickCount NOTIFY keyPressed)
    Q_PROPERTY(quint32 longClickCount READ longClickCount NOTIFY keyPressed)
    Q_PROPERTY(bool mpuState READ mpuState NOTIFY mpuStateChanged)

    Q_PROPERTY(int mpuRate READ mpuRate WRITE setMpuRate NOTIFY mpuRateChanged)
    Q_PROPERTY(VecsDevice::GyroRange gyroRange READ gyroRange WRITE setGyroRange NOTIFY gyroRangeChanged)
    Q_PROPERTY(VecsDevice::AccelRange accelRange READ accelRange WRITE setAccelRange NOTIFY accelRangeChanged)
    Q_PROPERTY(VecsDevice::DeviceRole role READ role WRITE setRole NOTIFY roleChanged)

    Q_PROPERTY(qint16 accelX READ accelX NOTIFY mpuDataRecieved)
    Q_PROPERTY(qint16 accelY READ accelY NOTIFY mpuDataRecieved)
    Q_PROPERTY(qint16 accelZ READ accelZ NOTIFY mpuDataRecieved)

    Q_PROPERTY(qint16 gyroX READ gyroX NOTIFY mpuDataRecieved)
    Q_PROPERTY(qint16 gyroY READ gyroY NOTIFY mpuDataRecieved)
    Q_PROPERTY(qint16 gyroZ READ gyroZ NOTIFY mpuDataRecieved)

    Q_PROPERTY(quint16 packetIndex READ packetIndex NOTIFY mpuDataRecieved)

public:        
    enum ConnectionState {
        StateDisconnected = 0,
        StateConnecting,
        StateConnected
    };
    Q_ENUM(ConnectionState)

    enum GyroRange {
        GYRO_250DEGS = 0,
        GYRO_500DEGS,
        GYRO_1000DEGS,
        GYRO_2000DEGS
    };
    Q_ENUM(GyroRange)

    enum AccelRange {
        ACC_2G = 0,
        ACC_4G,
        ACC_8G,
        ACC_16G
    };
    Q_ENUM(AccelRange)

    enum ButtonClick {
        SINGLE_CLICK = 1,
        DOUBLE_CLICK,
        LONG_CLICK
    };
    Q_ENUM(ButtonClick)

    enum DeviceRole {
        RoleUndefined = 0,
        RoleDoctor,
        RolePatientHand,
        RolePatientBack
    };
    Q_ENUM(DeviceRole)

    VecsDevice(QObject *parent = 0);
    VecsDevice(const QBluetoothAddress &address, qint16 rssi = 0, QObject *parent = 0);
    ~VecsDevice();

    QString address() const;
    qint16 rssi() const;
    ConnectionState connectionState() const;
    int batteryLevel() const;

    int mpuRate() const;
    bool mpuState() const;
    GyroRange gyroRange() const;
    AccelRange accelRange() const;    

    quint32 singleClickCount() const;
    quint32 doubleClickCount() const;
    quint32 longClickCount() const;

    int interval() const;
    DeviceRole role() const;

// Данные из пакета MPU
    qint16 accelX() const;
    qint16 accelY() const;
    qint16 accelZ() const;
    qint16 gyroX() const;
    qint16 gyroY() const;
    qint16 gyroZ() const;
    quint16 packetIndex() const;    

    int maxReconnections() const;

public slots:
    void connectToDevice();
    void disconnectFromDevice();

    void keyRequest(quint8 delay = 1);
    void mpuStart();
    void mpuStop();

    void setMpuRate(int mpuRate);
    void setGyroRange(VecsDevice::GyroRange gyroRange);
    void setAccelRange(VecsDevice::AccelRange accelRange);

    void setInterval(int interval);
    void setRole(VecsDevice::DeviceRole role);

    void setMaxReconnections(int maxReconnections);

signals:
    void stateChanged();
    void batteryLevelChanged(int level);
    void keyPressed(VecsDevice::ButtonClick type);
    void mpuStateChanged();
    void mpuDataRecieved(const QByteArray &data);
    void mpuRateChanged();
    void gyroRangeChanged();
    void accelRangeChanged();
    void roleChanged();

private slots:
    void deviceConnected();
    void deviceDisconnected();
    void serviceDiscoveryDone();
    void controllerError(QLowEnergyController::Error error);

    void timerJob();

    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void characteristicNotification(const QLowEnergyCharacteristic &c, const QByteArray &v);
    void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &v);

private:
    QLowEnergyService *addService(const QBluetoothUuid &uuid);
    bool enableNotifications(QLowEnergyService *service, const QBluetoothUuid &uuid, bool enable);
    bool writeCharacteristic(QLowEnergyService *service, const QBluetoothUuid &uuid, quint8 value);
    void parseMpuData(const QByteArray &data);

private:
    // UUID наших проприетарных сервисов и характеристик, которые не входят в список стандартных сервисов Bluetooth
    enum ServiceUuid {
        // Сервис кнопки
        KeyService          = 0xffe0,
        CharKeyPressState   = 0xffe1,
        CharKeyRequest      = 0xffe2,
        // Сервис MPU (Motion Processor Unit)
        MpuService          = 0xfff0,
        CharAccelRange      = 0xfff1,
        CharGyroRange       = 0xfff2,
        CharMpuControl      = 0xfff3,
        CharMpuData         = 0xfff4,
        CharMpuTemp         = 0xfff5
    };

    QBluetoothAddress m_address;
    qint16 m_rssi;        

    int m_batteryLevel;

    int m_mpuRate;
    GyroRange m_gyroRange;
    AccelRange m_accelRange;

    ConnectionState m_connectionState;
    bool m_mpuState;

    quint32 m_singleClickCount;
    quint32 m_doubleClickCount;
    quint32 m_longClickCount;

    qint16 m_accelX;
    qint16 m_accelY;
    qint16 m_accelZ;
    qint16 m_gyroX;
    qint16 m_gyroY;
    qint16 m_gyroZ;
    quint16 m_packetIndex;

    DeviceRole m_role;

    bool m_normalDisconnect;
    int m_maxReconnections;
    int m_reconnections;

    QTimer *m_timer;
    QLowEnergyController *m_controller;
    QLowEnergyService *m_batteryService;
    QLowEnergyService *m_keyService;
    QLowEnergyService *m_mpuService;
};

#endif // VECSDEVICE_H
