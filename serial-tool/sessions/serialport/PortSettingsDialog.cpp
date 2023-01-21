#include "PortSettingsDialog.h"
#include "ui_PortSettingsDialog.h"

PortSettingsDialog::PortSettingsDialog(SessionSerialPortConfig config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PortSettingsDialog)
{
    ui->setupUi(this);

    switch ((QSerialPort::DataBits)config.mConfigPortDataBits)
    {
    case QSerialPort::Data5:
        ui->comboBox_dataBits->setCurrentIndex(0);
        break;
    case QSerialPort::Data6:
        ui->comboBox_dataBits->setCurrentIndex(1);
        break;
    case QSerialPort::Data7:
        ui->comboBox_dataBits->setCurrentIndex(2);
        break;
    default:
        ui->comboBox_dataBits->setCurrentIndex(3);
        break;
    }

    switch((QSerialPort::Parity)config.mConfigPortParity)
    {
    case QSerialPort::EvenParity:
        ui->comboBox_parity->setCurrentIndex(1);
        break;
    case QSerialPort::OddParity:
        ui->comboBox_parity->setCurrentIndex(2);
        break;
    case QSerialPort::SpaceParity:
        ui->comboBox_parity->setCurrentIndex(3);
        break;
    case QSerialPort::MarkParity:
        ui->comboBox_parity->setCurrentIndex(4);
        break;
    default:
        ui->comboBox_parity->setCurrentIndex(0);
        break;
    }

    switch((QSerialPort::StopBits)config.mConfigPortStopBits)
    {
    case QSerialPort::OneAndHalfStop:
        ui->comboBox_stopBits->setCurrentIndex(1);
        break;
    case QSerialPort::TwoStop:
        ui->comboBox_stopBits->setCurrentIndex(2);
        break;
    default:
        ui->comboBox_stopBits->setCurrentIndex(0);
        break;
    }

    switch ((QSerialPort::FlowControl)config.mConfigPortFlowControl)
    {
    case QSerialPort::HardwareControl:
        ui->comboBox_flowControl->setCurrentIndex(1);
        break;
    case QSerialPort::SoftwareControl:
        ui->comboBox_flowControl->setCurrentIndex(2);
        break;
    default:
        ui->comboBox_flowControl->setCurrentIndex(0);
        break;
    }

    connect(ui->pushButton_ok, &QPushButton::clicked, this, [=](){
        int dataBits = QSerialPort::Data8;
        if(ui->comboBox_dataBits->currentIndex() == 0)
            dataBits = QSerialPort::Data5;
        else if(ui->comboBox_dataBits->currentIndex() == 1)
            dataBits = QSerialPort::Data6;
        else if(ui->comboBox_dataBits->currentIndex() == 2)
            dataBits = QSerialPort::Data7;

        int parity = QSerialPort::NoParity;
        if(ui->comboBox_parity->currentIndex() == 1)
            parity = QSerialPort::EvenParity;
        else if(ui->comboBox_parity->currentIndex() == 2)
            parity = QSerialPort::OddParity;
        else if(ui->comboBox_parity->currentIndex() == 3)
            parity = QSerialPort::SpaceParity;
        else if(ui->comboBox_parity->currentIndex() == 4)
            parity = QSerialPort::MarkParity;

        int stopBits = QSerialPort::OneStop;
        if(ui->comboBox_stopBits->currentIndex() == 1)
            stopBits = QSerialPort::OneAndHalfStop;
        else if(ui->comboBox_stopBits->currentIndex() == 2)
            stopBits = QSerialPort::TwoStop;

        int flowControl = QSerialPort::NoFlowControl;
        if(ui->comboBox_flowControl->currentIndex() == 1)
            flowControl = QSerialPort::HardwareControl;
        else if(ui->comboBox_flowControl->currentIndex() == 2)
            flowControl = QSerialPort::SoftwareControl;

        emit signalSettings(dataBits, parity, stopBits, flowControl);
        this->close();
    });
}

PortSettingsDialog::~PortSettingsDialog()
{
    delete ui;
}
