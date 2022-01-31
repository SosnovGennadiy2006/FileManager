#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "FileManager_global.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIODevice>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QDir>
#include <QThread>
#include <QDirIterator>
#include <QDesktopServices>
#include <iostream>
#include <libs/json.hpp>
#include <filecontroller.h>

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
    QString copyFolderPath;
    QFileSystemWatcher watcherDirectory;
    QFileSystemWatcher watcherFile;

    void addPathToWatcher();

    void addThread(const QStringList& files, operationTypes type);

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
    resultCode addFiles(const QString& dir_name, const QStringList& files);
    resultCode addFiles(int dir_pos, const QStringList& files);
    resultCode addFile(const QString& dir_name, const QString& file_path);
    resultCode addFile(int dir_pos, const QString& file_path);
    resultCode deletePath(const QString& path);
    resultCode deletePath(int pos);
    resultCode deleteFileSafety(int pos_dir, int pos_file);
    resultCode deleteFileForever(int pos_dir, int pos_file);
    resultCode deleteSomeFiles(const QString& dir_name, const QStringList& files);
    resultCode deleteSomeFiles(int pos_dir, const QStringList& files);
    resultCode makeCommand(int argc, char *argv[]);
    resultCode makeCommand(int argc, const QStringList& argv);

    QString getDirectory(int pos_dir);
    QString getFile(int pos_dir, int pos_file);
    QStringList getPaths();
    QStringList getFiles(const QString& dir);
    QStringList getFiles(int pos_dir);
    QStringList getFilters(const QString& dir);
    QStringList getFilters(int pos_dir);

    resultCode setFiles(const QString& dir, const QStringList& files);
    resultCode setFiles(int pos_dir, const QStringList& files);

    QStringList parseFilters(const QString& filters);

    void printPaths();
    resultCode printFiles(const QString& dir);

    void deleteSomeCopiedFiles(const QStringList& files);

public slots:
    void ifDirectoryChanged(const QString& path);
    void ifFileChanged(const QString& path);

};

#endif // FILEMANAGER_H
