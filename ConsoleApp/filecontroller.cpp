#include "filecontroller.h"

FileController::FileController(const QString& dest, operationTypes _type)
{
    operation = _type;
    destination = dest;
    files = {};
}

void FileController::addFile(const QString& file)
{
    files.push_back(file);
}

void FileController::addFiles(const QStringList &files)
{
    for (qsizetype i = 0; i < files.size(); i++)
    {
        this->files.push_back(files[i]);
    }
}

void FileController::deleteFiles(const QStringList& files)
{

    for (qsizetype i = 0; i < files.size(); i++)
    {
        QFile f(files[i]);
        f.open(QIODevice::WriteOnly);
        f.remove();
    }
}

void FileController::copy()
{
    QDir dir(destination);
    QFileInfo info;
    for (qsizetype i = 0; i < files.size(); i++)
    {
        info.setFile(files[i]);
        QFile::copy(files[i], dir.filePath(info.fileName()));
    }
}

void FileController::process()
{
    switch (operation)
    {
        case operationTypes::operation_copy:
        {
            copy();
            break;
        }
        case operationTypes::operation_delete:
        {
            deleteFiles(files);
            break;
        }
        default:
        {
            emit finished();
            return;
        }
    }

    emit finished();
    return;
}
