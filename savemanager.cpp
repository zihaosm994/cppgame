#include "savemanager.h"
#include <QDateTime>
#include <QDebug>

// SaveData实现
SaveData::SaveData()
    : magicCrystal(0),
    passData_1_isPass(false),
    passData_2_isPass(false),
    saveSlot(0)
{
}

bool SaveData::saveToFile(const QString &filePath)
{
    return false;
}

bool SaveData::loadFromFile(const QString &filePath)
{
    return false;
}

// SaveManager实现
SaveManager::SaveManager()
{
}

SaveManager::~SaveManager()
{
}

QString SaveManager::getSaveDirectory()
{
    QString saveDir = QDir::currentPath() + "/saves";
    QDir dir;
    if (!dir.exists(saveDir))
    {
        dir.mkpath(saveDir);
    }
    return saveDir;
}

QString SaveManager::getSaveFilePath(int slot)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS)
    {
        return QString();
    }
    return getSaveDirectory() + QString("/save_%1.dat").arg(slot);
}

bool SaveManager::saveGame(const SaveData &data, int slot)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS)
    {
        qDebug() << "无效的存档槽位:" << slot;
        return false;
    }

    SaveData saveData = data;
    saveData.saveSlot = slot;
    saveData.saveTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString filePath = getSaveFilePath(slot);
    return saveData.saveToFile(filePath);
}

bool SaveManager::loadGame(SaveData &data, int slot)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS)
    {
        qDebug() << "无效的存档槽位:" << slot;
        return false;
    }

    QString filePath = getSaveFilePath(slot);
    return data.loadFromFile(filePath);
}

bool SaveManager::hasSave(int slot)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS)
    {
        return false;
    }

    QString filePath = getSaveFilePath(slot);
    return QFile::exists(filePath);
}

QString SaveManager::getSaveInfo(int slot)
{
    if (!hasSave(slot))
    {
        return "空存档";
    }

    SaveData data;
    if (data.loadFromFile(getSaveFilePath(slot)))
    {
        return QString("%1 - %2").arg(data.saveName).arg(data.saveTime);
    }

    return "读取失败";
}

bool SaveManager::deleteSave(int slot)
{
    if (!hasSave(slot))
    {
        return false;
    }

    QString filePath = getSaveFilePath(slot);
    return QFile::remove(filePath);
}
