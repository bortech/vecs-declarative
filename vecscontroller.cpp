#include "vecscontroller.h"
#include <QObjectList>

VecsController::VecsController(QObject *parent) :
    QObject(parent),
    m_discovering(false),
    m_agent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_agent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(addDevice(QBluetoothDeviceInfo)));
    connect(m_agent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(m_agent, SIGNAL(finished()),
            this, SLOT(scanFinished()));    
}

VecsController::~VecsController()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

void VecsController::startScan()
{
    qDeleteAll(m_devices);
    m_devices.clear();
    emit devicesUpdated();

    m_agent->start();

    m_discovering = true;
    emit stateChanged();

    setMessage("Scanning for devices...");
}

void VecsController::setMessage(const QString &message)
{
    m_message = message;
    emit messageChanged();
}

void VecsController::startSession()
{
    for (const auto& dev : m_devices) {
        switch (dev->role()) {
        case VecsDevice::RoleUndefined:
            dev->disconnectFromDevice();
            break;
        case VecsDevice::RoleDoctor:
            if (dev->connectionState() == VecsDevice::StateDisconnected)
                dev->connectToDevice();
            break;
        case VecsDevice::RolePatientHand:
        case VecsDevice::RolePatientBack:
            if (dev->connectionState() == VecsDevice::StateConnected) {
                dev->mpuStart();
            } else if (dev->connectionState() == VecsDevice::StateDisconnected) {
                dev->connectToDevice(); // MPU запуститься автоматически, т.к. выставлена роль
            }
            break;
        }
    }
}

QString VecsController::message() const
{
    return m_message;
}

QVariant VecsController::model() const
{
    QObjectList objects;

    for (const auto& i : m_devices)
        objects.append(i);

    return QVariant::fromValue(objects);
}

QList<VecsDevice *> VecsController::devices() const
{
    return m_devices;
}

bool VecsController::discovering() const
{
    return m_discovering;
}

void VecsController::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration &&
        device.name().contains("VE Control Sensor")) {

        VecsDevice *vecs = new VecsDevice(device.address(), device.rssi(), this);

        m_devices.append(vecs);
        emit devicesUpdated();

        setMessage(QString("Device found [%1]").arg(device.address().toString()));
    }
}

void VecsController::scanFinished()
{    
    emit devicesUpdated();

    m_discovering = false;
    emit stateChanged();

    if (m_devices.isEmpty()) {
        setMessage("Scan finished: no devices found");
    } else {
        setMessage(QString("Scan finished: found %1 devices").arg(m_devices.size()));
    }    
}

void VecsController::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setMessage("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setMessage("Writing or reading from the device resulted in an error.");
    else
        setMessage("An unknown error has occurred.");
}
