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
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法打开文件进行保存:" << filePath;
        return false;
    }

    QTextStream out(&file);

    // 保存元数据
    out << saveName << "\n";
    out << saveTime << "\n";
    out << saveSlot << "\n";

    // 保存游戏数据
    out << magicCrystal << "\n";
    out << playerData << "\n";
    out << weaponData_1 << "\n";
    out << weaponData_2 << "\n";
    out << static_cast<int>(passData_1_isPass) << "\n";
    out << static_cast<int>(passData_2_isPass) << "\n";

    file.close();
    return true;
}

bool SaveData::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "无法打开文件进行读取:" << filePath;
        return false;
    }

    QTextStream in(&file);

    // 读取元数据
    saveName = in.readLine();
    saveTime = in.readLine();
    in >> saveSlot;

    // 读取游戏数据
    in >> magicCrystal;
    in >> playerData;
    in >> weaponData_1;
    in >> weaponData_2;

    int isPass1, isPass2;
    in >> isPass1 >> isPass2;
    passData_1_isPass = static_cast<bool>(isPass1);
    passData_2_isPass = static_cast<bool>(isPass2);

    file.close();
    return true;
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
