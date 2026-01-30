#pragma once

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneHoverEvent>

#include "usbtypes.h"

class EventMarker : public QGraphicsEllipseItem {
public:
    explicit EventMarker(const UsbEvent &event, qreal x, qreal y, qreal size = 8.0);

    const UsbEvent &event() const { return m_event; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    UsbEvent m_event;
    QColor m_baseColor;
    qreal m_baseSize;
};
