#include "RxTextEdit.h"
#include <QDateTime>
#include <QDebug>

#define Time_Pattern        "[hh:mm:ss.zzz] "
#define Unknow_Char         "?"

RxTextEdit::RxTextEdit(QWidget *parent) :
    TextEdit(parent),
    mHexLastLineTime(0),
    mHexLastSecondLineTime(0),
    mEnableShowHex(true),
    mEnableShowTime(true),
    mCodec("UTF-8")
{
    mCodecUTF8 = QTextCodec::codecForName("UTF-8");
    mCodecGB18030 = QTextCodec::codecForName("GB-18030");
}

void RxTextEdit::clearAll()
{
    mAllByteArray.clear();
    QsciScintilla::clear();

    /// Hex
    mHexLastLineTime = 0;
    mHexLastSecondLineTime = 0;

    mHexCompleteContent.clear();
    mHexLastLine.clear();
    mHexLastSecondLine.clear();

    mHexTimeCompleteContent.clear();
    mHexTimeLastLine.clear();
    mHexTimeLastSecondLine.clear();

    mHexLastLineByteArray.clear();
    mHexLastSecondLineByteArray.clear();

    /// Text
    mText.clear();
    mTextTime.clear();
    mTextRemainArray.clear();
}

void RxTextEdit::appendByteArray(QByteArray byteArray)
{
    mAllByteArray.append(byteArray);

    appendHexData(byteArray);
    appendTextData(byteArray);
}

void RxTextEdit::setShowHex(bool enabled)
{
    if(mEnableShowHex == enabled)
        return;

    mEnableShowHex = enabled;
    updateAfterConfigChanged();
}

void RxTextEdit::setShowTime(bool enabled)
{
    if(mEnableShowTime == enabled)
        return;

    mEnableShowTime = enabled;
    updateAfterConfigChanged();
}

void RxTextEdit::appendHexData(QByteArray byteArray)
{
    while(!byteArray.isEmpty())
    {
        int addedSize = HEX_Number_Per_Line - mHexLastLineByteArray.size();
        if(addedSize > byteArray.size())
            addedSize = byteArray.size();

        QByteArray addedByteArray = byteArray.left(addedSize);
        byteArray.remove(0, addedSize);

        mHexLastLineByteArray.append(addedByteArray);

        /// 更新最近两行显示内容
        if(!mHexLastSecondLineByteArray.isEmpty())
        {
            QString line = byteArrayToHexLine(mHexLastSecondLineByteArray, mHexLastLineByteArray);
            mHexLastSecondLine = line;
            mHexTimeLastSecondLine = QDateTime::fromMSecsSinceEpoch(mHexLastSecondLineTime).toString(Time_Pattern) + line;
        }

        if(!mHexLastLineByteArray.isEmpty())
        {
            QString line = byteArrayToHexLine(mHexLastLineByteArray);
            mHexLastLine = line;
            mHexLastLineTime = QDateTime::currentMSecsSinceEpoch();
            mHexTimeLastLine = QDateTime::fromMSecsSinceEpoch(mHexLastLineTime).toString(Time_Pattern) + line;
        }

        /// 更新显示
        if(mEnableShowHex)
        {
            /// 删除旧内容
            removeLastLines(1);

            /// 添加新的显示内容
            if(mEnableShowTime)
            {
                append(mHexTimeLastSecondLine);
                append(mHexTimeLastLine);
            }
            else
            {
                append(mHexLastSecondLine);
                append(mHexLastLine);
            }
        }

        if(mHexLastLineByteArray.size() == HEX_Number_Per_Line)
        {
            mHexLastSecondLineTime = mHexLastLineTime;

            mHexCompleteContent.append(mHexLastSecondLine);
            mHexTimeCompleteContent.append(mHexTimeLastSecondLine);

            mHexLastSecondLine = mHexLastLine;
            mHexLastLine = QString();

            mHexTimeLastSecondLine = mHexTimeLastLine;
            mHexTimeLastLine = QString();

            mHexLastSecondLineByteArray = mHexLastLineByteArray;
            mHexLastLineByteArray = QByteArray();
        }
    }
}

void RxTextEdit::appendTextData(QByteArray byteArray)
{
    if(mCodec == "UTF-8")
    {
        appendTextData_UTF8(byteArray);
    }
    else if(mCodec == "GB18030")
    {
        appendTextData_GB18030(byteArray);
    }
}

void RxTextEdit::updateAfterConfigChanged()
{
    clear();
    if(mEnableShowHex)
    {
        if(mEnableShowTime)
        {
            append(mHexTimeCompleteContent);
            append(mHexTimeLastSecondLine);
            append(mHexTimeLastLine);
        }
        else
        {
            append(mHexCompleteContent);
            append(mHexLastSecondLine);
            append(mHexLastLine);
        }
    }
    else
    {
        if(mEnableShowTime)
        {
            append(mTextTime);
        }
        else
        {
            append(mText);
        }
    }
}

QString RxTextEdit::byteArrayToHexLine(QByteArray array, QByteArray nextArray)
{
    QString line;
    for(int i=0; i<HEX_Number_Per_Line; i++)
    {
        if(i < array.size())
            line += QString("%1 ").arg((unsigned char)array[i], 2, 16, QLatin1Char('0'));
        else
            line += "   ";
    }
    line += "| ";

    if(mCodec == "UTF-8")
        line += byteArrayToHexAppendText_UTF8(array, nextArray);
    else if(mCodec == "GB18030")
        line += byteArrayToHexAppendText_GB18030(array, nextArray);

    /// 添加换行
    if(array.size() >= HEX_Number_Per_Line)
        line += "\n";

    return line;
}

/*
 *  UTF-8 编码
 *  Unicode/UCS-4   bit数    UTF-8       byte数
 *  0000~007F       0~7     0XXX XXXX   1
 *  0080~07FF       8~11    110X XXXX   2
 *                          10XX XXXX
 *  0800~FFFF       12~16   1110 XXXX   3
 *                          10XX XXXX
 *                          10XX XXXX
 *  1 0000~1F FFFF  17~21   1111 0XXX   4
 *                          10XX XXXX
 *                          10XX XXXX
 *                          10XX XXXX
 *  Unicode6.1定义范围：0~10 FFFF
 */
QString RxTextEdit::byteArrayToHexAppendText_UTF8(QByteArray array, QByteArray nextArray)
{
    QString decodedText;
    QByteArray currentDecodeArray;
    int currentDecodeArraySize = 0;
    for(int i=0; i<array.size(); )
    {
        unsigned char c = array.at(i);
        if(currentDecodeArray.isEmpty())
        {
            if(c & 0x80)
            {
                if((c >> 5) == 0b110)
                {
                    currentDecodeArray.append(c);
                    currentDecodeArraySize = 2;
                }
                else if((c >> 4) == 0b1110)
                {
                    currentDecodeArray.append(c);
                    currentDecodeArraySize = 3;
                }
                else if((c >> 3) == 0b11110)
                {
                    currentDecodeArray.append(c);
                    currentDecodeArraySize = 4;
                }
                else
                {
                    decodedText += Unknow_Char;
                }
            }
            else
            {
                if(QChar(c).isPrint())
                    decodedText += c;
                else
                {
                    if(c == '\r')
                        decodedText += "\\r";
                    else if(c == '\n')
                        decodedText += "\\n";
                    else
                        decodedText += Unknow_Char;
                }
            }
            i++;
        }
        else
        {
            if((c >> 6) == 0b10)
            {
                currentDecodeArray.append(c);
                if(currentDecodeArray.size() == currentDecodeArraySize)
                {
                    decodedText += mCodecUTF8->toUnicode(currentDecodeArray);
                    currentDecodeArray.clear();
                }
                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
    }

    ////////////////////////////////////////// next line array
    if(!currentDecodeArray.isEmpty())
    {
        int needSize = currentDecodeArraySize - currentDecodeArray.size();
        if(nextArray.size() >= needSize)
        {
            currentDecodeArray += nextArray.left(needSize);
            decodedText += mCodecUTF8->toUnicode(currentDecodeArray);
            currentDecodeArray.clear();
        }
    }

    return decodedText;
}

/*
 * GB18030标准采用单字节、双字节和四字节三种方式对字符编码。
 * 单字节部分采用GB/T 11383的编码结构与规则，使用0x00～0x7F码位(对应于ASCII码的相应码位)。
 * 双字节部分，首字节码位从0x81至0xFE，尾字节码位分别是0x40至0x7E和0x80至0xFE。
 * 四字节部分采用GB/T 11383未采用的0x30到0x39作为对双字节编码扩充的后缀，这样扩充的四字节编码
 *      ，其范围为0x81308130到0xFE39FE39。其中第一、三个字节编码码位均为0x81至0xFE，第二、四个字节编码码位均为0x30至0x39。
 */
QString RxTextEdit::byteArrayToHexAppendText_GB18030(QByteArray array, QByteArray nextArray)
{
    QString decodedText;

    QByteArray currentDecodeArray;
    for(int i=0; i<array.size(); )
    {
        unsigned char c = array.at(i);
        if(currentDecodeArray.isEmpty())
        {
            if(c >= 0x81 && c <= 0xfe)
            {
                currentDecodeArray.append(c);
            }
            else
            {
                if(QChar(c).isPrint())
                    decodedText += c;
                else
                {
                    if(c == '\r')
                        decodedText += "\\r";
                    else if(c == '\n')
                        decodedText += "\\n";
                    else
                        decodedText += Unknow_Char;
                }
            }

            i++;
        }
        else if(currentDecodeArray.size() == 1)
        {
            if(c >= 0x40 && c <= 0xfe && c != 0x7f)
            {
                currentDecodeArray.append(c);

                decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                currentDecodeArray.clear();

                i++;
            }
            else if(c >= 0x30 && c <= 0x39)
            {
                currentDecodeArray.append(c);
                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
        else if(currentDecodeArray.size() == 2)
        {
            if(c >= 0x81 && c <= 0xfe)
            {
                currentDecodeArray.append(c);
                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
        else if(currentDecodeArray.size() == 3)
        {
            if(c >= 0x30 && c <= 0x39)
            {
                currentDecodeArray.append(c);

                decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                currentDecodeArray.clear();

                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
    }

    ////////////////////////////////////////// next line array
    if(currentDecodeArray.size() == 1)
    {
        if(nextArray.size() >= 1)
        {
            unsigned char c = nextArray[0];
            if(c >= 0x40 && c <= 0xfe && c != 0x7f)
            {
                currentDecodeArray.append(c);
                decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                currentDecodeArray.clear();
            }
            else if(c >= 0x30 && c <= 0x39)
            {
                currentDecodeArray.append(c);
                nextArray.remove(0, 1);
            }
        }
    }

    if(currentDecodeArray.size() >= 2)
    {
        if(nextArray.size() >= 2)
        {
            unsigned char c0 = nextArray.at(0);
            if(c0 >= 0x81 && c0 <= 0xfe)
            {
                unsigned char c1 = nextArray.at(1);
                if(c1 >= 0x30 && c1 <= 0x39)
                {
                    currentDecodeArray.append(c0);
                    currentDecodeArray.append(c1);
                    decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                    currentDecodeArray.clear();
                }
            }
        }
    }

    return decodedText;
}

void RxTextEdit::removeLastLines(int removeLines)
{
    bool readOnly = this->isReadOnly();
    if(readOnly)
    {
        this->setReadOnly(false); /// 需要取消 ReadOnly 才能 SCI_DELETERANGE
    }

    int lineCount = SendScintilla(SCI_GETLINECOUNT);
    if(removeLines >= lineCount - 1)
    {
        removeLines = lineCount - 1;
    }

    int removeStartPos = SendScintilla(SCI_POSITIONFROMLINE, lineCount - removeLines - 1);
    int removeEndPos = SendScintilla(SCI_GETLENGTH);

#if 1
    SendScintilla(SCI_DELETERANGE, removeStartPos, removeEndPos - removeStartPos);
#else
    SendScintilla(SCI_SETTARGETSTART, removeStartPos);
    SendScintilla(SCI_SETTARGETEND, removeEndPos);
    SendScintilla(SCI_REPLACETARGET, 0, (void*) 0);
#endif

    if(readOnly)
    {
        this->setReadOnly(readOnly);
    }
}

/*
 *  UTF-8 编码
 *  Unicode/UCS-4   bit数    UTF-8       byte数
 *  0000~007F       0~7     0XXX XXXX   1
 *  0080~07FF       8~11    110X XXXX   2
 *                          10XX XXXX
 *  0800~FFFF       12~16   1110 XXXX   3
 *                          10XX XXXX
 *                          10XX XXXX
 *  1 0000~1F FFFF  17~21   1111 0XXX   4
 *                          10XX XXXX
 *                          10XX XXXX
 *                          10XX XXXX
 *  Unicode6.1定义范围：0~10 FFFF
 */
void RxTextEdit::appendTextData_UTF8(QByteArray byteArray)
{
    /*
     * QString mText;
     * QString mTextTime;
     * QByteArray mTextRemainArray;
    */

    mTextRemainArray.append(byteArray);

    int lastIndex, cut = 0;
    bool isCut = false;

    lastIndex = mTextRemainArray.length() - 1;
    if (mTextRemainArray.at(lastIndex) & 0x80) // 0xxx xxxx -> OK
    {
        // UTF8最大编码为4字节，因此向前搜寻三字节
        for (int i = lastIndex; i >= 0 && ++cut < 4; --i)
        {
            uint8_t byte = uint8_t(mTextRemainArray.at(i));
            if (((cut < 2) && (byte & 0xE0) == 0xC0) ||
                ((cut < 3) && (byte & 0xF0) == 0xE0) ||
                (byte & 0xF8) == 0xF0)
            {
                isCut = true;
                break;
            }
        }
    }
    lastIndex -= isCut ? cut - 1 : -1;
    QByteArray cutArray = mTextRemainArray.mid(lastIndex);
    mTextRemainArray.remove(lastIndex, cut);
    QString text = mCodecUTF8->toUnicode(mTextRemainArray);
    mTextRemainArray = cutArray;

    QString timeString = QDateTime::currentDateTime().toString(Time_Pattern);
    QString textTime;

    if(mTextTime.isEmpty() || mTextTime.endsWith("\n"))
    {
        textTime += timeString;
    }
    textTime += text;

    for(int i=textTime.size() - 2; i>=0; i--) // 最后一个 "\n" 不管
    {
        if(textTime.at(i) == QChar('\n'))
        {
            textTime.insert(i+1, timeString);
        }
    }

    if(!mEnableShowHex)
    {
        if(mEnableShowTime)
        {
            append(textTime);
        }
        else
        {
            append(text);
        }
    }

    mText += text;
    mTextTime += textTime;
}

/*
 * GB18030标准采用单字节、双字节和四字节三种方式对字符编码。
 * 单字节部分采用GB/T 11383的编码结构与规则，使用0x00～0x7F码位(对应于ASCII码的相应码位)。
 * 双字节部分，首字节码位从0x81至0xFE，尾字节码位分别是0x40至0x7E和0x80至0xFE。
 * 四字节部分采用GB/T 11383未采用的0x30到0x39作为对双字节编码扩充的后缀，这样扩充的四字节编码
 *      ，其范围为0x81308130到0xFE39FE39。其中第一、三个字节编码码位均为0x81至0xFE，第二、四个字节编码码位均为0x30至0x39。
 */
void RxTextEdit::appendTextData_GB18030(QByteArray byteArray)
{
    mTextRemainArray.append(byteArray);

    QString decodedText;
    QByteArray currentDecodeArray;
    for(int i=0; i<mTextRemainArray.size(); )
    {
        unsigned char c = mTextRemainArray.at(i);
        if(currentDecodeArray.isEmpty())
        {
            if(c >= 0x81 && c <= 0xfe)
            {
                currentDecodeArray.append(c);
            }
            else
            {
                decodedText += c;
            }

            i++;
        }
        else if(currentDecodeArray.size() == 1)
        {
            if(c >= 0x40 && c <= 0xfe && c != 0x7f)
            {
                currentDecodeArray.append(c);

                decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                currentDecodeArray.clear();

                i++;
            }
            else if(c >= 0x30 && c <= 0x39)
            {
                currentDecodeArray.append(c);
                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
        else if(currentDecodeArray.size() == 2)
        {
            if(c >= 0x81 && c <= 0xfe)
            {
                currentDecodeArray.append(c);
                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
        else if(currentDecodeArray.size() == 3)
        {
            if(c >= 0x30 && c <= 0x39)
            {
                currentDecodeArray.append(c);

                decodedText += mCodecGB18030->toUnicode(currentDecodeArray);
                currentDecodeArray.clear();

                i++;
            }
            else
            {
                // 出错的情况下不对 i 进行++，对当前 c 重新开始进行解码
                for(int n=0; n<currentDecodeArray.size(); n++)
                    decodedText += Unknow_Char;
                currentDecodeArray.clear();
            }
        }
    }

    mTextRemainArray = currentDecodeArray;

    QString timeString = QDateTime::currentDateTime().toString(Time_Pattern);
    QString textTime;

    if(mTextTime.isEmpty() || mTextTime.endsWith("\n"))
    {
        textTime += timeString;
    }
    textTime += decodedText;

    for(int i=textTime.size() - 2; i>=0; i--) // 最后一个 "\n" 不管
    {
        if(textTime.at(i) == QChar('\n'))
        {
            textTime.insert(i+1, timeString);
        }
    }

    if(!mEnableShowHex)
    {
        if(mEnableShowTime)
        {
            append(textTime);
        }
        else
        {
            append(decodedText);
        }
    }

    mText += decodedText;
    mTextTime += textTime;
}
