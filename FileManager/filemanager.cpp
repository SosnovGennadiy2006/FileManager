#include "filemanager.h"

//Public Methods:

FileManager::FileManager()
{
    database_path = "../database.json";
    copyFolderPath = "../CopiedFiles";
    openDB();

    if (isDBOpen())
    {
        checkDirectories();

        addPathToWatcher();

        QObject::connect(&watcherDirectory, &QFileSystemWatcher::directoryChanged, \
                         [this](const QString& path)
        {
            this->ifDirectoryChanged(path);
        });

        QObject::connect(&watcherFile, &QFileSystemWatcher::fileChanged, \
                         [this](const QString& path)
        {
            this->ifFileChanged(path);
        });
    }
}

bool FileManager::isDBOpen()
{
    return database != nullptr;
}

bool FileManager::openDB()
{
    QFile file(database_path);

    if (file.exists())
    {
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);

            database = nlohmann::json::parse(stream.readAll().toStdString());

            file.close();

            return true;
        }
    }
    return false;
}

bool FileManager::saveDB()
{
    QFile file(database_path);

    if (file.exists())
    {
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            QTextStream stream(&file);

            stream << QString::fromStdString(database.dump(4));

            file.close();

            return true;
        }
    }
    return false;
}

QString FileManager::codeInfo(resultCode code)
{
    if (code == resultCode::connectionError)
    {
        return "Error to connect to datebase!";
    }else if (code == resultCode::employedPath)
    {
        return "This path has already been used!";
    }else if (code == resultCode::notFolder)
    {
        return "This path does not lead to a folder, but to a file!";
    }else if (code == resultCode::errorPath)
    {
        return "Error, path doesn't exist!";
    }else if (code == resultCode::incorrectPath)
    {
        return "Error, incorrect path!";
    }
    return "";
}

bool FileManager::isExists(const QString &path)
{
    QFileInfo f(path);
    return f.exists();
}

bool FileManager::isEmployed(const QString &path)
{
    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        if (database["paths"][i]["dir"] == path.toStdString())
        {
            return true;
        }
    }
    return false;
}

bool FileManager::isFile(const QString &path)
{
    QFileInfo f(path);
    return f.isFile();
}

bool FileManager::isDir(const QString &path)
{
    QFileInfo f(path);
    return f.isDir();
}

resultCode FileManager::checkPath(const QString &directory)
{
    if (isExists(directory))
    {
        QFileInfo p(directory);
        if (!isEmployed(p.absoluteFilePath()))
        {
            return resultCode::OK;
        }
        return resultCode::employedPath;
    }
    return resultCode::errorPath;
}

int FileManager::findDirectory(const QString &directory)
{
    int pos = -1;

    for (int i = 0; i < static_cast<int>(database["paths"].size()); i++)
    {
        if (QString::compare(directory, QString::fromStdString(database["paths"][i]["dir"].get<std::string>())) == 0)
        {
            pos = i;
        }
    }

    return pos;
}

void FileManager::checkDirectories()
{
    QList<int> positions;
    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        if (!isExists(QString::fromStdString(database["paths"][i]["dir"])))
        {
            positions.push_back(i);
        }
    }

    for (int i = positions.size() - 1; i >= 0; i--)
    {
        deletePath(i);
    }

    for (int i = 0; i < static_cast<int>(database["paths"].size()); i++)
    {
        checkFiles(i);
    }

    saveDB();
}

resultCode FileManager::checkFiles(const QString &dir)
{
    resultCode code = checkPath(dir);

    if (code == resultCode::employedPath)
    {
        int dir_id = findDirectory(dir);

        QList<int> positions;

        QStringList files = getFiles(dir);

        for (qsizetype i = 0; i < files.size(); i++)
        {
            if (!isExists(files[i]))
            {
                positions.push_back(i);
            }
        }

        for (int i = positions.size() - 1; i >= 0; i++)
        {
            deleteFileForever(dir_id, positions[i]);
        }

        return resultCode::OK;
    }

    return code;
}

resultCode FileManager::checkFiles(int pos_dir)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {

        QList<int> positions;

        QStringList files = getFiles(pos_dir);

        for (qsizetype i = 0; i < files.size(); i++)
        {
            if (!isExists(files[i]))
            {
                positions.push_back(i);
            }
        }

        for (int i = positions.size() - 1; i >= 0; i--)
        {
            deleteFileForever(pos_dir, positions[i]);
        }

        return resultCode::OK;
    }

    return resultCode::incorrectPathId;
}

resultCode FileManager::addPath(const QString &path, const QString& pattern)
{
    resultCode code = checkPath(path);

    if (isFile(path))
        return resultCode::notFolder;

    if (code == resultCode::OK)
    {
        QStringList filters = parseFilters(pattern);

        QFileInfo p(path);
        QDir dir(p.absoluteFilePath());

        nlohmann::json j;
        j["dir"] = p.absoluteFilePath().toStdString();
        j["pattern"] = pattern.toStdString();
        j["files"] = "[]"_json;
        j["deleted_files"] = "[]"_json;

        QDirIterator it(p.absoluteFilePath(), filters, QDir::Files);

        watcherDirectory.addPath(p.absoluteFilePath());

        while (it.hasNext())
        {
            QString path = it.next();

            watcherFile.addPath(path);
            j["files"].push_back(path.toStdString());
        }

        database["paths"].push_back(j);

        saveDB();
    }

    return code;
}

resultCode FileManager::addFiles(const QString &dir_name, const QStringList &files)
{
    resultCode code = checkPath(dir_name);

    if (code == resultCode::employedPath)
    {
        for (qsizetype i = 0; i < files.size(); i++)
        {
            code = checkPath(files[i]);
            if (code != resultCode::OK)
            {
                return code;
            }
        }

        int dir_pos = findDirectory(dir_name);

        for (qsizetype i = 0; i < files.size(); i++)
        {
            database["paths"][dir_pos]["files"].push_back(files[i].toStdString());
        }

        saveDB();

        return resultCode::OK;
    }

    if (code == resultCode::OK)
        return resultCode::incorrectPath;
    return code;
}

resultCode FileManager::addFiles(int dir_pos, const QStringList &files)
{
    if (dir_pos <= static_cast<int>(database["paths"].size()) || dir_pos >= 0)
    {
        resultCode code;
        for (qsizetype i = 0; i < files.size(); i++)
        {
            code = checkPath(files[i]);
            if (code != resultCode::OK)
                return code;
            if (isDir(files[i]))
                return resultCode::notFile;
        }

        for (qsizetype i = 0; i < files.size(); i++)
        {
            database["paths"][dir_pos]["files"].push_back(files[i].toStdString());
        }

        saveDB();

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::addFile(const QString &dir_name, const QString &file_path)
{
    resultCode code = checkPath(dir_name);

    if (isFile(dir_name))
        return resultCode::notFolder;

    if (code == resultCode::employedPath)
    {
        code = checkPath(file_path);

        if (code != resultCode::OK)
            return code;

        int dir_pos = findDirectory(dir_name);

        database["paths"][dir_pos]["files"].push_back(dir_name.toStdString());

        saveDB();

        return resultCode::OK;
    }

    if (code == resultCode::OK)
        return resultCode::incorrectPath;
    return code;
}

resultCode FileManager::addFile(int dir_pos, const QString &file_path)
{

    if (dir_pos <= static_cast<int>(database["paths"].size()) || dir_pos >= 0)
    {
        resultCode code;
        code = checkPath(file_path);
        if (code != resultCode::OK)
            return code;

        database["paths"][dir_pos]["files"].push_back(file_path.toStdString());

        saveDB();

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::deletePath(const QString &path)
{
    resultCode code = checkPath(path);

    if (isFile(path))
        return resultCode::notFolder;

    if (code == resultCode::employedPath)
    {
        int pos = findDirectory(path);

        if (pos != -1)
        {
            database["paths"].erase(pos);

            saveDB();

            return resultCode::OK;
        }

        return resultCode::incorrectPath;
    }

    if (code == resultCode::OK)
        return resultCode::incorrectPath;
    return code;
}

resultCode FileManager::deletePath(int pos)
{
    if (pos < static_cast<int>(database["paths"].size()) || pos >= 0)
    {
        database["paths"].erase(pos);

        saveDB();

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::deleteFileSafety(int pos_dir, int pos_file)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        auto files = getFiles(pos_dir);
        if (pos_file < files.size() || pos_file > 0)
        {
            database["paths"][pos_dir]["deleted_files"].push_back(database["paths"][pos_dir]["files"][pos_file]);
            database["paths"][pos_dir]["files"].erase(pos_file);

            return resultCode::OK;
        }
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::deleteFileForever(int pos_dir, int pos_file)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        auto files = getFiles(pos_dir);
        if (pos_file < files.size() || pos_file > 0)
        {
            database["paths"][pos_dir]["files"].erase(pos_file);

            return resultCode::OK;
        }
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::deleteSomeFiles(const QString &dir_name, const QStringList &files)
{

    resultCode code = checkPath(dir_name);

    if (isFile(dir_name))
        return resultCode::notFolder;

    if (code == resultCode::employedPath)
    {
        int pos = findDirectory(dir_name);

        if (pos != -1)
        {
            QList<int> positions;

            for (qsizetype i = 0; i < static_cast<qsizetype>(database["paths"][pos]["files"].size()); i++)
            {
                if (files.contains(QString::fromStdString(database["paths"][pos]["files"][i].get<std::string>())))
                {
                    positions.push_back(i);
                }
            }

            for (int i = positions.size() - 1; i > -1; i--)
            {
                database["paths"][pos]["files"].erase(positions[i]);
            }

            saveDB();

            return resultCode::OK;
        }

        return resultCode::incorrectPath;
    }

    if (code == resultCode::OK)
        return resultCode::incorrectPath;
    return code;
}

resultCode FileManager::deleteSomeFiles(int pos_dir, const QStringList &files)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        QList<int> positions;

        for (qsizetype i = 0; i < static_cast<qsizetype>(database["paths"][pos_dir]["files"].size()); i++)
        {
            if (files.contains(QString::fromStdString(database["paths"][pos_dir]["files"][i].get<std::string>())))
            {
                positions.push_back(i);
            }
        }

        for (int i = positions.size() - 1; i > -1; i--)
        {
            database["paths"][pos_dir]["files"].erase(positions[i]);
        }

        saveDB();

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::makeCommand(int argc, char **argv)
{
    if (isDBOpen())
    {
        if (argc == 4)
        {
            if (QString::compare(argv[1], "add") == 0)
            {
                resultCode code = addPath(argv[2], argv[3]);
                return code;
            }
        }else if (argc == 3)
        {
            if (QString::compare(argv[1], "add") == 0)
            {
                resultCode code = addPath(argv[2]);
                return code;
            } else if (QString::compare(argv[1], "delete") == 0)
            {
                resultCode code = deletePath(argv[2] - 1);
                return code;
            }
        } else if (argc == 2)
        {
            if (QString::compare(argv[1], "printPaths") == 0)
            {
                std::cout << "There are " << database["paths"].size() \
                          << " paths:" << std::endl;
                printPaths();
                return resultCode::OK;
            }
        }
        return resultCode::OK;
    }else
    {
        return resultCode::connectionError;
    }
}

resultCode FileManager::makeCommand(int argc, const QStringList& argv)
{
    if (isDBOpen())
    {
        if (argc == 4)
        {
            if (QString::compare(argv[1], "add") == 0)
            {
                resultCode code = addPath(argv[2], argv[3]);
                return code;
            }
        }else if (argc == 3)
        {
            if (QString::compare(argv[1], "add") == 0)
            {
                resultCode code = addPath(argv[2]);
                return code;
            } else if (QString::compare(argv[1], "delete") == 0)
            {
                resultCode code = deletePath(argv[2].toInt() - 1);
                return code;
            }
        } else if (argc == 2)
        {
            if (QString::compare(argv[1], "printPaths") == 0)
            {
                std::cout << "There are " << database["paths"].size() \
                          << " paths:" << std::endl;
                printPaths();
                return resultCode::OK;
            }
        }
        return resultCode::OK;
    }else
    {
        return resultCode::connectionError;
    }
}

QString FileManager::getDirectory(int pos_dir)
{
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        return QString::fromStdString(database["paths"][pos_dir].get<std::string>());
    }
    return "";
}

QString FileManager::getFile(int pos_dir, int pos_file)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        auto files = getFiles(pos_dir);
        if (pos_file < files.size() || pos_file > 0)
        {
            return QString::fromStdString(database["paths"][pos_dir]["files"][pos_file].get<std::string>());
        }
    }
    return "";
}

QStringList FileManager::getPaths()
{
    QStringList paths;

    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        paths.push_back(QString::fromStdString(database["paths"][i]["dir"].get<std::string>()));
    }

    return paths;
}

QStringList FileManager::getFiles(const QString &dir)
{
    resultCode code = checkPath(dir);
    QStringList files;

    if (code == resultCode::employedPath && isDir(dir))
    {
        int pos = findDirectory(dir);
        for (size_t i = 0; i < database["paths"][pos]["files"].size(); i++)
        {
            files.push_back(QString::fromStdString(database["paths"][pos]["files"][i].get<std::string>()));
        }
    }
    return files;
}

QStringList FileManager::getFiles(int pos_dir)
{
    QStringList files;
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        return getFiles(QString::fromStdString(database["paths"][pos_dir]["dir"].get<std::string>()));
    }
    return files;
}

QStringList FileManager::getFilters(const QString &dir)
{
    resultCode code = checkPath(dir);
    QStringList filters;

    if (code == resultCode::employedPath && isDir(dir))
    {
        int pos = findDirectory(dir);
        filters = parseFilters(QString::fromStdString(database["paths"][pos]["pattern"].get<std::string>()));
    }

    return filters;
}

QStringList FileManager::getFilters(int pos_dir)
{
    QStringList files;
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        return parseFilters(QString::fromStdString(database["paths"][pos_dir]["pattern"].get<std::string>()));
    }
    return files;
}

resultCode FileManager::setFiles(const QString &dir, const QStringList &files)
{
    resultCode code = checkPath(dir);

    if (isFile(dir))
        return resultCode::notFolder;

    if (code == resultCode::employedPath)
    {
        int pos = findDirectory(dir);

        database["paths"][pos]["files"] = "[]"_json;
        for (auto name : files)
        {
            database["paths"][pos]["files"].push_back(name.toStdString());
        }

        return resultCode::OK;
    }

    if (code == resultCode::OK)
        return resultCode::incorrectPath;
    return code;
}

resultCode FileManager::setFiles(int pos_dir, const QStringList &files)
{
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir >= 0)
    {
        database["paths"][pos_dir]["files"] = "[]"_json;
        for (auto name : files)
        {
            database["paths"][pos_dir]["files"].push_back(name.toStdString());
        }

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

QStringList FileManager::parseFilters(const QString &filters)
{

    QStringList _filters = {};

    if (filters != "all" && filters != "")
    {
        _filters = filters.split("|");
    }

    return _filters;
}

void FileManager::printPaths()
{
    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        std::cout << i + 1 << ") " << database["paths"][i]["dir"].get<std::string>() \
                << ";" << std::endl;
    }
}

resultCode FileManager::printFiles(const QString& dir)
{
    resultCode code = checkPath(dir);

    if (isFile(dir))
        return resultCode::notFolder;

    if (code == resultCode::employedPath)
    {
        QStringList directoryFiles = getFiles(dir);

        for (qsizetype i = 0; i < directoryFiles.size(); i++)
        {
            std::cout << i + 1 << ") " << directoryFiles[i].toStdString() << ";" << std::endl;
        }

        return resultCode::OK;
    }
    return code;
}

void FileManager::deleteSomeCopiedFiles(const QStringList &files)
{
    QStringList files_to_delete;
    QDir copy_dir(copyFolderPath);
    QFileInfo file;

    for (qsizetype i = 0; i  < files.size(); i++)
    {
        file.setFile(files[i]);
        files_to_delete.push_back(copy_dir.filePath(file.fileName()));
    }

    addThread(files_to_delete, operationTypes::operation_delete);
}

//slots:

void FileManager::ifDirectoryChanged(const QString& path)
{
    QStringList files = getFiles(path);
    QStringList filesToCopy = {};
    QStringList filters = getFilters(path);
    QStringList filesToDelete = {};

    QDirIterator it(path, filters, QDir::Files);

    while (it.hasNext())
    {
       QString file_path = it.next();

       if (!files.contains(file_path))
       {
           files.push_back(file_path);
           filesToCopy.push_back(file_path);
       }
    }

    for (qsizetype i = 0; i < files.size(); i++)
    {
        if (!isExists(files[i]))
            filesToDelete.push_back(files[i]);
    }

    if (filesToCopy.size() != 0)
    {
        addThread(filesToCopy, operationTypes::operation_copy);
        addFiles(path, filesToCopy);
    }
    if (filesToDelete.size() != 0)
    {
        deleteSomeFiles(path, filesToDelete);
        deleteSomeCopiedFiles(filesToDelete);
    }
}

void FileManager::ifFileChanged(const QString& path)
{
    QDesktopServices::openUrl(QUrl(path));
}

//Private Methods:

void FileManager::addPathToWatcher()
{
    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        watcherDirectory.addPath(QString::fromStdString(database["paths"][i]["dir"].get<std::string>()));

        for (size_t j = 0; j < database["paths"][i]["files"].size(); j++)
        {
            watcherFile.addPath(QString::fromStdString(database["paths"][i]["files"][j].get<std::string>()));
        }
    }
}

void FileManager::addThread(const QStringList& files, operationTypes type)
{
    FileController *controller = new FileController(copyFolderPath, type);
    QThread* thread = new QThread;
    controller->addFiles(files);
    controller->moveToThread(thread);

    connect(thread, SIGNAL(started()), controller, SLOT(process()));
    connect(controller, SIGNAL(finished()), thread, SLOT(quit()));
    connect(controller, SIGNAL(finished()), controller, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();

    return;
}
