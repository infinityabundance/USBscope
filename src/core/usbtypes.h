#pragma once

#include <QString>
#include <QVariantList>

struct UsbEvent {
    QString timestamp;
    QString level;
    QString subsystem;
    QString source;
    QString message;
    bool isUsb = false;
    bool isError = false;
    QString deviceId;
};

struct UsbDeviceInfo {
    QString busId;
    QString deviceId;
    QString vendorId;
    QString productId;
    QString summary;
    QString sysPath;
};

QVariantList toVariant(const UsbEvent &event);
UsbEvent fromVariant(const QVariantList &data);

QVariantList toVariant(const UsbDeviceInfo &device);
UsbDeviceInfo deviceFromVariant(const QVariantList &data);
