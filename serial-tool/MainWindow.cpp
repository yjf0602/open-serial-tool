#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QGridLayout>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QApplication>
#include <QTranslator>
#include <QDesktopServices>
#include "SettingsDialog.h"
#include "version.h"
#include "AboutDialog.h"

#define SessionConfig_FileDirPath                       "Sessions"
#define SesssionConfigFilePostFix                       ".sessions"
#define Default_Last_SessionConfig_FileName             "LastSessionConfigs.sessions"
#define SessionConfig_Key_MainWindowGeometry            "MainWindow/Geometry"
#define SessionConfig_Key_MainWindowState               "MainWindow/State"
#define SessionConfig_Key_MainWindowDockState           "MainWindow/DockState"
#define SessionConfig_Key_SerialPortSessions            "SerialPortSessions"
#define SessionConfig_Key_SerialPortSession_Config      "ConfigByteArray"
#define SessionConfig_Key_SerialPortSession_Id          "Id"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTrayIcon(nullptr),
    mTrayMenu(nullptr)
{
    ui->setupUi(this);

    if(QSystemTrayIcon::isSystemTrayAvailable())
    {
        initTrayIcon();
    }

    connectActions();

    initMenuBarCornerButton();

    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);

    mDockManager = new ads::CDockManager(this);

    loadSessionsConfig(lastSessionConfigsFilePath());

    if(mSessions.empty())
        createNewSerialPortSession(QUuid::createUuid().toString(), QByteArray());

    if(appSettings->get(Setting_Key_StayOnTop).toBool())
    {
        ui->actionStays_on_top->trigger();
    }

    connect(appSettings, SIGNAL(signalSettingChanged(QString,QVariant)), this, SLOT(slotSettingChanged(QString,QVariant)));

    loadWindowState(lastSessionConfigsFilePath());

    setWindowTitle(windowTitle() + " V" + VERSION);
}

MainWindow::~MainWindow()
{
    QFile::remove(lastSessionConfigsFilePath());
    saveCurrentSessionsConfig(lastSessionConfigsFilePath());
    saveWindowState(lastSessionConfigsFilePath());

    appSettings->set(Setting_Key_StayOnTop, ui->actionStays_on_top->isChecked());

    delete ui;
}

void MainWindow::initTrayIcon()
{
    mTrayIcon = new QSystemTrayIcon(this);
    connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotTrayIconActived(QSystemTrayIcon::ActivationReason)));

    mTrayIcon->setIcon(QIcon(":/icon.ico"));
    mTrayIcon->show();

    ////////////////////////////////// 系统托盘菜单初始化
    mTrayMenu = new QMenu();
    mTrayIcon->setContextMenu(mTrayMenu);

    mTrayMenu->addAction(ui->actionQuit);
}

void MainWindow::connectActions()
{
    connect(ui->actionNew_session, SIGNAL(triggered(bool)), this, SLOT(slotActionNewSession()));

    connect(ui->actionClose_all_sessions, SIGNAL(triggered(bool)), this, SLOT(slotActionCloseAllSessions()));
    connect(ui->actionSave_all_sessions, SIGNAL(triggered(bool)), this, SLOT(slotActionSaveSessions()));
    connect(ui->actionLoad_sessions, SIGNAL(triggered(bool)), this, SLOT(slotActionLoadSessions()));

    connect(ui->actiontoolBar, SIGNAL(triggered(bool)), this, SLOT(slotActionToolBar()));
    connect(ui->toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(slotToolBarVisibilityChanged()));

    connect(ui->actionQuit, &QAction::triggered, this, [=](){
        QApplication::quit();
    });

    connect(ui->actionStays_on_top, SIGNAL(triggered(bool)), this, SLOT(slotActionStaysOnTop()));

    connect(ui->actionOptions, SIGNAL(triggered(bool)), this, SLOT(slotActionOptions()));

    connect(ui->actionAbout, &QAction::triggered, this, [=](){
        AboutDialog *dialog = new AboutDialog(this);
        dialog->setModal(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->show();
    });
}

void MainWindow::initMenuBarCornerButton()
{
    mButtonToolBarVisible = new QPushButton(this);
    mButtonToolBarVisible->setObjectName("toolbarArrow");
    mButtonToolBarVisible->setCheckable(true);
    mButtonToolBarVisible->setChecked(false);
    connect(mButtonToolBarVisible, SIGNAL(clicked(bool)), this, SLOT(slotToolBarButtonClicked()));
    mButtonToolBarVisible->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    ui->menuBar->setCornerWidget(mButtonToolBarVisible);
}

void MainWindow::createNewSerialPortSession(QString id, QByteArray configByteArray)
{
    auto dw = new ads::CDockWidget(id, this);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    dw->setFeature(ads::CDockWidget::DeleteContentOnClose, true);

    SessionSerialPort *session = new SessionSerialPort(this, dw, configByteArray);
    dw->setWidget(session);

    mDockManager->addDockWidget(ads::CenterDockWidgetArea, dw);

    mSessionIds.push_back(id);
    mSessions.push_back(session);
    mDockWidgets.push_back(dw);

    connect(dw, &ads::CDockWidget::closed, this, [=](){
        int dwIndex = mDockWidgets.indexOf(dw);
        if(dwIndex >=0 && dwIndex < mSessions.size())
        {
            mSessionIds.removeAt(dwIndex);
            mSessions.removeAt(dwIndex);
            mDockWidgets.removeAt(dwIndex);
        }
    });
}

QString MainWindow::lastSessionConfigsFilePath()
{
    QString appDataPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1);
    return appDataPath + "/" + Default_Last_SessionConfig_FileName;
}

QString MainWindow::sessionsConfigsFileDirPath()
{
    QString appDataPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1);
    QString configsFileDir = appDataPath + "/" + SessionConfig_FileDirPath;
    QDir dir;
    if(!dir.exists(configsFileDir))
        dir.mkpath(configsFileDir);
    return configsFileDir;
}

void MainWindow::saveCurrentSessionsConfig(QString file)
{
    QSettings settings(file, QSettings::IniFormat);

    settings.beginWriteArray(SessionConfig_Key_SerialPortSessions);
    for(int i=0; i<mSessions.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue(SessionConfig_Key_SerialPortSession_Config,
                          mSessions[i]->configByteArray());
        settings.setValue(SessionConfig_Key_SerialPortSession_Id,
                          mSessionIds[i]);
    }
    settings.endArray();
}

void MainWindow::saveWindowState(QString file)
{
    QSettings settings(file, QSettings::IniFormat);
    settings.setValue(SessionConfig_Key_MainWindowGeometry, this->saveGeometry());
    settings.setValue(SessionConfig_Key_MainWindowState, this->saveState());
    settings.setValue(SessionConfig_Key_MainWindowDockState, mDockManager->saveState());
}

void MainWindow::loadWindowState(QString file)
{
    QSettings settings(file, QSettings::IniFormat);
    if(settings.contains(SessionConfig_Key_MainWindowGeometry))
        this->restoreGeometry(settings.value(SessionConfig_Key_MainWindowGeometry).toByteArray());
    if(settings.contains(SessionConfig_Key_MainWindowState))
        this->restoreState(settings.value(SessionConfig_Key_MainWindowState).toByteArray());
    if(settings.contains(SessionConfig_Key_MainWindowDockState))
        mDockManager->restoreState(settings.value(SessionConfig_Key_MainWindowDockState).toByteArray());
}

void MainWindow::loadSessionsConfig(QString file)
{
    QSettings settings(file, QSettings::IniFormat);

    int size = settings.beginReadArray(SessionConfig_Key_SerialPortSessions);
    for(int i=0; i<size; i++)
    {
        settings.setArrayIndex(i);
        QByteArray configByteArray = settings.value(SessionConfig_Key_SerialPortSession_Config).toByteArray();
        QString id = settings.value(SessionConfig_Key_SerialPortSession_Id).toString();
        createNewSerialPortSession(id, configByteArray);
    }
    settings.endArray();
}

void MainWindow::slotTrayIconActived(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::MiddleClick:
            this->setVisible(true);
            this->activateWindow();
            this->showNormal();
            break;
        default:
            ;
    }
}

void MainWindow::slotActionNewSession()
{
    createNewSerialPortSession(QUuid::createUuid().toString(), QByteArray());
}

void MainWindow::slotActionCloseAllSessions()
{
    for(int i=0; i<mDockWidgets.size(); i++)
    {
        mDockManager->removeDockWidget(mDockWidgets[i]);
        mDockWidgets[i]->close();
        mDockWidgets[i]->deleteLater();
    }

    mDockWidgets.clear();
    mSessions.clear();
}

void MainWindow::slotActionSaveSessions()
{
    QString dirPath = sessionsConfigsFileDirPath();
    QString fileName = dirPath + "/" + QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") + SesssionConfigFilePostFix;
    saveCurrentSessionsConfig(fileName);
    QDesktopServices::openUrl(QUrl("file:///" + dirPath));
}

void MainWindow::slotActionLoadSessions()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, "Load sessions", sessionsConfigsFileDirPath(), QString("Sessions (*") + SesssionConfigFilePostFix + ")");
    if(fileName.isEmpty())
        return;

    slotActionCloseAllSessions();
    loadSessionsConfig(fileName);
}

void MainWindow::slotActionToolBar()
{
    ui->toolBar->setVisible(ui->actiontoolBar->isChecked());
    mButtonToolBarVisible->setChecked(ui->actiontoolBar->isChecked());
}

void MainWindow::slotToolBarVisibilityChanged()
{
    ui->actiontoolBar->setChecked(ui->toolBar->isVisible());
    mButtonToolBarVisible->setChecked(ui->actiontoolBar->isChecked());
}

void MainWindow::slotToolBarButtonClicked()
{
    ui->toolBar->setVisible(mButtonToolBarVisible->isChecked());
}

void MainWindow::slotActionStaysOnTop()
{
    setWindowFlag(Qt::WindowStaysOnTopHint, ui->actionStays_on_top->isChecked());

    show();

    for(int i=0; i < mDockManager->dockAreaCount(); i++)
    {
        QWidget *widget = (QWidget*)(mDockManager->dockArea(i));
        if(widget)
        {
            widget->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        }
    }
}

void MainWindow::slotActionOptions()
{
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
}

void MainWindow::slotSettingChanged(QString key, QVariant value)
{
    if(key == Setting_Key_Language)
    {
        if(value.toString() == Setting_Language_Chinese)
        {
            QTranslator *translator = new QTranslator();
            translator->load(":/translations/serial-tool_zh.qm");
            qApp->installTranslator(translator);
        }
        else if(value.toString() == Setting_Language_English)
        {
            QTranslator *translator = new QTranslator();
            translator->load(":/translations/serial-tool_en.qm");
            qApp->installTranslator(translator);
        }
        ui->retranslateUi(this);

        for(int i=0; i<mSessions.size(); i++)
        {
            mSessions[i]->updateUiWithNewLanguage();
        }
    }
}
