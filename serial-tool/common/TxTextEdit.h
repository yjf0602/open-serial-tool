#ifndef TXTEXTEDIT_H
#define TXTEXTEDIT_H

#include "TextEdit.h"
#include <QObject>

class TxTextEdit : public TextEdit
{
    Q_OBJECT
public:
    TxTextEdit(QWidget *parent = nullptr);
};

#endif // TXTEXTEDIT_H
