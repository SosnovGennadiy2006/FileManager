#include "filemanager.h"

FileManager::FileManager()
{
    if (!openDB())
    {
        std::cout << "End!" << std::endl;
    }
}

bool FileManager::openDB()
{
    QFile file("../database.json");

    if (file.exists())
    {
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);

            database = nlohmann::json::parse(qPrintable(stream.readAll()));

            file.close();

            return true;
        }
    }
    return false;
}

bool FileManager::saveDB()
{
    QFile file("../database.json");

    if (file.exists())
    {
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);


            stream << QString::fromStdString(database.dump(4));

            file.close();

            return true;
        }
    }
    return false;
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

resultCode FileManager::addPath(const QString &path)
{
    if (isExists(path))
    {
        if (isDir(path))
        {
            QFileInfo p(path);
            if (!isEmployed(p.absoluteFilePath()))
            {
                nlohmann::json j;
                j["dir"] = qPrintable(p.absoluteFilePath());
                database["paths"].push_back(j);

                saveDB();
                return resultCode::OK;
            }
            return resultCode::employedPath;
        }
        return resultCode::notDir;
    }
    return resultCode::errorPath;
}

std::vector<QString> FileManager::getPaths()
{
    std::vector<QString> paths;

    for (size_t i = 0; i < database["paths"].size(); i++)
    {
        paths.push_back(QString::fromStdString(database["paths"][i]["dir"].dump()));
    }

    return paths;
}
