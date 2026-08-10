#pragma once

#include <QWidget>
#include <QPlainTextEdit>

class CodeMinimap : public QWidget {
    Q_OBJECT
public:
    explicit CodeMinimap(QPlainTextEdit* editor, QWidget* parent = nullptr);

    void updateMinimap();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPlainTextEdit* m_editor;
};
