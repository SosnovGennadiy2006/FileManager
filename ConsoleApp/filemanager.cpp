#include "filemanager.h"

//Public Methods:

FileManager::FileManager()
{
#ifdef QT_DEBUG
    database_path = "../database.json";
#else
    database_path = "../../database.json";
#endif
    openDB();

    if (isDBOpen())
    {
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

        checkDirectories();
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
        if (isDir(directory))
        {
            QFileInfo p(directory);
            if (!isEmployed(p.absoluteFilePath()))
            {
                return resultCode::OK;
            }
            return resultCode::employedPath;
        }
        return resultCode::notFolder;
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
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir > 0)
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

    if (code == resultCode::OK)
    {
        QStringList filters = {};

        if (pattern != "all" && pattern != "")
            filters = pattern.split("|");

        QFileInfo p(path);
        QDir dir(p.absoluteFilePath());

        nlohmann::json j;
        j["dir"] = p.absoluteFilePath().toStdString();
        j["pattern"] = pattern.toStdString();
        j["files"] = "[]"_json;
        j["deleted_files"] = "[]"_json;

        QDirIterator it(p.absoluteFilePath(), filters, QDir::Files);

        while (it.hasNext())
        {
            j["files"].push_back(it.next().toStdString());
        }

        database["paths"].push_back(j);

        saveDB();
    }

    return code;
}

resultCode FileManager::deletePath(const QString &path)
{
    resultCode code = checkPath(path);

    if (code == resultCode::employedPath)
    {
        int pos = findDirectory(path);

        if (pos != -1)
        {
            database["paths"].erase(pos);

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
    if (pos < static_cast<int>(database["paths"].size()) || pos > 0)
    {
        database["paths"].erase(pos);

        return resultCode::OK;
    }
    return resultCode::incorrectPathId;
}

resultCode FileManager::deleteFileSafety(int pos_dir, int pos_file)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir > 0)
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
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir > 0)
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

resultCode FileManager::makeCommand(int argc, char **argv)
{
    if (isDBOpen())
    {
        if (argc == 3)
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

QString FileManager::getDirectory(int pos_dir)
{
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir > 0)
    {
        return QString::fromStdString(database["paths"][pos_dir].get<std::string>());
    }
    return "";
}

QString FileManager::getFile(int pos_dir, int pos_file)
{
    if (pos_dir < static_cast<int>(database["paths"].size()) || pos_dir > 0)
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

    if (code == resultCode::employedPath)
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
    if (pos_dir <= static_cast<int>(database["paths"].size()) || pos_dir > 0)
    {
        return getFiles(QString::fromStdString(database["paths"][pos_dir]["dir"].get<std::string>()));
    }
    return files;
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

void FileManager::ifDirectoryChanged(const QString& path)
{
    std::cout << qPrintable(path) << std::endl;

}

void FileManager::ifFileChanged(const QString& path)
{
    std::cout << qPrintable(path) << std::endl;

    //QDesktopServices::openUrl(QUrl(path));
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
