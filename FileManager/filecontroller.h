#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <iostream>

enum operationTypes {
    operation_copy,
    operation_delete
};

class FileController : public QObject
{
    Q_OBJECT

private:
    operationTypes operation;
    QStringList files;
    QString destination;

public:
    FileController(const QString& dest, operationTypes _type);

    void addFile(const QString& file);
    void addFiles(const QStringList& files);

    void deleteFiles(const QStringList& files);
    void copy();

public slots:
    void process();

signals:
    void finished();

};

#endif // FILECONTROLLER_H
