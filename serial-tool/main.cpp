#include <QApplication>
#include <QSystemTrayIcon>
#include <QFontDatabase>
#include <QFont>
#include <QTranslator>
#include "AppSettings.h"
#include "MainWindow.h"
#include <SingleApplication>

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);

    if (QSystemTrayIcon::isSystemTrayAvailable())
        QApplication::setQuitOnLastWindowClosed(false);

    QFile file(":/qss/default.qss");
    file.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(file.readAll());

    appSettings->init();

    int fontid = QFontDatabase::addApplicationFont(":/font/sarasa-mono-sc-regular.ttf");
    if(fontid >= 0)
    {
        QString fontString = QFontDatabase::applicationFontFamilies(fontid).at(0);
        if(appSettings->get(Setting_Key_Font).toString().isEmpty())
            appSettings->set(Setting_Key_Font, fontString);
    }

    if(appSettings->get(Setting_Key_Language).toString() == Setting_Language_Chinese)
    {
        QTranslator *translator = new QTranslator();
        translator->load(":/translations/serial-tool_zh.qm");
        a.installTranslator(translator);
    }
    else if(appSettings->get(Setting_Key_Language).toString() == Setting_Language_English)
    {
        QTranslator *translator = new QTranslator();
        translator->load(":/translations/serial-tool_en.qm");
        a.installTranslator(translator);
    }

    MainWindow w;
    w.show();

    //获取命令行参数，判断是否是【开机自启动】，如果是，最小化。
    QStringList arguments = QCoreApplication::arguments();

    //以第一个参数是程序路径，第二个参数才是min,所以我们取第二个参数。
    if(arguments.size() > 1)
    {
        if(arguments.at(1) == "min")
        {
            w.hide();
        }
    }

    return a.exec();
}
