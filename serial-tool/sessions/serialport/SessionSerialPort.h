#ifndef SESSIONSERIALPORT_H
#define SESSIONSERIALPORT_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include "SessionSerialPortConfig.h"
#include "DockManager.h"

namespace Ui {
class SessionSerialPort;
}

class SessionSerialPort : public QWidget
{
    Q_OBJECT

public:
    explicit SessionSerialPort(QWidget *parent, ads::CDockWidget *dockWidget, QByteArray configByteArray);
    ~SessionSerialPort();

    void updateTexteditFontFromSettings();
    QByteArray configByteArray();

    void updateUiWithNewLanguage();

public slots:
    void slotCheckAvailableSerialPort();
    void slotAutoConnect();

    void slotComboBoxPortActived(int index);

    void slotButtonOpenSerialPort();
    void slotButtonClear();

    void slotButtonSaveRxData();

    void slotSerialPortErrorOccurred(QSerialPort::SerialPortError error);
    void slotSerialPortReadReady();

    void slotButtonSerialPortSettings();

    void slotButtonSend();

    void slotCheckBoxResend();
    void slotResendTimeout();

    void slotComboBoxTxHistoryActived(QString text);

protected:
    void restoreConfig(QByteArray configByteArray);
    void updateUIWithConfig();
    void updateConfigWithUI();
    QString getPortNameFromUI();
    void updateSeiralPortState();

    void initRxSettings();

    bool isSerialPortExist(QString portName);

    void saveBinaryData(QString fileName);
    void saveHexData(QString fileName);
    void saveHexWithTextData(QString fileName);
    void saveHexWithTextAndTimeData(QString fileName);
    void saveTextData(QString fileName);
    void saveTextWithTimeData(QString fileName);

    void saveTextToFile(QString text, QString fileName);

    void sendHexData();
    void sendTextData();

    void updateCntLabel();

private:
    Ui::SessionSerialPort *ui;
    ads::CDockWidget *mDockWidget;

    SessionSerialPortConfig mConfig;

    QSerialPort mSerialPort;

    bool mAutoConnectTriedButFailed;
    bool mDisconnectByUser;

    QTextCodec *mCodecGB;

    qint64 mSendCnt;

    QTimer *mResendTimer;
};

#endif // SESSIONSERIALPORT_H
