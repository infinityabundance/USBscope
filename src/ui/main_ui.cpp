#include <QApplication>
#include <QIcon>

#include "dbus_helpers.h"
#include "icon_helpers.h"
#include "mainwindow.h"

namespace {
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
    app.setWindowIcon(loadUsbScopeIcon());
    window.setWindowIcon(loadUsbScopeIcon());
    window.show();

    return app.exec();
}
