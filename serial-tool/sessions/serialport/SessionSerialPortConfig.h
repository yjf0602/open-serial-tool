#ifndef SESSIONSERIALPORTCONFIG_H
#define SESSIONSERIALPORTCONFIG_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSerialPort>
#include <QSerialPortInfo>

/// \brief The SessionSerialPortConfig class
/// 使用 json 进行保存解析
class SessionSerialPortConfig
{
    friend class SessionSerialPort;
    friend class PortSettingsDialog;
public:
    SessionSerialPortConfig();

    QByteArray toByteArray();
    bool fromByteArray(QByteArray byteArray);

    void updateValueFromObject(QString key, bool &value, QJsonObject &object);
    void updateValueFromObject(QString key, QString &value, QJsonObject &object);
    void updateValueFromObject(QString key, int &value, QJsonObject &object);
    void updateValueFromObject(QString key, QStringList &value, QJsonObject &object);

private:
    QString mConfigPortName;
    int mConfigPortBaud;

    int mConfigPortDataBits;
    int mConfigPortParity;
    int mConfigPortStopBits;
    int mConfigPortFlowControl;

    bool mConfigAutoConnect;

    bool mConfigRxSettingVisible;
    bool mConfigRxWarpLine;
    bool mConfigRxShowHEX;
    bool mConfigRxShowTime;
    QString mConfigRxCodec;

    bool mConfigTxSettingVisible;
    bool mConfigTxWarpLine;
    bool mConfigTxShowHEX;
    QString mConfigTxCodec;

    int mConfigTxResendPeriod;
    QStringList mConfigTxHistory;
    bool mConfigTxVisible;
};

#endif // SESSIONSERIALPORTCONFIG_H
