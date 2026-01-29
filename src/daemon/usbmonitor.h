#pragma once

#include <QObject>
#include <QSocketNotifier>

#include <libudev.h>

#include "usbtypes.h"

class UsbMonitor : public QObject {
    Q_OBJECT
public:
    explicit UsbMonitor(QObject *parent = nullptr);
    ~UsbMonitor() override;

    void start();

signals:
    void devicesChanged(const QList<UsbDeviceInfo> &devices);

private slots:
    void handleUdevEvent();

private:
    QList<UsbDeviceInfo> buildDeviceList();

    struct udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;
    QSocketNotifier *m_notifier = nullptr;
};
