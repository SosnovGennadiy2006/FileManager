#include <QCoreApplication>
#include <filemanager.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setlocale(0, "");

    FileManager manager = FileManager();

    manager.addPath("D:/lib", "a*.txt");

    return a.exec();
}
