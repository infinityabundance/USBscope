#include "trayicon.h"

#include <QApplication>
#include <QClipboard>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
#include <QIcon>
#include <QProcess>
#include <QUrl>

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent) {
    setIcon(QIcon::fromTheme("usb"));
    setupMenu();

    connect(&m_client, &UsbscopeDBusClient::LogEvent, this, &TrayIcon::handleLogEvent);
    connect(&m_client, &UsbscopeDBusClient::ErrorBurst, this, &TrayIcon::handleErrorBurst);

    connect(&m_tooltipTimer, &QTimer::timeout, this, &TrayIcon::updateTooltip);
    m_tooltipTimer.start(5000);

    connect(&m_statusTimer, &QTimer::timeout, this, &TrayIcon::updateDaemonStatus);
    m_statusTimer.start(5000);

    updateTooltip();
    updateDaemonStatus();
}

void TrayIcon::setupMenu() {
    m_statusAction = m_menu.addAction("Daemon: unknown");
    m_statusAction->setEnabled(false);
    m_menu.addSeparator();

    m_openAction = m_menu.addAction("Open USBscope");
    m_aboutAction = m_menu.addAction("About USBscope");
    m_startDaemonAction = m_menu.addAction("Start Daemon");
    m_stopDaemonAction = m_menu.addAction("Stop Daemon");
    m_copyAction = m_menu.addAction("Copy last error");
    m_menu.addSeparator();
    m_quitAction = m_menu.addAction("Quit");

    connect(m_openAction, &QAction::triggered, this, &TrayIcon::openUi);
    connect(m_aboutAction, &QAction::triggered, this, &TrayIcon::openRepo);
    connect(m_startDaemonAction, &QAction::triggered, this, &TrayIcon::startDaemon);
    connect(m_stopDaemonAction, &QAction::triggered, this, &TrayIcon::stopDaemon);
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
    QString status = daemonStatusText();
    if (!m_lastErrorTime.isValid()) {
        return status + "\nNo recent USB errors";
    }

    qint64 seconds = m_lastErrorTime.secsTo(QDateTime::currentDateTime());
    return status + QStringLiteral("\nLast USB error: %1 seconds ago").arg(seconds);
}

void TrayIcon::openUi() {
    startUsbScopeUi();
}

void TrayIcon::openRepo() {
    const QUrl url("https://github.com/infinityabundance/USBscope");
    if (!QDesktopServices::openUrl(url)) {
        QProcess::startDetached("xdg-open", {url.toString()});
    }
}

void TrayIcon::copyLastError() {
    if (m_lastErrorMessage.isEmpty()) {
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_lastErrorMessage);
}

void TrayIcon::updateDaemonStatus() {
    if (!m_statusAction) {
        return;
    }
    const bool running = isUsbScopeRunning();
    m_statusAction->setText(daemonStatusText());
    if (m_startDaemonAction) {
        m_startDaemonAction->setEnabled(!running);
    }
    if (m_stopDaemonAction) {
        m_stopDaemonAction->setEnabled(running);
    }
    updateTooltip();
}

QString TrayIcon::daemonStatusText() const {
    const bool running = isUsbScopeRunning();
    return running ? QStringLiteral("Daemon: running") : QStringLiteral("Daemon: stopped");
}

void TrayIcon::startDaemon() {
    startUsbScopeDaemon();
    updateDaemonStatus();
}

void TrayIcon::stopDaemon() {
    stopUsbScopeDaemon();
    updateDaemonStatus();
}
