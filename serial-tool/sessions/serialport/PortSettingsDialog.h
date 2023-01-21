#ifndef PORTSETTINGSDIALOG_H
#define PORTSETTINGSDIALOG_H

#include <QDialog>
#include "SessionSerialPortConfig.h"

namespace Ui {
class PortSettingsDialog;
}

class PortSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PortSettingsDialog(SessionSerialPortConfig config, QWidget *parent = nullptr);
    ~PortSettingsDialog();

signals:
    void signalSettings(
            int dataBits,
            int parity,
            int stopBits,
            int flowControl
            );

private:
    Ui::PortSettingsDialog *ui;
};

#endif // PORTSETTINGSDIALOG_H
