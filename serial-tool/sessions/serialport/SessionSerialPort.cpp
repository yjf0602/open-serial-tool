#include "SessionSerialPort.h"
#include "ui_SessionSerialPort.h"
#include "AppSettings.h"
#include <QAction>
#include <QLineEdit>
#include <QTimer>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QFile>
#include <fstream>
#include "PortSettingsDialog.h"

SessionSerialPort::SessionSerialPort(QWidget *parent, ads::CDockWidget *dockWidget, QByteArray configByteArray) :
    QWidget(parent),
    mDockWidget(dockWidget),
    ui(new Ui::SessionSerialPort),
    mAutoConnectTriedButFailed(false),
    mDisconnectByUser(false),
    mSendCnt(0)
{
    ui->setupUi(this);

    mCodecGB = QTextCodec::codecForName("GB-18030");

    //////////////////////////////////// SerialPort
    connect(&mSerialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
            this, SLOT(slotSerialPortErrorOccurred(QSerialPort::SerialPortError)));
    connect(&mSerialPort, SIGNAL(readyRead()),
            this, SLOT(slotSerialPortReadReady()));

    connect(ui->pushButton_serialPortSettings, SIGNAL(clicked(bool)),
            this, SLOT(slotButtonSerialPortSettings()));

    connect(ui->pushButton_openSerialPort, SIGNAL(clicked(bool)),
            this, SLOT(slotButtonOpenSerialPort()));

    connect(ui->pushButton_clearSerialPortData, SIGNAL(clicked(bool)),
            this, SLOT(slotButtonClear()));

    connect(ui->pushButton_ssp_rx_save, SIGNAL(clicked(bool)),
            this, SLOT(slotButtonSaveRxData()));

    connect(ui->pushButton_ssp_rx_settings, &QPushButton::toggled,
            ui->widget_rx_settings, &QWidget::setVisible);
    ui->widget_rx_settings->setVisible(false);

    connect(ui->pushButton_ssp_tx_settings, &QPushButton::toggled,
            ui->widget_tx_settings, &QWidget::setVisible);
    ui->widget_tx_settings->setVisible(false);

    connect(ui->checkBox_tx_visible, &QCheckBox::toggled,
            ui->widget_ssp_tx, &QWidget::setVisible);

    connect(ui->pushButton_send, SIGNAL(clicked(bool)), this, SLOT(slotButtonSend()));

    updateTexteditFontFromSettings();
    ui->textedit_ssp_rx->setReadOnly(true);

    connect(ui->comboBox_port, &QComboBox::currentTextChanged, this, [=](){
        QString portName = getPortNameFromUI();
        if(portName.isEmpty())
        {
            mDockWidget->setWindowTitle("Serial Port");
            return;
        }
        mDockWidget->setWindowTitle(portName);
    });

    connect(ui->comboBox_port, SIGNAL(activated(int)), this, SLOT(slotComboBoxPortActived(int)));
    slotCheckAvailableSerialPort();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotCheckAvailableSerialPort()));
    connect(timer, SIGNAL(timeout()), this, SLOT(slotAutoConnect()));
    timer->start(200);

    connect(ui->checkBox_resend, SIGNAL(clicked(bool)), this, SLOT(slotCheckBoxResend()));
    mResendTimer = new QTimer(this);
    connect(mResendTimer, SIGNAL(timeout()), this, SLOT(slotResendTimeout()));

    connect(ui->comboBox_tx_history, SIGNAL(activated(QString)), this, SLOT(slotComboBoxTxHistoryActived(QString)));

    updateConfigWithUI();

    restoreConfig(configByteArray);

    initRxSettings();

    updateSeiralPortState();
}

SessionSerialPort::~SessionSerialPort()
{
    delete ui;
}

void SessionSerialPort::updateTexteditFontFromSettings()
{
    QString fontName = appSettings->get(Setting_Key_Font).toString();
    int fontSize = appSettings->get(Setting_Key_Font_Size).toInt();
    QString style = appSettings->get(Setting_Key_Font_Style).toString();
    ui->textedit_ssp_rx->setFonts(fontName, fontSize, style);
    ui->textedit_ssp_tx->setFonts(fontName, fontSize, style);
}

QByteArray SessionSerialPort::configByteArray()
{
    updateConfigWithUI();
    return mConfig.toByteArray();
}

void SessionSerialPort::updateUiWithNewLanguage()
{
    ui->retranslateUi(this);
}

void SessionSerialPort::slotCheckAvailableSerialPort()
{
    QString currentSerialPotName = ui->comboBox_port->currentText();
    int currentCursorPos = ui->comboBox_port->lineEdit()->cursorPosition();

    QStringList availablePortList;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString text = info.portName() + " (" + info.description() + ")";
        availablePortList.append(text);
    }

    /// 检查是否发生了变化，如果没有，直接返回
    if(availablePortList.size() == ui->comboBox_port->count())
    {
        bool noChange = true;
        for(int i=0; i<ui->comboBox_port->count(); i++)
        {
            if(!availablePortList.contains(ui->comboBox_port->itemText(i)))
            {
                noChange = false;
                break;
            }
        }
        if(noChange)
            return;
    }

    ui->comboBox_port->clear();
    ui->comboBox_port->addItems(availablePortList);

    /// 更新下拉选项长度
    if(!availablePortList.isEmpty())
    {
        int maxWidth = 0;
        QFontMetrics fm(ui->comboBox_port->font());
        for(int i=0; i<availablePortList.size(); i++)
        {
            int width = fm.boundingRect(availablePortList[i]).width(); // 计算字符串的像素宽度
            if(width > maxWidth)
                maxWidth = width;
        }
        ui->comboBox_port->view()->setMinimumWidth(maxWidth + 8);
        ui->comboBox_port->view()->setMinimumHeight(ui->comboBox_port->height() * availablePortList.length());
    }

    ui->comboBox_port->setCurrentText(currentSerialPotName);
    ui->comboBox_port->lineEdit()->setCursorPosition(currentCursorPos);

    if(currentSerialPotName.isEmpty())
    {
        ui->comboBox_port->setCurrentIndex(0);
        slotComboBoxPortActived(0);
    }
}

void SessionSerialPort::slotAutoConnect()
{
    if(mDisconnectByUser)
        return;

    if(!ui->pushButton_autoOpen->isChecked())
        return;

    if(mSerialPort.isOpen())
        return;

    QString portName = getPortNameFromUI();
    if(portName.isEmpty())
        return;

    if(!isSerialPortExist(portName))
        return;

    if(mAutoConnectTriedButFailed)
        return;

    mSerialPort.setPortName(getPortNameFromUI());
    mSerialPort.setBaudRate(ui->comboBox_baudrate->currentText().toInt());
    mSerialPort.setDataBits((QSerialPort::DataBits)mConfig.mConfigPortDataBits);
    mSerialPort.setParity((QSerialPort::Parity)mConfig.mConfigPortParity);
    mSerialPort.setStopBits((QSerialPort::StopBits)mConfig.mConfigPortStopBits);
    mSerialPort.setFlowControl((QSerialPort::FlowControl)mConfig.mConfigPortFlowControl);

    if(mSerialPort.open(QIODevice::ReadWrite))
    {
        ui->pushButton_openSerialPort->setChecked(true);

        ui->comboBox_port->setEnabled(false);
        ui->comboBox_baudrate->setEnabled(false);
        ui->pushButton_send->setEnabled(true);
    }
    else
    {
        ui->pushButton_openSerialPort->setChecked(false);
        mAutoConnectTriedButFailed = true;
    }

    updateSeiralPortState();
}

void SessionSerialPort::slotComboBoxPortActived(int index)
{
    Q_UNUSED(index);
    ui->comboBox_port->lineEdit()->setCursorPosition(0);
}

void SessionSerialPort::slotButtonOpenSerialPort()
{
    mAutoConnectTriedButFailed = false;

    if(mSerialPort.isOpen())
    {
        mSerialPort.close();
        ui->pushButton_openSerialPort->setChecked(false);

        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);

        ui->pushButton_send->setEnabled(false);

        mDisconnectByUser = true;
    }
    else
    {
        mSerialPort.setPortName(getPortNameFromUI());
        mSerialPort.setBaudRate(ui->comboBox_baudrate->currentText().toInt());
        mSerialPort.setDataBits((QSerialPort::DataBits)mConfig.mConfigPortDataBits);
        mSerialPort.setParity((QSerialPort::Parity)mConfig.mConfigPortParity);
        mSerialPort.setStopBits((QSerialPort::StopBits)mConfig.mConfigPortStopBits);
        mSerialPort.setFlowControl((QSerialPort::FlowControl)mConfig.mConfigPortFlowControl);

        if(mSerialPort.open(QIODevice::ReadWrite))
        {
            ui->pushButton_openSerialPort->setChecked(true);

            ui->comboBox_port->setEnabled(false);
            ui->comboBox_baudrate->setEnabled(false);
            ui->pushButton_send->setEnabled(true);
        }
        else
        {
            ui->pushButton_openSerialPort->setChecked(false);

            QMessageBox::information(
                        this,
                        "Error",
                        tr("Failed to open serial port: ") + mSerialPort.portName() + " with error: " + mSerialPort.errorString(),
                        QMessageBox::Ok);
        }

        mDisconnectByUser = false;
    }

    updateSeiralPortState();
}

void SessionSerialPort::slotButtonClear()
{
    ui->textedit_ssp_rx->clearAll();
    mSendCnt = 0;
    updateCntLabel();
}

void SessionSerialPort::slotButtonSaveRxData()
{
    if(ui->textedit_ssp_rx->rxDataSize() <= 0)
        return;

    QString appDataPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1);
    QString dataPath = appDataPath + "/RxData";
    QDir dir;
    if(!dir.exists(dataPath))
        dir.mkpath(dataPath);

    QString rxDataTimeTag = "RxData-" + QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
    QString currentDataPath = dataPath + "/" + rxDataTimeTag;
    dir.mkpath(currentDataPath);

    saveBinaryData(             currentDataPath + "/" + rxDataTimeTag + ".binary");
    saveHexData(                currentDataPath + "/" + rxDataTimeTag + "_hex.txt");
    saveHexWithTextData(        currentDataPath + "/" + rxDataTimeTag + "_hex_text.txt");
    saveHexWithTextAndTimeData( currentDataPath + "/" + rxDataTimeTag + "_hex_text_time.txt");
    saveTextData(               currentDataPath + "/" + rxDataTimeTag + "_text.txt");
    saveTextWithTimeData(       currentDataPath + "/" + rxDataTimeTag + "_text_time.txt");

    dir.setPath(currentDataPath);
    QDesktopServices::openUrl(QUrl(dir.absolutePath()));
}

void SessionSerialPort::slotSerialPortErrorOccurred(QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::NoError)
    {
        qDebug() << error;
        if(mSerialPort.isOpen())
            mSerialPort.close();

        ui->pushButton_openSerialPort->setChecked(false);

        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);

        ui->pushButton_send->setEnabled(false);

        mDisconnectByUser = false;
    }

    updateSeiralPortState();
}

void SessionSerialPort::slotSerialPortReadReady()
{
    QByteArray allData = mSerialPort.readAll();
    if(allData.isEmpty())
        return;

    ui->textedit_ssp_rx->appendByteArray(allData);
    updateCntLabel();
}

void SessionSerialPort::slotButtonSerialPortSettings()
{
    PortSettingsDialog *dialog = new PortSettingsDialog(mConfig, this);
    dialog->setModal(true);
    connect(dialog, &PortSettingsDialog::signalSettings, this, [=](
            int dataBits,
            int parity,
            int stopBits,
            int flowControl)
    {
        mConfig.mConfigPortDataBits = dataBits;
        mConfig.mConfigPortParity = parity;
        mConfig.mConfigPortStopBits = stopBits;
        mConfig.mConfigPortFlowControl = flowControl;
        mSerialPort.setDataBits((QSerialPort::DataBits)dataBits);
        mSerialPort.setParity((QSerialPort::Parity)parity);
        mSerialPort.setStopBits((QSerialPort::StopBits)stopBits);
        mSerialPort.setFlowControl((QSerialPort::FlowControl)flowControl);
        updateSeiralPortState();
    });
    dialog->show();
}

void SessionSerialPort::slotButtonSend()
{
    if(ui->radioButton_tx_ascii->isChecked())
    {
        sendTextData();
    }
    else
    {
        sendHexData();
    }

    QString text = ui->textedit_ssp_tx->text();
    int index = ui->comboBox_tx_history->findText(text);
    if(index == 0)
        return;

    if(index > 0)
    {
        ui->comboBox_tx_history->removeItem(index);
    }

    ui->comboBox_tx_history->insertItem(0, text);
    ui->comboBox_tx_history->setCurrentIndex(0);
}

void SessionSerialPort::slotCheckBoxResend()
{
    if(ui->checkBox_resend->isChecked())
    {
        int ms = ui->spinBox_resend_time->text().toInt();
        mResendTimer->start(ms);
    }
    else
    {
        mResendTimer->stop();
    }
}

void SessionSerialPort::slotResendTimeout()
{
    if(!ui->checkBox_resend->isChecked())
        return;

    if(!mSerialPort.isOpen())
        return;

    slotButtonSend();

    int ms = ui->spinBox_resend_time->text().toInt();
    if(ms != mResendTimer->interval())
        mResendTimer->setInterval(ms);
}

void SessionSerialPort::slotComboBoxTxHistoryActived(QString text)
{
    ui->textedit_ssp_tx->setText(text);
}

void SessionSerialPort::restoreConfig(QByteArray configByteArray)
{
    if(configByteArray.isEmpty())
        return;

    if(!mConfig.fromByteArray(configByteArray))
    {
        qCritical() << "Failed to restore from configByteArray";
        return;
    }

    updateUIWithConfig();
}

void SessionSerialPort::updateUIWithConfig()
{
    ui->comboBox_port->setCurrentText(mConfig.mConfigPortName);
    ui->comboBox_port->lineEdit()->setCursorPosition(0);
    ui->comboBox_baudrate->setCurrentText(QString::number(mConfig.mConfigPortBaud));

    ui->pushButton_autoOpen->setChecked(mConfig.mConfigAutoConnect);

    ui->pushButton_ssp_rx_settings->setChecked(mConfig.mConfigRxSettingVisible);
    ui->checkBox_rx_warpLine->setChecked(mConfig.mConfigRxWarpLine);
    ui->radioButton_rx_hex->setChecked(mConfig.mConfigRxShowHEX);
    ui->radioButton_rx_ascii->setChecked(!mConfig.mConfigRxShowHEX);
    ui->checkBox_rx_showTime->setChecked(mConfig.mConfigRxShowTime);
    ui->comboBox_rxCodec->setCurrentText(mConfig.mConfigRxCodec);

    ui->pushButton_ssp_tx_settings->setChecked(mConfig.mConfigTxSettingVisible);
    ui->checkBox_tx_warpLine->setChecked(mConfig.mConfigTxWarpLine);
    ui->radioButton_tx_hex->setChecked(mConfig.mConfigTxShowHEX);
    ui->radioButton_tx_ascii->setChecked(!mConfig.mConfigTxShowHEX);
    ui->comboBox_txCodec->setCurrentText(mConfig.mConfigTxCodec);

    ui->spinBox_resend_time->setValue(mConfig.mConfigTxResendPeriod);
    ui->comboBox_tx_history->clear();
    ui->comboBox_tx_history->addItems(mConfig.mConfigTxHistory);

    ui->checkBox_tx_visible->setChecked(mConfig.mConfigTxVisible);
}

void SessionSerialPort::updateConfigWithUI()
{
    mConfig.mConfigPortName = ui->comboBox_port->currentText();
    mConfig.mConfigPortBaud = ui->comboBox_baudrate->currentText().toInt();

    mConfig.mConfigAutoConnect = ui->pushButton_autoOpen->isChecked();

    mConfig.mConfigRxSettingVisible = ui->pushButton_ssp_rx_settings->isChecked();
    mConfig.mConfigRxWarpLine = ui->checkBox_rx_warpLine->isChecked();
    mConfig.mConfigRxShowHEX = ui->radioButton_rx_hex->isChecked();
    mConfig.mConfigRxShowTime = ui->checkBox_rx_showTime->isChecked();
    mConfig.mConfigRxCodec = ui->comboBox_rxCodec->currentText();

    mConfig.mConfigTxSettingVisible = ui->pushButton_ssp_tx_settings->isChecked();
    mConfig.mConfigTxWarpLine = ui->checkBox_tx_warpLine->isChecked();
    mConfig.mConfigTxShowHEX = ui->radioButton_tx_hex->isChecked();
    mConfig.mConfigTxCodec = ui->comboBox_txCodec->currentText();

    mConfig.mConfigTxResendPeriod = ui->spinBox_resend_time->value();
    mConfig.mConfigTxHistory.clear();
    for(int i=0; i<ui->comboBox_tx_history->count(); i++)
    {
        mConfig.mConfigTxHistory.append(ui->comboBox_tx_history->itemText(i));
    }

    mConfig.mConfigTxVisible = ui->checkBox_tx_visible->isChecked();
}

QString SessionSerialPort::getPortNameFromUI()
{
    QString port = ui->comboBox_port->currentText();
    QStringList list = port.split(" ");
    if(list.empty())
        return QString();
    return list.first();
}

void SessionSerialPort::updateSeiralPortState()
{
    QString text;
    if(mSerialPort.isOpen())
        text += mSerialPort.portName() + " Opened, ";
    else
        text += "Closed, ";

    text += QString::number(mConfig.mConfigPortBaud) + ", ";

    if(mConfig.mConfigPortDataBits == QSerialPort::Data5)
        text += "5/";
    else if(mConfig.mConfigPortDataBits == QSerialPort::Data6)
        text += "6/";
    else if(mConfig.mConfigPortDataBits == QSerialPort::Data7)
        text += "7/";
    else if(mConfig.mConfigPortDataBits == QSerialPort::Data8)
        text += "8/";

    if(mConfig.mConfigPortStopBits == QSerialPort::OneStop)
        text += "1, ";
    else if(mConfig.mConfigPortStopBits == QSerialPort::OneAndHalfStop)
        text += "1.5, ";
    else if(mConfig.mConfigPortStopBits == QSerialPort::TwoStop)
        text += "2, ";

    if(mConfig.mConfigPortParity == QSerialPort::NoParity)
        text += "none, ";
    else if(mConfig.mConfigPortParity == QSerialPort::EvenParity)
        text += "even, ";
    else if(mConfig.mConfigPortParity == QSerialPort::OddParity)
        text += "odd, ";
    else if(mConfig.mConfigPortParity == QSerialPort::SpaceParity)
        text += "space, ";
    else if(mConfig.mConfigPortParity == QSerialPort::MarkParity)
        text += "mark, ";

    if(mConfig.mConfigPortFlowControl == QSerialPort::NoFlowControl)
        text += "none";
    else if(mConfig.mConfigPortFlowControl == QSerialPort::HardwareControl)
        text += "RTS/CTS";
    else
        text += "XON/XOFF";

    ui->label_serialport_state->setText(text);
}

void SessionSerialPort::initRxSettings()
{
    ui->textedit_ssp_rx->setWrap(ui->checkBox_rx_warpLine->isChecked());
    ui->textedit_ssp_rx->setShowTime(ui->checkBox_rx_showTime->isChecked());
    ui->textedit_ssp_rx->setShowHex(ui->radioButton_rx_hex->isChecked());
    ui->textedit_ssp_rx->setTextCodec(ui->comboBox_rxCodec->currentText());

    connect(ui->checkBox_rx_warpLine, SIGNAL(toggled(bool)), ui->textedit_ssp_rx, SLOT(setWrap(bool)));
    connect(ui->checkBox_rx_showTime, SIGNAL(toggled(bool)), ui->textedit_ssp_rx, SLOT(setShowTime(bool)));
    connect(ui->radioButton_rx_hex, SIGNAL(toggled(bool)), ui->textedit_ssp_rx, SLOT(setShowHex(bool)));
    connect(ui->comboBox_rxCodec, SIGNAL(currentTextChanged(QString)), ui->textedit_ssp_rx, SLOT(setTextCodec(QString)));
}

bool SessionSerialPort::isSerialPortExist(QString portName)
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if(info.portName() == portName)
            return true;
    }
    return false;
}

void SessionSerialPort::saveBinaryData(QString fileName)
{
    std::ofstream out;
    out.open(fileName.toStdString(), std::ios::out | std::ios::binary);
    if(!out.is_open())
        return;

    QByteArray binary = ui->textedit_ssp_rx->getAllByteArray();
    out.write(binary.data(), binary.size());
}

void SessionSerialPort::saveHexData(QString fileName)
{
    std::ofstream out;
    out.open(fileName.toStdString(), std::ios::out);
    if(!out.is_open())
        return;

    QByteArray array = ui->textedit_ssp_rx->getAllByteArray();
    for(int i=0; i < array.size() / 16; i++)
    {
        for(int j=0; j<16; j++)
        {
            out << QString("%1").arg((unsigned char)array[i*16+j], 2, 16, QLatin1Char('0')).toStdString();
            if(j == 15)
                out << std::endl;
            else
                out << " ";
        }
    }

    int startIndex = (array.size() / 16) * 16;
    int endIndex = array.size();
    for(int i=startIndex; i<endIndex; i++)
    {
        out << QString("%1").arg((unsigned char)array[i], 2, 16, QLatin1Char('0')).toStdString();
        if(i != endIndex - 1)
            out << " ";
    }
}

void SessionSerialPort::saveHexWithTextData(QString fileName)
{
    saveTextToFile(ui->textedit_ssp_rx->getHexWithTextData(), fileName);
}

void SessionSerialPort::saveHexWithTextAndTimeData(QString fileName)
{
    saveTextToFile(ui->textedit_ssp_rx->getHexWithTextAndTimeData(), fileName);
}

void SessionSerialPort::saveTextData(QString fileName)
{
    saveTextToFile(ui->textedit_ssp_rx->getTextData(), fileName);
}

void SessionSerialPort::saveTextWithTimeData(QString fileName)
{
    saveTextToFile(ui->textedit_ssp_rx->getTextWithTimeData(), fileName);
}

void SessionSerialPort::saveTextToFile(QString text, QString fileName)
{
#ifdef Q_OS_WINDOWS
    text.replace("\r\n", "\r");
#endif

#if 1
    QFile outFile(fileName);
    if(!outFile.open(QIODevice::WriteOnly|QIODevice::Text))
        return;

    QTextStream out(&outFile);
    out << text;
#else
    std::ofstream out;
    out.open(fileName.toStdString(), std::ios::out);
    if(!out.is_open())
        return;

    out << text.toStdString();
#endif
}

void SessionSerialPort::sendHexData()
{
    QString text = ui->textedit_ssp_tx->text();

    QString hexText;
    for(int i=0; i<text.size(); i++)
    {
        QChar qc = text.at(i);
        if(!qc.isLetterOrNumber()) // 可以排除中文等
            continue;

        unsigned char c = qc.toLower().toLatin1();
        if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
        {
            hexText.append(c);
        }
    }

    QByteArray sendData;
    for(int i=0; i<hexText.size()/2; i++)
    {
        bool ok;
        unsigned char c = hexText.midRef(i*2, 2).toUInt(&ok, 16);
        sendData.append(c);
    }

    if((hexText.size() % 2) == 1)
    {
        bool ok;
        unsigned char c = hexText.rightRef(1).toUInt(&ok, 16);
        sendData.append(c);
    }

    mSendCnt += mSerialPort.write(sendData);
    updateCntLabel();
}

void SessionSerialPort::sendTextData()
{
    QString text = ui->textedit_ssp_tx->text();
    QByteArray sendData;
    if(ui->comboBox_txCodec->currentText() == "UTF-8")
    {
        sendData = text.toUtf8();
    }
    else
    {
        sendData = mCodecGB->fromUnicode(text);
    }
    mSendCnt += mSerialPort.write(sendData);
    updateCntLabel();
}

void SessionSerialPort::updateCntLabel()
{
    ui->label_rx_cnt->setText("Rx: " + QString::number(ui->textedit_ssp_rx->rxDataSize()));
    ui->label_tx_cnt->setText("Tx: " + QString::number(mSendCnt));
}
