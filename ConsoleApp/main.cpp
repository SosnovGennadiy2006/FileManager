#include <QCoreApplication>
#include <filemanager.h>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileManager manager;

    std::cout << manager.addPath("C:/");

    return a.exec();
}
