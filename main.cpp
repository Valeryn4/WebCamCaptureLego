#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();
    }
    catch (QException &e)
    {
        qDebug() << e.what();
    }
    catch (std::exception &e)
    {
        qDebug() << e.what();
    }

    return -1;
}
