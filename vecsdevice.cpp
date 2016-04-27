#include "vecsdevice.h"
#include <QDebug>
#include <QDataStream>

VecsDevice::VecsDevice(QObject *parent) :
    QObject(parent)
{
    qDebug() << "Instance of VecsDevice is created";
}

VecsDevice::VecsDevice(const QBluetoothAddress &address, qint16 rssi, QObject *parent) :
    QObject(parent),
    m_address(address),
    m_rssi(rssi),    
    m_batteryLevel(100),
    m_mpuRate(100),
    m_gyroRange(VecsDevice::GYRO_250DEGS),
    m_accelRange(VecsDevice::ACC_2G),
    m_connectionState(StateDisconnected),
    m_singleClickCount(0),
    m_doubleClickCount(0),
    m_longClickCount(0),
    m_role(VecsDevice::RoleUndefined),
    m_normalDisconnect(true),
    m_maxReconnections(3),
    m_reconnections(0),
    m_timer(nullptr),
    m_controller(nullptr),
    m_batteryService(nullptr),
    m_keyService(nullptr),
    m_mpuService(nullptr)
{
    m_accelX = m_accelY = m_accelZ = 0;
    m_gyroX = m_gyroY = m_gyroZ = 0;


    m_controller = new QLowEnergyController(m_address, this);
    connect(m_controller, &QLowEnergyController::connected, this, &VecsDevice::deviceConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &VecsDevice::deviceDisconnected);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &VecsDevice::serviceDiscoveryDone);

    m_timer = new QTimer(this);
    m_timer->setInterval(5000);
    connect(m_timer, &QTimer::timeout, this, &VecsDevice::timerJob);
}

VecsDevice::~VecsDevice()
{
    disconnectFromDevice();
}

void VecsDevice::connectToDevice()
{   
    m_controller->connectToDevice();
    m_connectionState = StateConnecting;
    emit stateChanged();    
}

void VecsDevice::disconnectFromDevice()
{
    m_normalDisconnect = true;
    m_timer->stop();
    m_controller->disconnectFromDevice();    

    // Все объекты удаляются при disconnectFromDevice() автоматически
    m_batteryService = nullptr;
    m_keyService = nullptr;
    m_mpuService = nullptr;
}

void VecsDevice::deviceConnected()
{
    qDebug() << "device [" << m_address.toString() << "] connected";    

    // Сброс данных для переподключения при обрыве связи
    m_normalDisconnect = false;
    m_reconnections = 0;

    // Очищаем счетчики нажатий
    m_singleClickCount = m_doubleClickCount = m_longClickCount = 0;

    // Запускаем обнаружение сервисов
    m_controller->discoverServices();
}

void VecsDevice::deviceDisconnected()
{    
    m_connectionState = StateDisconnected;
    emit stateChanged();

    if (!m_normalDisconnect) {
        if (m_reconnections < m_maxReconnections) {
            m_reconnections++;
            qDebug() << "recon: " << m_reconnections << " of max: " << m_maxReconnections;
            qDebug() << "Connection to [" << m_address.toString() << "] lost.. trying to reconnect after 0.5sec";
            QTimer::singleShot(500, this, SLOT(connectToDevice()));
        }
    } else {
        qDebug() << "device [" << m_address.toString() << "] disconnected";
    }
}


void VecsDevice::keyRequest(quint8 delay)
{
    if (m_connectionState != StateConnected)
        return;

    writeCharacteristic(m_keyService, QBluetoothUuid((quint16)VecsDevice::CharKeyRequest), delay);
}

void VecsDevice::mpuStart()
{
    if (m_connectionState != StateConnected)
        return;

    // Останавлиаваем MPU (если перезапуск с новыми параметрами)
    mpuStop();

    // Пробуждение MPU, настройка частоты передачи данных и запуск
    writeCharacteristic(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharMpuControl), (quint8)m_mpuRate);

    // Задаем диапазоны измерений гироскопа и акселерометра
    writeCharacteristic(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharAccelRange), (quint8)m_accelRange);
    writeCharacteristic(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharGyroRange), (quint8)m_gyroRange);

    // Включение приема данных от MPU
    enableNotifications(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharMpuData), true);

    m_mpuState = true;
    emit mpuStateChanged();
}

void VecsDevice::mpuStop()
{
    if (m_connectionState != StateConnected)
        return;

    // Отключаем отправку данных
    enableNotifications(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharMpuData), false);
    // Останавливаем MPU
    writeCharacteristic(m_mpuService, QBluetoothUuid((quint16)VecsDevice::CharMpuControl), 0);

    m_mpuState = false;
    emit mpuStateChanged();
}

void VecsDevice::controllerError(QLowEnergyController::Error error)
{
    qDebug() << "device controller error: " << error;
}

void VecsDevice::timerJob()
{
    if (m_batteryService == nullptr)
        return;
    if (m_batteryService->state() != QLowEnergyService::ServiceDiscovered)
        return;

    QLowEnergyCharacteristic batteryLevel = m_batteryService->characteristic(QBluetoothUuid::BatteryLevel);
    m_batteryService->readCharacteristic(batteryLevel);
}

QLowEnergyService *VecsDevice::addService(const QBluetoothUuid &uuid)
{
    QLowEnergyService *service = m_controller->createServiceObject(uuid, this);
    if (service == nullptr) {
        qDebug() << "error: " << QString("0x%1").arg(uuid.toUInt16(), 4, 16, QLatin1Char('0')) << "service not found on the device";
        return nullptr;
    }

    connect(service, &QLowEnergyService::stateChanged, this, &VecsDevice::serviceStateChanged);
    connect(service, &QLowEnergyService::characteristicRead, this, &VecsDevice::characteristicRead);
    connect(service, &QLowEnergyService::characteristicChanged, this, &VecsDevice::characteristicNotification);
    service->discoverDetails();

    return service;
}

bool VecsDevice::enableNotifications(QLowEnergyService *service, const QBluetoothUuid &uuid, bool enable)
{
    if (service == nullptr)
        return false;
    if (service->state() != QLowEnergyService::ServiceDiscovered)
        return false;

    // Получаем характеристику
    const QLowEnergyCharacteristic c = service->characteristic(uuid);
    if (!c.isValid()) {
        qDebug() << "error: characteristic (uuid: " << QString("0x%1").arg(uuid.toUInt16(), 4, 16, QLatin1Char('0')) << ") not found";
        return false;
    }

    // Получаем дескриптор настройки
    const QLowEnergyDescriptor desc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!desc.isValid()) {
        qDebug() << "error: characteristic (uuid: " << QString("0x%1").arg(uuid.toUInt16(), 4, 16, QLatin1Char('0')) << ") configuration descriptor not found";
        return false;
    }
    // Включаем уведомления по этой характеристике
    if (enable)
        service->writeDescriptor(desc, QByteArray::fromHex("0100"));
    else
        service->writeDescriptor(desc, QByteArray::fromHex("0000"));

    return true;
}

bool VecsDevice::writeCharacteristic(QLowEnergyService *service, const QBluetoothUuid &uuid, quint8 value)
{
    if (service == nullptr)
        return false;
    if (service->state() != QLowEnergyService::ServiceDiscovered)
        return false;

    const QLowEnergyCharacteristic c = service->characteristic(uuid);
    if (!c.isValid()) {
        qDebug() << "error: characteristic (uuid: " << QString("0x%1").arg(uuid.toUInt16(), 4, 16, QLatin1Char('0')) << ") not found";
        return false;
    }

    service->writeCharacteristic(c, QByteArray::fromRawData((const char *)&value, 1), QLowEnergyService::WriteWithoutResponse);
    return true;
}

void VecsDevice::parseMpuData(const QByteArray &data)
{
    QDataStream s(data);
    s >> m_accelX >> m_accelY >> m_accelZ >> m_packetIndex >> m_gyroX >> m_gyroY >> m_gyroZ;
}

int VecsDevice::maxReconnections() const
{
    return m_maxReconnections;
}

void VecsDevice::setMaxReconnections(int maxReconnections)
{
    m_maxReconnections = maxReconnections;
}

bool VecsDevice::mpuState() const
{
    return m_mpuState;
}

quint16 VecsDevice::packetIndex() const
{
    return m_packetIndex;
}

qint16 VecsDevice::gyroZ() const
{
    return m_gyroZ;
}

qint16 VecsDevice::gyroY() const
{
    return m_gyroY;
}

qint16 VecsDevice::gyroX() const
{
    return m_gyroX;
}

qint16 VecsDevice::accelZ() const
{
    return m_accelZ;
}

qint16 VecsDevice::accelY() const
{
    return m_accelY;
}

qint16 VecsDevice::accelX() const
{
    return m_accelX;
}

VecsDevice::DeviceRole VecsDevice::role() const
{
    return m_role;
}

void VecsDevice::setRole(VecsDevice::DeviceRole role)
{
    if (role != m_role) {
        m_role = role;
        emit roleChanged();
    }
}

void VecsDevice::serviceDiscoveryDone()
{
    m_batteryService = addService(QBluetoothUuid::BatteryService);
    m_keyService = addService(QBluetoothUuid((quint16)VecsDevice::KeyService));
    m_mpuService = addService(QBluetoothUuid((quint16)VecsDevice::MpuService));

    // Сообщаем об изменении состояния после окончания обнаружения сервисов
    m_connectionState = StateConnected;
    emit stateChanged();
}

void VecsDevice::serviceStateChanged(QLowEnergyService::ServiceState state)
{
    // Игнорируем все состояния кроме ServiceDiscovered
    if (state != QLowEnergyService::ServiceDiscovered)
        return;

    // Распознаем отправителя сигнала
    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    if (!service)
        return;

//    qDebug() << "info: " << service->serviceName() << QString("0x%1").arg(service->serviceUuid().toUInt16(), 4, 16, QLatin1Char('0')) << "fully discovered";

    if (service == m_batteryService) {
        const QLowEnergyCharacteristic batteryLevel = service->characteristic(QBluetoothUuid::BatteryLevel);
        if (batteryLevel.isValid()) {
            // Обновляем значение заряда
            m_batteryLevel = batteryLevel.value().at(0);
            emit batteryLevelChanged(m_batteryLevel);

            // Запускаем таймер на периодический опрос состояния батарейки            
            m_timer->start();
        } else {
            qDebug() << "error: BatteryLevel characteristic not found";
        }
    } else if (service == m_keyService) {
        // Включаем уведомления по характеристике KeyPressState
        enableNotifications(service, QBluetoothUuid((quint16)VecsDevice::CharKeyPressState), true);
    } else if (service == m_mpuService) {
        // Останавливаем работу MPU если был запущен до этого
        mpuStop();

/* FIXME: Убрано, т.к. если пользователь настроил параметры до подключения,
 * то при подключении они внезапно меняются на те, которые остались на приборе
 * в прошлой сессии
 *
        // Характеристика "Диапазон аксеелерометра"
        const QLowEnergyCharacteristic accelRange = service->characteristic(QBluetoothUuid((quint16)VecsDevice::CharAccelRange));
        if (accelRange.isValid()) {
            m_accelRange = (VecsDevice::AccelRange)accelRange.value().at(0);
            emit accelRangeChanged();
        } else {
            qDebug() << "error: AccelRange characteristic not found";
        }

        // Характеристика "Диапазон гироскопа"
        const QLowEnergyCharacteristic gyroRange = service->characteristic(QBluetoothUuid((quint16)VecsDevice::CharGyroRange));
        if (gyroRange.isValid()) {
            m_gyroRange = (VecsDevice::GyroRange)gyroRange.value().at(0);
            emit gyroRangeChanged();
        } else {
            qDebug() << "error: GyroRange characteristic not found";
        }
*/

        // Автоматически запускаем прием данных для ролей пациента
        if (m_role == RolePatientBack || m_role == RolePatientHand) {
            mpuStart();
        }
    }
}

void VecsDevice::characteristicNotification(const QLowEnergyCharacteristic &c, const QByteArray &v)
{
//    qDebug() <<  QString("[%1] notify %2 (uuid: 0x%3) value: [%4]")
//                 .arg(this->address())
//                 .arg(c.name())
//                 .arg(c.uuid().toUInt16(), 4, 16, QLatin1Char('0'))
//                 .arg(QString(v.toHex()));

    switch (c.uuid().toUInt16()) {
    case VecsDevice::CharKeyPressState:
    {
        VecsDevice::ButtonClick type = (VecsDevice::ButtonClick)v.at(0);

        switch (type) {
        case SINGLE_CLICK:
            m_singleClickCount++;
            break;
        case DOUBLE_CLICK:
            m_doubleClickCount++;
            break;
        case LONG_CLICK:
            m_longClickCount++;
            break;
        }

        emit keyPressed(type);
        break;
    }
    case VecsDevice::CharMpuData:
    {        
        parseMpuData(v);
        emit mpuDataRecieved(v);
        break;
    }
    }
}

void VecsDevice::characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &v)
{    
//    qDebug() <<  QString("[%1] read %2 (uuid: 0x%3) value: %4")
//                 .arg(this->address())
//                 .arg(c.name())
//                 .arg(c.uuid().toUInt16(), 4, 16, QLatin1Char('0'))
//                 .arg(QString(v.toHex()));

    switch (c.uuid().toUInt16()) {
        case QBluetoothUuid::BatteryLevel:
            if (v.at(0) != m_batteryLevel) {
                m_batteryLevel = v.at(0);
                emit batteryLevelChanged(m_batteryLevel);
            }
            break;
    }
}

VecsDevice::AccelRange VecsDevice::accelRange() const
{
    return m_accelRange;
}

void VecsDevice::setAccelRange(VecsDevice::AccelRange accelRange)
{
    if (m_accelRange != accelRange) {
        m_accelRange = accelRange;
        emit accelRangeChanged();
    }
}

VecsDevice::GyroRange VecsDevice::gyroRange() const
{
    return m_gyroRange;
}

void VecsDevice::setGyroRange(VecsDevice::GyroRange gyroRange)
{
    if (m_gyroRange != gyroRange) {
        m_gyroRange = gyroRange;
        emit gyroRangeChanged();
    }
}

int VecsDevice::mpuRate() const
{
    return m_mpuRate;
}

void VecsDevice::setMpuRate(int mpuRate)
{
    if (mpuRate < 1)
        mpuRate = 1;
    else if (mpuRate > 200)
        mpuRate = 200;

    if (mpuRate != m_mpuRate) {
        m_mpuRate = mpuRate;
        emit mpuRateChanged();
    }
}

VecsDevice::ConnectionState VecsDevice::connectionState() const
{
    return m_connectionState;
}

int VecsDevice::batteryLevel() const
{
    return m_batteryLevel;
}

qint16 VecsDevice::rssi() const
{
    return m_rssi;
}

QString VecsDevice::address() const
{
    return m_address.toString();
}

quint32 VecsDevice::longClickCount() const
{
    return m_longClickCount;
}

quint32 VecsDevice::doubleClickCount() const
{
    return m_doubleClickCount;
}

quint32 VecsDevice::singleClickCount() const
{
    return m_singleClickCount;
}

int VecsDevice::interval() const
{
    return m_timer->interval();
}

void VecsDevice::setInterval(int interval)
{
    m_timer->setInterval((interval < 1000) ? 1000 : interval);
}
