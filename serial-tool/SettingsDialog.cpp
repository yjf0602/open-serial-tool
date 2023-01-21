#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "AppSettings.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    /// 显示当前设置
    if(appSettings->get(Setting_Key_Language) == Setting_Language_English)
        ui->comboBox_language->setCurrentIndex(1);
    ui->checkBox_runOnSystemStartup->setChecked(appSettings->get(Setting_Key_RunOnSystemStartup).toBool());

    connect(ui->pushButton_cancel, &QPushButton::clicked, this, [=](){
        this->close();
    });

    connect(ui->pushButton_ok, &QPushButton::clicked, this, [=](){
        /// 检查设置是否发生变化，进行应用
        QString currentLanguage = Setting_Language_Chinese;
        if(ui->comboBox_language->currentIndex() == 1)
            currentLanguage = Setting_Language_English;
        appSettings->set(Setting_Key_Language, QVariant(currentLanguage));

        appSettings->set(Setting_Key_RunOnSystemStartup, ui->checkBox_runOnSystemStartup->isChecked());
        setProcessAutoRun(QApplication::applicationFilePath(), ui->checkBox_runOnSystemStartup->isChecked());

        this->close();
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
void SettingsDialog::setProcessAutoRun(QString appPath, bool flag)
{
    QSettings settings(AUTO_RUN, QSettings::NativeFormat);

    appPath += " min"; // 作为第二个参数，开机启动后隐藏主界面

    // 以程序名称作为注册表中的键,根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName(); // 键-名称

    // 如果注册表中的路径和当前程序路径不一样，则表示没有设置自启动或本自启动程序已经更换了路径
    QString oldPath = settings.value(name).toString(); // 获取目前的值-绝对路劲
    QString newPath = QDir::toNativeSeparators(appPath);    // toNativeSeparators函数将"/"替换为"\"
    if(flag)
    {
        if (oldPath != newPath)
            settings.setValue(name, newPath);
    }
    else
       settings.remove(name);
}
