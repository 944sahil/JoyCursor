#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QWidget>
#include <QPainter>
#include <QThread>
#include "../workers/ControllerWorker.h"

// Custom widget for a perfect green dot
class DotWidget : public QWidget {
public:
    explicit DotWidget(QWidget* parent = nullptr) : QWidget(parent), m_color("#FF3B30") {}
    void setColor(const QColor& color) { m_color = color; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(m_color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(rect());
    }
private:
    QColor m_color;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QLabel* iconLabel;
    QLabel* controllerNameLabel;
    QLabel* statusLabel;
    QFrame* profileBox;
    QWidget* profileStatusDot; // Now a DotWidget
    QLabel* profileNameLabel;
    QPushButton* configureButton;
    QPushButton* manageButton;

private slots:
    void onControllerConnected(const QString& name);
    void onControllerDisconnected();

private:
    QThread* workerThread;
    ControllerWorker* controllerWorker;
}; 