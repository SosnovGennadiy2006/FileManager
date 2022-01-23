#include <QCoreApplication>
#include <filemanager.h>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setlocale(0, "");

    FileManager manager;

    manager.addPath("D:\\", "Microsoft*.dll");

    return a.exec();
}
