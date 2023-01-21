#include "SessionSerialPortConfig.h"
#include "common/AppSettings.h"
#include <QDebug>

#define Key_PortName                "PortName"
#define Key_PortBaud                "PortBaud"
#define Key_PortDataBits            "PortDataBits"
#define Key_PortParity              "PortParity"
#define Key_PortStopBits            "PortStopBits"
#define Key_PortFlowControl         "PortFlowControl"

#define Key_AutoConnect             "AutoConnect"
#define Key_RxSettingsVisible       "RxSettingsVisible"
#define Key_RxWarpLine              "RxWarpLine"
#define Key_RxShowHex               "RxShowHex"
#define Key_RxShowTime              "RxShowTime"
#define Key_RxCodec                 "RxCodec"

#define Key_TxSettingsVisible       "TxSettingsVisible"
#define Key_TxWarpLine              "TxWarpLine"
#define Key_TxShowHex               "TxShowHex"
#define Key_TxResendPeriod          "TxResendPeriod"
#define Key_TxHistory               "TxHistory"
#define Key_TxVisible               "TxVisible"
#define Key_TxCodec                 "TxCodec"

SessionSerialPortConfig::SessionSerialPortConfig() :
    mConfigPortDataBits(QSerialPort::Data8),
    mConfigPortParity(QSerialPort::NoParity),
    mConfigPortStopBits(QSerialPort::OneStop),
    mConfigPortFlowControl(QSerialPort::NoFlowControl),
    mConfigRxCodec("UTF-8"),
    mConfigTxCodec("UTF-8")
{

}

QByteArray SessionSerialPortConfig::toByteArray()
{
    QJsonObject object;

    object.insert(Key_PortName, mConfigPortName);
    object.insert(Key_PortBaud, mConfigPortBaud);

    object.insert(Key_PortDataBits, mConfigPortDataBits);
    object.insert(Key_PortParity, mConfigPortParity);
    object.insert(Key_PortStopBits, mConfigPortStopBits);
    object.insert(Key_PortFlowControl, mConfigPortFlowControl);

    object.insert(Key_AutoConnect, mConfigAutoConnect);

    object.insert(Key_RxSettingsVisible, mConfigRxSettingVisible);
    object.insert(Key_RxWarpLine, mConfigRxWarpLine);
    object.insert(Key_RxShowHex, mConfigRxShowHEX);
    object.insert(Key_RxShowTime, mConfigRxShowTime);
    object.insert(Key_RxCodec, mConfigRxCodec);

    object.insert(Key_TxSettingsVisible, mConfigTxSettingVisible);
    object.insert(Key_TxWarpLine, mConfigTxWarpLine);
    object.insert(Key_TxShowHex, mConfigTxShowHEX);
    object.insert(Key_TxCodec, mConfigTxCodec);

    object.insert(Key_TxResendPeriod, mConfigTxResendPeriod);

    QJsonArray histArray;
    for(int i=0; i<mConfigTxHistory.size(); i++)
        histArray.append(mConfigTxHistory[i]);
    object.insert(Key_TxHistory, QJsonValue(histArray));

    object.insert(Key_TxVisible, mConfigTxVisible);

    QJsonDocument document;
    document.setObject(object);
    return document.toJson(QJsonDocument::Compact);
}

bool SessionSerialPortConfig::fromByteArray(QByteArray byteArray)
{
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(byteArray, &jsonError);
    if( document.isNull() || (jsonError.error != QJsonParseError::NoError))
    {
        qCritical() << jsonError.errorString();
        return false;
    }

    if( !document.isObject() )
        return false;

    QJsonObject object = document.object();
    updateValueFromObject(Key_PortName, mConfigPortName, object);
    updateValueFromObject(Key_PortBaud, mConfigPortBaud, object);

    updateValueFromObject(Key_PortDataBits, mConfigPortDataBits, object);
    updateValueFromObject(Key_PortParity, mConfigPortParity, object);
    updateValueFromObject(Key_PortStopBits, mConfigPortStopBits, object);
    updateValueFromObject(Key_PortFlowControl, mConfigPortFlowControl, object);

    updateValueFromObject(Key_AutoConnect, mConfigAutoConnect, object);

    updateValueFromObject(Key_RxSettingsVisible, mConfigRxSettingVisible, object);
    updateValueFromObject(Key_RxWarpLine, mConfigRxWarpLine, object);
    updateValueFromObject(Key_RxShowHex, mConfigRxShowHEX, object);
    updateValueFromObject(Key_RxShowTime, mConfigRxShowTime, object);
    updateValueFromObject(Key_RxCodec, mConfigRxCodec, object);

    updateValueFromObject(Key_TxSettingsVisible, mConfigTxSettingVisible, object);
    updateValueFromObject(Key_TxWarpLine, mConfigTxWarpLine, object);
    updateValueFromObject(Key_TxShowHex, mConfigTxShowHEX, object);
    updateValueFromObject(Key_TxCodec, mConfigTxCodec, object);

    updateValueFromObject(Key_TxResendPeriod, mConfigTxResendPeriod, object);
    updateValueFromObject(Key_TxHistory, mConfigTxHistory, object);

    updateValueFromObject(Key_TxVisible, mConfigTxVisible, object);

    return true;
}

void SessionSerialPortConfig::updateValueFromObject(QString key, bool &value, QJsonObject &object)
{
    if(object.contains(key))
        value = object.value(key).toBool();
}

void SessionSerialPortConfig::updateValueFromObject(QString key, QString &value, QJsonObject &object)
{
    if(object.contains(key))
        value = object.value(key).toString();
}

void SessionSerialPortConfig::updateValueFromObject(QString key, int &value, QJsonObject &object)
{
    if(object.contains(key))
        value = object.value(key).toInt();
}

void SessionSerialPortConfig::updateValueFromObject(QString key, QStringList &value, QJsonObject &object)
{
    if(object.contains(key))
    {
        QJsonArray array = object.value(key).toArray();
        if(array.isEmpty())
            return;

        value.clear();
        for(int i=0; i<array.size(); i++)
        {
            value.append(array.at(i).toString());
        }
    }
}
