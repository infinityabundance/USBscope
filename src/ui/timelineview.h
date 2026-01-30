#pragma once

#include <QGraphicsView>
#include <QWheelEvent>

class QComboBox;
class QPushButton;

class TimelineView : public QGraphicsView {
    Q_OBJECT
public:
    explicit TimelineView(QWidget *parent = nullptr);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToView();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupControls();

    qreal m_currentZoom = 1.0;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;

    // Control widgets (stored as members for access)
    QComboBox *m_timeRangeCombo = nullptr;
    QPushButton *m_zoomInBtn = nullptr;
    QPushButton *m_zoomOutBtn = nullptr;
    QPushButton *m_resetZoomBtn = nullptr;
};
