#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // поддержка масштабирования High-DPI
    QApplication::setStyle(QStyleFactory::create("Fusion")); // установка стиля GUI

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
