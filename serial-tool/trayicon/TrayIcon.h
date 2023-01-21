#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QObject>
#include <QMenu>
#include <QAction>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);

private:
    QMenu *mMenu;

    QAction *mActionQuit;
};

#endif // TRAYICON_H
