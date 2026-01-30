#include "eventmarker.h"

#include <QBrush>
#include <QPen>
#include <QToolTip>
#include <QCursor>

EventMarker::EventMarker(const UsbEvent &event, qreal x, qreal y, qreal size)
    : QGraphicsEllipseItem(x - size/2, y - size/2, size, size)
    , m_event(event)
    , m_baseSize(size) {

    // Color-code by severity (Breeze theme colors)
    if (event.isError) {
        m_baseColor = QColor("#da4453"); // Breeze Red
    } else if (event.level.contains("warn", Qt::CaseInsensitive)) {
        m_baseColor = QColor("#f67400"); // Breeze Orange
    } else if (event.isUsb) {
        m_baseColor = QColor("#27ae60"); // Breeze Green
    } else {
        m_baseColor = QColor("#3daee9"); // Breeze Blue
    }

    setBrush(QBrush(m_baseColor));
    setPen(QPen(m_baseColor.darker(120), 1.5));

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setCursor(Qt::PointingHandCursor);
}

void EventMarker::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    // Enlarge on hover
    qreal enlargedSize = m_baseSize * 1.5;
    qreal centerX = rect().center().x();
    qreal centerY = rect().center().y();
    setRect(centerX - enlargedSize/2, centerY - enlargedSize/2, enlargedSize, enlargedSize);

    // Brighten color
    setBrush(QBrush(m_baseColor.lighter(120)));
    setPen(QPen(m_baseColor, 2));

    // Show tooltip with event details
    QString tooltipText = QString(
        "<b>%1</b><br>"
        "<b>Level:</b> %2<br>"
        "<b>Time:</b> %3<br>"
        "<b>Message:</b> %4"
    ).arg(m_event.subsystem)
     .arg(m_event.level)
     .arg(m_event.timestamp)
     .arg(m_event.message.length() > 100 ? m_event.message.left(100) + "..." : m_event.message);

    QToolTip::showText(event->screenPos(), tooltipText);

    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void EventMarker::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    // Restore original size
    qreal centerX = rect().center().x();
    qreal centerY = rect().center().y();
    setRect(centerX - m_baseSize/2, centerY - m_baseSize/2, m_baseSize, m_baseSize);

    // Restore original color
    setBrush(QBrush(m_baseColor));
    setPen(QPen(m_baseColor.darker(120), 1.5));

    QToolTip::hideText();

    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

void EventMarker::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // Event will be emitted by the scene
    QGraphicsEllipseItem::mousePressEvent(event);
}
