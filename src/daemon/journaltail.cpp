#include "journaltail.h"

#include <QRegularExpression>

JournalTail::JournalTail(QObject *parent)
    : QObject(parent) {
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &JournalTail::handleReadyRead);
}

void JournalTail::start() {
    // Seed with recent kernel logs before following new entries.
    m_process.start("journalctl", {"-k", "-n", "200", "-f", "-o", "short"});
}

void JournalTail::handleReadyRead() {
    while (m_process.canReadLine()) {
        QString line = QString::fromUtf8(m_process.readLine()).trimmed();
        if (!line.isEmpty()) {
            emit eventParsed(parseLine(line));
        }
    }
}

UsbEvent JournalTail::parseLine(const QString &line) const {
    UsbEvent event;
    event.timestamp = line.section(' ', 0, 2).trimmed();
    event.source = line.section(' ', 3, 3).trimmed();
    event.message = line.section(' ', 4).trimmed();
    event.subsystem = QStringLiteral("kernel");

    const QString lowered = line.toLower();
    event.isUsb = lowered.contains("usb") || lowered.contains("xhci") || lowered.contains("usbhid") || lowered.contains("hub");
    event.isError = lowered.contains("error") || lowered.contains("fail") || lowered.contains("timeout");
    event.level = event.isError ? QStringLiteral("error") : QStringLiteral("info");

    return event;
}
