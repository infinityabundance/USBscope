#include "timelinescene.h"

#include "eventmarker.h"

#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

TimelineScene::TimelineScene(QObject *parent)
    : QGraphicsScene(parent) {
    setSceneRect(0, 0, m_sceneWidth, m_sceneHeight);
    setBackgroundBrush(QBrush(QColor("#f5f5f5")));
}

void TimelineScene::setEvents(const QList<UsbEvent> &events) {
    m_events = events;

    // Determine time range
    if (!events.isEmpty()) {
        QDateTime minTime = QDateTime::currentDateTime();
        QDateTime maxTime = QDateTime::fromSecsSinceEpoch(0);

        for (const UsbEvent &event : events) {
            QDateTime eventTime = QDateTime::fromString(event.timestamp, Qt::ISODate);
            if (!eventTime.isValid()) {
                eventTime = QDateTime::fromString(event.timestamp, "MMM dd hh:mm:ss");
            }
            if (eventTime.isValid()) {
                if (eventTime < minTime) minTime = eventTime;
                if (eventTime > maxTime) maxTime = eventTime;
            }
        }

        m_startTime = minTime;
        m_endTime = maxTime;
    }

    rebuildScene();
}

void TimelineScene::addEvent(const UsbEvent &event) {
    m_events.append(event);

    // Update time range if needed
    QDateTime eventTime = QDateTime::fromString(event.timestamp, Qt::ISODate);
    if (!eventTime.isValid()) {
        eventTime = QDateTime::fromString(event.timestamp, "MMM dd hh:mm:ss");
    }

    if (eventTime.isValid()) {
        if (!m_startTime.isValid() || eventTime < m_startTime) {
            m_startTime = eventTime;
            rebuildScene();
            return;
        }
        if (!m_endTime.isValid() || eventTime > m_endTime) {
            m_endTime = eventTime;
        }

        // Add single marker without rebuilding
        qreal x = timestampToX(event.timestamp);
        qreal y = eventTypeToY(event);
        EventMarker *marker = new EventMarker(event, x, y);
        addItem(marker);
    }
}

void TimelineScene::updateTimeRange(const QDateTime &start, const QDateTime &end) {
    m_startTime = start;
    m_endTime = end;
    rebuildScene();
}

void TimelineScene::rebuildScene() {
    clear();

    if (m_events.isEmpty() || !m_startTime.isValid() || !m_endTime.isValid()) {
        return;
    }

    // Draw lane dividers and labels
    QStringList laneLabels = {"USB Events", "Errors", "Other Events"};
    for (int i = 0; i < 3; ++i) {
        qreal y = i * m_laneHeight + m_laneHeight;

        // Draw lane divider
        QGraphicsLineItem *line = addLine(0, y, m_sceneWidth, y, QPen(QColor("#cccccc"), 1));
        line->setZValue(-1);

        // Draw lane label
        QGraphicsSimpleTextItem *label = addSimpleText(laneLabels[i]);
        label->setPos(10, i * m_laneHeight + 10);
        label->setBrush(QBrush(QColor("#666666")));
    }

    // Add event markers
    for (const UsbEvent &event : m_events) {
        qreal x = timestampToX(event.timestamp);
        qreal y = eventTypeToY(event);

        EventMarker *marker = new EventMarker(event, x, y);
        addItem(marker);
    }

    // Draw time axis
    QGraphicsLineItem *timeAxis = addLine(0, m_sceneHeight - 20, m_sceneWidth, m_sceneHeight - 20,
                                           QPen(QColor("#333333"), 2));
    timeAxis->setZValue(-1);
}

qreal TimelineScene::timestampToX(const QString &timestamp) const {
    if (!m_startTime.isValid() || !m_endTime.isValid()) {
        return 0;
    }

    QDateTime eventTime = QDateTime::fromString(timestamp, Qt::ISODate);
    if (!eventTime.isValid()) {
        eventTime = QDateTime::fromString(timestamp, "MMM dd hh:mm:ss");
    }

    if (!eventTime.isValid()) {
        return 0;
    }

    qint64 totalSeconds = m_startTime.secsTo(m_endTime);
    if (totalSeconds <= 0) {
        return m_sceneWidth / 2;
    }

    qint64 eventSeconds = m_startTime.secsTo(eventTime);
    qreal margin = 50.0;
    return margin + ((m_sceneWidth - 2 * margin) * eventSeconds) / totalSeconds;
}

qreal TimelineScene::eventTypeToY(const UsbEvent &event) const {
    // Lane 0: USB Events
    // Lane 1: Errors
    // Lane 2: Other Events

    if (event.isError) {
        return m_laneHeight + m_laneHeight / 2; // Lane 1
    } else if (event.isUsb) {
        return m_laneHeight / 2; // Lane 0
    } else {
        return 2 * m_laneHeight + m_laneHeight / 2; // Lane 2
    }
}

void TimelineScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (EventMarker *marker = dynamic_cast<EventMarker *>(item)) {
        emit eventClicked(marker->event());
    }

    QGraphicsScene::mousePressEvent(event);
}
