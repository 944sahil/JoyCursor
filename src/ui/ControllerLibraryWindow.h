#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QThread>
#include <QTimer>
#include <QMap>
#include <QString>
#include "../workers/CoreWorker.h"
#include <QEnterEvent>
#include <QSet>
#include "ControllerCustomizationWindow.h"

// Card widget for a single controller
class ControllerCard : public QFrame {
    Q_OBJECT
public:
    explicit ControllerCard(const QString& guid, const QString& name, bool connected, QWidget* parent = nullptr);
    void setConnected(bool connected);
    void setName(const QString& name);
    QString getGuid() const { return m_guid; }
    QString getName() const { return m_name; }
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
signals:
    void clicked(const QString& guid);
private:
    QString m_guid;
    QString m_name;
    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_statusLabel;
    void updateStatus(bool connected);
    void setupUI();
    void setHover(bool hover);
    bool m_hovered = false;
};

class ControllerLibraryWindow : public QWidget {
    Q_OBJECT
public:
    explicit ControllerLibraryWindow(CoreWorker* sharedCoreWorker, QWidget* parent = nullptr);
    ~ControllerLibraryWindow();

    void refreshControllerList();
    void showEvent(QShowEvent* event) override;

signals:
    void windowClosed();
    void controllerSelected(const QString& guid);

private slots:
    void onControllerConnected(const QString& guid, const QString& name);
    void onControllerDisconnected(const QString& guid);
    void onControllerCardClicked(const QString& guid);
    void openCustomizationWindow(const QString& guid);

private:
    void setupUI();
    void loadKnownControllers();
    void updateControllerStatus(const QString& guid, bool connected);
    void saveControllerChanges();
    void clearControllerCards();

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QWidget* m_cardListWidget;
    QVBoxLayout* m_cardListLayout;

    CoreWorker* m_coreWorker;

    // Track known controllers: GUID -> name
    QMap<QString, QString> m_knownControllers;
    // Track currently connected controllers: GUID -> name
    QMap<QString, QString> m_connectedControllers;
    QSet<QString> m_connectedGuids; // Track currently connected controller GUIDs
    // Map GUID to ControllerCard
    QMap<QString, ControllerCard*> m_cardWidgets;
}; 