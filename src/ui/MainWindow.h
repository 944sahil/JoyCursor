#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QWidget>
#include <QPainter>

// Custom widget for a perfect green dot
class DotWidget : public QWidget {
public:
    explicit DotWidget(QWidget* parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#FF3B30")); // Red dot for disconnected
        p.setPen(Qt::NoPen);
        p.drawEllipse(rect());
    }
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
}; 