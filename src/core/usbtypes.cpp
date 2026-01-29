#include "usbtypes.h"

QVariantList toVariant(const UsbEvent &event) {
    return {
        event.timestamp,
        event.level,
        event.subsystem,
        event.source,
        event.message,
        event.isUsb,
        event.isError,
        event.deviceId
    };
}

UsbEvent fromVariant(const QVariantList &data) {
    UsbEvent event;
    if (data.size() < 8) {
        return event;
    }
    event.timestamp = data.at(0).toString();
    event.level = data.at(1).toString();
    event.subsystem = data.at(2).toString();
    event.source = data.at(3).toString();
    event.message = data.at(4).toString();
    event.isUsb = data.at(5).toBool();
    event.isError = data.at(6).toBool();
    event.deviceId = data.at(7).toString();
    return event;
}

QVariantList toVariant(const UsbDeviceInfo &device) {
    return {
        device.busId,
        device.deviceId,
        device.vendorId,
        device.productId,
        device.summary,
        device.sysPath
    };
}

UsbDeviceInfo deviceFromVariant(const QVariantList &data) {
    UsbDeviceInfo device;
    if (data.size() < 6) {
        return device;
    }
    device.busId = data.at(0).toString();
    device.deviceId = data.at(1).toString();
    device.vendorId = data.at(2).toString();
    device.productId = data.at(3).toString();
    device.summary = data.at(4).toString();
    device.sysPath = data.at(5).toString();
    return device;
}
