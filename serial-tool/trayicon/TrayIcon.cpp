#include "TrayIcon.h"
#include <QApplication>

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon{parent}
{

}
