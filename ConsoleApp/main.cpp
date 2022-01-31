#include <QCoreApplication>
#include "filemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileManager manager;

    resultCode code = manager.makeCommand(argc, argv);

    std::cout << manager.codeInfo(code).toStdString() << std::endl;

    return a.exec();
}
