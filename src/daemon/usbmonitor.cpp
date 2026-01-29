#include "usbmonitor.h"

#include <QDebug>

namespace {
QString safeStr(const char *value) {
    return value ? QString::fromUtf8(value) : QString();
}
}

UsbMonitor::UsbMonitor(QObject *parent)
    : QObject(parent) {
}

UsbMonitor::~UsbMonitor() {
    delete m_notifier;
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
}

void UsbMonitor::start() {
    m_udev = udev_new();
    if (!m_udev) {
        return;
    }

    emit devicesChanged(buildDeviceList());

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", nullptr);
    udev_monitor_enable_receiving(m_monitor);

    int fd = udev_monitor_get_fd(m_monitor);
    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &UsbMonitor::handleUdevEvent);
}

void UsbMonitor::handleUdevEvent() {
    if (!m_monitor) {
        return;
    }

    udev_device *dev = udev_monitor_receive_device(m_monitor);
    if (dev) {
        udev_device_unref(dev);
    }

    emit devicesChanged(buildDeviceList());
}

QList<UsbDeviceInfo> UsbMonitor::buildDeviceList() {
    QList<UsbDeviceInfo> devices;
    if (!m_udev) {
        return devices;
    }

    udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    if (!enumerate) {
        return devices;
    }

    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);
    udev_list_entry *entry = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry *current = nullptr;
    udev_list_entry_foreach(current, entry) {
        const char *path = udev_list_entry_get_name(current);
        udev_device *dev = udev_device_new_from_syspath(m_udev, path);
        if (!dev) {
            continue;
        }

        const char *devtype = udev_device_get_devtype(dev);
        if (!devtype || QString::fromUtf8(devtype) != QLatin1String("usb_device")) {
            udev_device_unref(dev);
            continue;
        }

        UsbDeviceInfo info;
        info.busId = safeStr(udev_device_get_sysname(dev));
        info.deviceId = safeStr(udev_device_get_property_value(dev, "ID_SERIAL_SHORT"));
        info.vendorId = safeStr(udev_device_get_sysattr_value(dev, "idVendor"));
        info.productId = safeStr(udev_device_get_sysattr_value(dev, "idProduct"));
        info.summary = safeStr(udev_device_get_property_value(dev, "ID_MODEL_FROM_DATABASE"));
        if (info.summary.isEmpty()) {
            info.summary = safeStr(udev_device_get_property_value(dev, "ID_MODEL"));
        }
        info.sysPath = safeStr(path);

        devices.append(info);
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    return devices;
}
