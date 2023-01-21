#ifndef RXTEXTEDIT_H
#define RXTEXTEDIT_H

#include "TextEdit.h"
#include <QTextCodec>
#include <string>

#define HEX_Number_Per_Line     16

class RxTextEdit : public TextEdit
{
    Q_OBJECT
public:
    RxTextEdit(QWidget *parent = nullptr);

    int rxDataSize(){return mAllByteArray.size();}

    void clearAll();
    void appendByteArray(QByteArray byteArray);

    QByteArray getAllByteArray(){return mAllByteArray;}
    QString getHexWithTextData()
    {
        QString text = mHexCompleteContent + mHexLastSecondLine + mHexLastLine;
        return text;
    }
    QString getHexWithTextAndTimeData()
    {
        QString text = mHexTimeCompleteContent + mHexTimeLastSecondLine + mHexTimeLastLine;
        return text;
    }
    QString getTextData(){return mText;}
    QString getTextWithTimeData(){return mTextTime;}

public slots:
    void setShowHex(bool enabled);
    void setShowTime(bool enabled);
    void setTextCodec(QString codec){mCodec = codec;}

protected:
    void appendHexData(QByteArray byteArray);
    void appendTextData(QByteArray byteArray);
    void updateAfterConfigChanged();

    QString byteArrayToHexLine(QByteArray array, QByteArray nextArray = QByteArray());
    QString byteArrayToHexAppendText_UTF8(QByteArray array, QByteArray nextArray);
    QString byteArrayToHexAppendText_GB18030(QByteArray array, QByteArray nextArray);

    void removeLastLines(int removeLines);

    void appendTextData_UTF8(QByteArray byteArray);
    void appendTextData_GB18030(QByteArray byteArray);

private:
    QByteArray mAllByteArray;
    bool mEnableShowHex;
    bool mEnableShowTime;

    qint64 mHexLastLineTime;
    qint64 mHexLastSecondLineTime;

    QString mHexCompleteContent;
    QString mHexLastLine;
    QString mHexLastSecondLine;

    QString mHexTimeCompleteContent;
    QString mHexTimeLastLine;
    QString mHexTimeLastSecondLine;

    QByteArray mHexLastLineByteArray;
    QByteArray mHexLastSecondLineByteArray;

    QString mCodec;
    QTextCodec *mCodecUTF8;
    QTextCodec *mCodecGB18030;

    QString mText;
    QString mTextTime;
    QByteArray mTextRemainArray;
};

#endif // RXTEXTEDIT_H
