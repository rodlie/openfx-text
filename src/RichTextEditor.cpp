
#include <QApplication>
#include "RichTextWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RichTextWidget w;
    w.show();
    return a.exec();
}
