#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <Qsci/qsciscintilla.h>

class TextEdit : public QsciScintilla
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = nullptr);

    void append(const QString &text);
    void setFonts(QString fonts, int size, QColor color = Qt::black, QString style = "");
    void setHighLight(const QString &language);
    void setIndentationsUseTabs(bool enable);
    void setTabWidth(int width);
    void setAutoIndent(bool enable);
    void setIndentationGuides(bool enable);

public slots:
    void setWrap(bool wrap);

private:
    void setMarginsWidth();
    void highlightNone();
    void highlightCpp();
    void highlightBash();
    void highlightLua();
    void highlightJSON();

private slots:
    void onTextChanged();
    void onLinesChanged();
    void onVScrollBarRangeChanged();
    void onVScrollBarValueChanged();

private:
    QString mFontFamily, mLanguage;
    QFont mLineNumFont;
    int mFontSize;
    int mLineCount = 1;
    bool mIsWrap = false;
    bool mScrollEnd = true;
};

#endif // TEXTEDIT_H
