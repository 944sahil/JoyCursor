#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QWidget>
#include <QPainter>
#include <QThread>
#include "../workers/CoreWorker.h"
#include "ControllerLibraryWindow.h"
#include "ControllerCustomizationWindow.h"

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
    QString m_currentControllerGuid;
    QString m_currentControllerName;
    bool m_currentControllerConnected = false;

private slots:
    void onControllerConnected(const QString& guid, const QString& name);
    void onControllerDisconnected(const QString& guid);
    void onManageControllersClicked();
    void onControllerLibraryClosed();
    void onConfigureControllerClicked();

private:
    QThread* workerThread;
    CoreWorker* coreWorker;
    ControllerLibraryWindow* controllerLibraryWindow;
}; 