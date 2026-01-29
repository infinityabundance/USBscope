#pragma once

#include <QObject>
#include <QProcess>

#include "usbtypes.h"

class JournalTail : public QObject {
    Q_OBJECT
public:
    explicit JournalTail(QObject *parent = nullptr);

    void start();

signals:
    void eventParsed(const UsbEvent &event);

private slots:
    void handleReadyRead();

private:
    UsbEvent parseLine(const QString &line) const;

    QProcess m_process;
};
