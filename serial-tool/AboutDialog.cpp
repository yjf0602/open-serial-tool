#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->label_version->setText(QString("Serial Tool V") + VERSION);

    ui->label_url->setOpenExternalLinks(true);
    ui->label_url->setText("<a style='color: blue; text-decoration: none' href=https://gitee.com/yjf0602/open-serial-tool>https://gitee.com/yjf0602/open-serial-tool");

    connect(ui->pushButton, &QPushButton::clicked, this, [=]{
        this->close();
    });
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
