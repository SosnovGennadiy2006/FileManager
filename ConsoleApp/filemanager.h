#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIODevice>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QDir>
#include <QEventLoop>
#include <QDirIterator>
#include <QDesktopServices>
#include <iostream>
#include <libs/json.hpp>

enum resultCode {
    OK = 0,
    connectionError,
    errorPath,
    employedPath,
    notFolder,
    notFile,
    incorrectPath,
    incorrectPathId
};

class FileManager : public QObject
{
    Q_OBJECT

private:
    nlohmann::json database;
    QString database_path;
    QFileSystemWatcher watcherDirectory;
    QFileSystemWatcher watcherFile;

    void addPathToWatcher();

public:
    FileManager();

    bool isDBOpen();
    bool openDB();
    bool saveDB();
    QString codeInfo(resultCode code);

    bool isExists(const QString& path);
    bool isEmployed(const QString& path);
    bool isFile(const QString& path);
    bool isDir(const QString& path);
    resultCode checkPath(const QString& directory);
    int findDirectory(const QString& directory);
    void checkDirectories();
    resultCode checkFiles(const QString& dir);
    resultCode checkFiles(int pos_dir);

    resultCode addPath(const QString& path, const QString& pattern = "all");
    resultCode deletePath(const QString& path);
    resultCode deletePath(int pos);
    resultCode deleteFileSafety(int pos_dir, int pos_file);
    resultCode deleteFileForever(int pos_dir, int pos_file);
    resultCode makeCommand(int argc, char *argv[]);

    QString getDirectory(int pos_dir);
    QString getFile(int pos_dir, int pos_file);
    QStringList getPaths();
    QStringList getFiles(const QString& dir);
    QStringList getFiles(int pos_dir);

    void printPaths();
    resultCode printFiles(const QString& dir);

public slots:
    void ifDirectoryChanged(const QString& path);
    void ifFileChanged(const QString& path);

};

#endif // FILEMANAGER_H
