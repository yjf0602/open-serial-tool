#include "AppSettings.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QDir>

AppSettings * AppSettings::mAppSettings = new AppSettings();

AppSettings::AppSettings(QObject *parent)
    : QObject{parent}
{

}

/*
 * 默认参数初始化
 */
void AppSettings::init()
{
    QSettings settings(iniFile(), QSettings::IniFormat);
    if(!settings.contains(Setting_Key_Font_Size))
        settings.setValue(Setting_Key_Font_Size, Setting_Key_Font_Size_Default);

    if(!settings.contains(Setting_Key_StayOnTop))
        settings.setValue(Setting_Key_StayOnTop, false);

    if(!settings.contains(Setting_Key_Language))
        settings.setValue(Setting_Key_Language, Setting_Language_Chinese);

    if(!settings.contains(Setting_Key_RunOnSystemStartup))
        settings.setValue(Setting_Key_RunOnSystemStartup, false);
}

void AppSettings::set(QString key, QVariant value)
{
    QSettings settings(iniFile(), QSettings::IniFormat);
    if(settings.value(key) != value)
    {
        settings.setValue(key, value);
        emit signalSettingChanged(key, value);
    }
}

QVariant AppSettings::get(QString key)
{
    QSettings settings(iniFile(), QSettings::IniFormat);
    return settings.value(key);
}

QString AppSettings::iniFile()
{
    QString appDataPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1);
    QDir dir;
    if(!dir.exists(appDataPath))
        dir.mkpath(appDataPath);
    return appDataPath + "/settings.ini";
}
