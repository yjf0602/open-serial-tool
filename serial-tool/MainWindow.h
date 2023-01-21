#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QToolButton>
#include "DockManager.h"
#include "sessions/serialport/SessionSerialPort.h"
#include "AppSettings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initTrayIcon();
    void connectActions();
    void initMenuBarCornerButton();

    void createNewSerialPortSession(QString id, QByteArray configByteArray);

    QString lastSessionConfigsFilePath();
    QString sessionsConfigsFileDirPath();

    void saveCurrentSessionsConfig(QString file);
    void saveWindowState(QString file);
    void loadWindowState(QString file);
    void loadSessionsConfig(QString file);

public slots:
    void slotTrayIconActived(QSystemTrayIcon::ActivationReason reason);

    void slotActionNewSession();
    void slotActionCloseAllSessions();
    void slotActionSaveSessions();
    void slotActionLoadSessions();

    void slotActionToolBar();
    void slotToolBarVisibilityChanged();
    void slotToolBarButtonClicked();

    void slotActionStaysOnTop();

    void slotActionOptions();
    void slotSettingChanged(QString key, QVariant value);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *mTrayIcon;
    QMenu *mTrayMenu;

    QPushButton *mButtonToolBarVisible;

    ads::CDockManager* mDockManager;

    QList<QString> mSessionIds;
    QList<SessionSerialPort*> mSessions;
    QList<ads::CDockWidget*> mDockWidgets;
};

#endif // MAINWINDOW_H
