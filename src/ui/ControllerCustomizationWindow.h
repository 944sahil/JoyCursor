#pragma once
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QMap>
#include <QSpinBox>
#include <QStackedWidget>
#include "../workers/CoreWorker.h"

class ControllerCustomizationWindow : public QWidget {
    Q_OBJECT
public:
    explicit ControllerCustomizationWindow(QWidget* parent = nullptr);
    ControllerCustomizationWindow(const QString& guid, const QString& name, bool connected, CoreWorker* coreWorker, QWidget* parent = nullptr);
    void setControllerInfo(const QString& guid, const QString& name, bool connected);
    static ControllerCustomizationWindow* openForController(const QString& guid, const QString& name, bool connected, CoreWorker* coreWorker, QWidget* parent = nullptr);
    static ControllerCustomizationWindow* getOpenWindow(const QString& guid);
    void setCoreWorker(CoreWorker* coreWorker);
    void loadMappingsFromCore();
    void saveMappingsToCore();
    void resetToDefault();

private:
    QString m_guid;
    QString m_name;
    bool m_connected;
    CoreWorker* m_coreWorker = nullptr;
    // Title and status
    QLabel* titleLabel;
    QLabel* statusLabel;
    QLabel* statusValueLabel;

    // Left Stick
    QGroupBox* leftStickGroup;
    QCheckBox* leftStickEnabled;
    QComboBox* leftStickActionType;
    QSlider* leftStickDeadzone; // (now unused, kept for compatibility)
    QSpinBox* leftStickDeadzoneSpin;
    QStackedWidget* leftStickStack;
    QWidget* leftStickCursorWidget; // contains sensi, boosted, smoothing
    QSlider* leftStickCursorSensi;
    QSlider* leftStickCursorBoosted;
    QSlider* leftStickCursorSmoothing;
    QWidget* leftStickScrollWidget; // contains scroll params
    QSlider* leftStickScrollVSensi;
    QSlider* leftStickScrollHSensi;
    QSlider* leftStickScrollVMax;
    QSlider* leftStickScrollHMax;

    // Right Stick
    QGroupBox* rightStickGroup;
    QCheckBox* rightStickEnabled;
    QComboBox* rightStickActionType;
    QSlider* rightStickDeadzone; // (now unused, kept for compatibility)
    QSpinBox* rightStickDeadzoneSpin;
    QStackedWidget* rightStickStack;
    QWidget* rightStickCursorWidget;
    QSlider* rightStickCursorSensi;
    QSlider* rightStickCursorBoosted;
    QSlider* rightStickCursorSmoothing;
    QWidget* rightStickScrollWidget;
    QSlider* rightStickScrollVSensi;
    QSlider* rightStickScrollHSensi;
    QSlider* rightStickScrollVMax;
    QSlider* rightStickScrollHMax;

    // Buttons
    QGroupBox* buttonsGroup;
    QTableWidget* buttonsTable;
    QMap<int, QCheckBox*> buttonEnabledMap;
    QMap<int, QComboBox*> buttonActionTypeMap;
    QMap<int, QComboBox*> buttonKeyMap;
    QMap<int, QCheckBox*> buttonRepeatMap;
    QMap<int, QSlider*> buttonDelayMap;
    QMap<int, QSlider*> buttonIntervalMap;
    QMap<int, QString> buttonRowToName;

    // Triggers
    QGroupBox* leftTriggerGroup;
    QCheckBox* leftTriggerEnabled;
    QComboBox* leftTriggerActionType;
    QSlider* leftTriggerThreshold; // (now unused, kept for compatibility)
    QSpinBox* leftTriggerThresholdSpin;
    QWidget* leftTriggerScrollWidget;
    QComboBox* leftTriggerScrollDir;
    QSlider* leftTriggerScrollVSensi;
    QSlider* leftTriggerScrollVMax;
    QWidget* leftTriggerButtonWidget;
    QComboBox* leftTriggerButtonAction;

    QGroupBox* rightTriggerGroup;
    QCheckBox* rightTriggerEnabled;
    QComboBox* rightTriggerActionType;
    QSlider* rightTriggerThreshold; // (now unused, kept for compatibility)
    QSpinBox* rightTriggerThresholdSpin;
    QWidget* rightTriggerScrollWidget;
    QComboBox* rightTriggerScrollDir;
    QSlider* rightTriggerScrollVSensi;
    QSlider* rightTriggerScrollVMax;
    QWidget* rightTriggerButtonWidget;
    QComboBox* rightTriggerButtonAction;

    // Buttons
    QPushButton* resetButton;
    QPushButton* okButton;
    QVBoxLayout* mainLayout;
    static QMap<QString, ControllerCustomizationWindow*> s_openWindows;
}; 