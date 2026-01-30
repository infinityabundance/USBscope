#pragma once

#include <QGraphicsScene>
#include <QDateTime>

#include "usbtypes.h"

class EventMarker;

class TimelineScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit TimelineScene(QObject *parent = nullptr);

    void setEvents(const QList<UsbEvent> &events);
    void addEvent(const UsbEvent &event);
    void updateTimeRange(const QDateTime &start, const QDateTime &end);

signals:
    void eventClicked(const UsbEvent &event);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void rebuildScene();
    qreal timestampToX(const QString &timestamp) const;
    qreal eventTypeToY(const UsbEvent &event) const;

    QList<UsbEvent> m_events;
    QDateTime m_startTime;
    QDateTime m_endTime;
    qreal m_sceneWidth = 10000.0;
    qreal m_sceneHeight = 300.0;
    qreal m_laneHeight = 60.0;
};
