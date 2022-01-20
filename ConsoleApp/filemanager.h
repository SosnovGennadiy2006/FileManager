#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIODevice>
#include <iostream>
#include <vector>
#include <libs/json.hpp>

enum resultCode {
    OK = 0,
    errorPath,
    employedPath,
    notFolder,
    notDir
};

class FileManager
{
private:
    nlohmann::json database;

public:
    FileManager();

    bool openDB();
    bool saveDB();

    bool isExists(const QString& path);
    bool isEmployed(const QString& path);
    bool isFile(const QString& path);
    bool isDir(const QString& path);

    resultCode addPath(const QString& path);

    std::vector<QString> getPaths();
};

#endif // FILEMANAGER_H
