#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QVariant>

#define appSettings     AppSettings::instance()

// 字体如果没有进行过设置，则使用资源包里面
#define Setting_Key_Font                    "Font" // 默认使用资源文件版本
#define Setting_Key_Font_Style              "FontStyle"
#define Setting_Key_Font_Size               "FontSize"
#define Setting_Key_Font_Size_Default       9

#define Setting_Key_StayOnTop               "StayOnTop"

#define Setting_Key_Language                "Language"
#define Setting_Language_Chinese            "chinese"
#define Setting_Language_English            "english"

#define Setting_Key_RunOnSystemStartup      "RunOnSystemStartup"

class AppSettings : public QObject
{
    Q_OBJECT
private:
    explicit AppSettings(QObject *parent = nullptr);
    AppSettings(AppSettings&) = delete;
    AppSettings& operator =(const AppSettings&) = delete;
    static AppSettings *mAppSettings;
public:
    static AppSettings *instance(){return mAppSettings;}

    void init();

    void set(QString key, QVariant value);
    QVariant get(QString key);

signals:
    void signalSettingChanged(QString key, QVariant value);

private:
    QString iniFile();
};

#endif // APPSETTINGS_H
