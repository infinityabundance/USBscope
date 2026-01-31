#include "icon_helpers.h"

#include <QCoreApplication>
#include <QFileInfo>

namespace {
QString iconFilePath() {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString assetSvgPath = appDir + "/../assets/usbscope.svg";
    if (QFileInfo::exists(assetSvgPath)) {
        return assetSvgPath;
    }

    const QString assetPngPath = appDir + "/../assets/usbscope.png";
    if (QFileInfo::exists(assetPngPath)) {
        return assetPngPath;
    }

    const QString localSvgPath = appDir + "/usbscope.svg";
    if (QFileInfo::exists(localSvgPath)) {
        return localSvgPath;
    }

    const QString localPngPath = appDir + "/usbscope.png";
    if (QFileInfo::exists(localPngPath)) {
        return localPngPath;
    }

    return {};
}
}

QIcon loadUsbScopeIcon() {
    const QString path = iconFilePath();
    if (!path.isEmpty()) {
        return QIcon(path);
    }

    QIcon themeIcon = QIcon::fromTheme("usbscope");
    if (!themeIcon.isNull()) {
        return themeIcon;
    }
    return QIcon::fromTheme("usb");
}
