#ifndef RICH_TEXT_WIDGET_H
#define RICH_TEXT_WIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFontComboBox>
#include <QSlider>

class RichTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RichTextWidget(QWidget *parent = nullptr);
    ~RichTextWidget();

signals:
    void textChanged();

public slots:
    void setText(const QString &text);
    const QString getText();
    void setTextAlign(int align);
    int getTextAlign();
    void setTextRotate(int value);
    int getTextRotate();

private:
    QTextEdit *htmlEditor;
    QPlainTextEdit *textEditor;
    QPushButton *textBoldButton;
    QPushButton *textItalicButton;
    QPushButton *textUnderlineButton;
    QPushButton *textColorButton;
    QFontComboBox *fontBox;
    QComboBox *fontSizeBox;
    QComboBox *textAlignBox;
    QSlider *textRotateSlider;

    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

private slots:
    void setup();
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void handleBoldButton(bool triggered);
    void handleItalicButton(bool triggered);
    void handleUnderLineButton(bool triggered);
    void handleTextFamily(const QString &f);
    void handleTextSize(const QString &p);
    void handleTextAlign(int index);
    void handleTextRotate(int value);
    void handleTextColor();
    void handleTextChanged();
};

#endif // RICH_TEXT_WIDGET_H
