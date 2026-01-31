#include "icon_helpers.h"

#include <QCoreApplication>
#include <QFileInfo>

namespace {
QString iconFilePath() {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString assetPath = appDir + "/../assets/usbscope.png";
    if (QFileInfo::exists(assetPath)) {
        return assetPath;
    }

    const QString localPath = appDir + "/usbscope.png";
    if (QFileInfo::exists(localPath)) {
        return localPath;
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
