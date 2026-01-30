#include "timelineview.h"

#include <QComboBox>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

TimelineView::TimelineView(QWidget *parent)
    : QGraphicsView(parent) {
    setDragMode(QGraphicsView::NoDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void TimelineView::zoomIn() {
    qreal factor = 1.15;
    m_currentZoom *= factor;
    scale(factor, 1.0); // Only zoom horizontally
}

void TimelineView::zoomOut() {
    qreal factor = 1.0 / 1.15;
    m_currentZoom *= factor;
    scale(factor, 1.0); // Only zoom horizontally
}

void TimelineView::resetZoom() {
    resetTransform();
    m_currentZoom = 1.0;
}

void TimelineView::fitToView() {
    if (scene()) {
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        m_currentZoom = transform().m11();
    }
}

void TimelineView::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        // Normal scrolling
        QGraphicsView::wheelEvent(event);
    }
}

void TimelineView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)) {
        // Enable panning
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void TimelineView::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void TimelineView::mouseReleaseEvent(QMouseEvent *event) {
    if (m_isPanning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}
