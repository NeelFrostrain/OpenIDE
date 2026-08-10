#pragma once

#include <QWidget>
#include <QPoint>

class QLabel;
class QPushButton;

// Custom title bar used because the window is frameless (Qt::FramelessWindowHint).
// Draws its own min/max/close buttons and implements drag-to-move.
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);

    void setTitle(const QString& title);

signals:
    void minimizeRequested();
    void maximizeRestoreRequested();
    void closeRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QLabel* m_titleLabel = nullptr;
    QPushButton* m_minButton = nullptr;
    QPushButton* m_maxButton = nullptr;
    QPushButton* m_closeButton = nullptr;
    QPoint m_dragOffset;
    bool m_dragging = false;
};
