#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QIcon>

#include "dbus_helpers.h"
#include "mainwindow.h"

namespace {
QIcon appIcon() {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString iconPath = appDir + "/../data/icons/usbscope.svg";
    if (QFileInfo::exists(iconPath)) {
        return QIcon(iconPath);
    }
    return QIcon::fromTheme("usb");
}

void ensureDaemonAndTrayRunning() {
    startUsbScopeDaemon();
    startUsbScopeTray();
}
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    registerUsbDbusTypes();
    ensureDaemonAndTrayRunning();

    MainWindow window;
    window.setWindowIcon(appIcon());
    window.show();

    return app.exec();
}
