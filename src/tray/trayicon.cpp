#include "trayicon.h"

#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QProcess>

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent) {
    setIcon(QIcon::fromTheme("usb"));
    setupMenu();

    connect(&m_client, &UsbscopeDBusClient::LogEvent, this, &TrayIcon::handleLogEvent);
    connect(&m_client, &UsbscopeDBusClient::ErrorBurst, this, &TrayIcon::handleErrorBurst);

    connect(&m_tooltipTimer, &QTimer::timeout, this, &TrayIcon::updateTooltip);
    m_tooltipTimer.start(5000);

    updateTooltip();
}

void TrayIcon::setupMenu() {
    m_openAction = m_menu.addAction("Open USBscope");
    m_copyAction = m_menu.addAction("Copy last error");
    m_menu.addSeparator();
    m_quitAction = m_menu.addAction("Quit");

    connect(m_openAction, &QAction::triggered, this, &TrayIcon::openUi);
    connect(m_copyAction, &QAction::triggered, this, &TrayIcon::copyLastError);
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    setContextMenu(&m_menu);
}

void TrayIcon::handleLogEvent(const UsbEvent &event) {
    if (event.isError && event.isUsb) {
        m_lastErrorMessage = event.message;
        m_lastErrorTime = QDateTime::currentDateTime();
        updateTooltip();
    }
}

void TrayIcon::handleErrorBurst(int count, const QString &lastMessage) {
    m_lastErrorMessage = lastMessage;
    m_lastErrorTime = QDateTime::currentDateTime();
    showMessage("USBscope", QStringLiteral("Error burst (%1): %2").arg(count).arg(lastMessage));
    updateTooltip();
}

void TrayIcon::updateTooltip() {
    setToolTip(tooltipText());
}

QString TrayIcon::tooltipText() const {
    if (!m_lastErrorTime.isValid()) {
        return QStringLiteral("No recent USB errors");
    }

    qint64 seconds = m_lastErrorTime.secsTo(QDateTime::currentDateTime());
    return QStringLiteral("Last USB error: %1 seconds ago").arg(seconds);
}

void TrayIcon::openUi() {
    QProcess::startDetached("usbscope-ui");
}

void TrayIcon::copyLastError() {
    if (m_lastErrorMessage.isEmpty()) {
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_lastErrorMessage);
}
