#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "Data.h"

// 存档数据结构
class SaveData
{
public:
    SaveData();

    // 游戏进度数据
    int magicCrystal;           // 魔法水晶数量
    PlayerData playerData;      // 玩家数据
    WeaponData weaponData_1;    // 武器1数据
    WeaponData weaponData_2;    // 武器2数据
    bool passData_1_isPass;     // 关卡1通关状态
    bool passData_2_isPass;     // 关卡2通关状态

    // 存档元数据
    QString saveName;           // 存档名称
    QString saveTime;           // 保存时间
    int saveSlot;               // 存档槽位（0-2）

    // 序列化/反序列化
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);
};

// 存档管理器
class SaveManager
{
public:
    SaveManager();
    ~SaveManager();

    // 保存/加载存档
    static bool saveGame(const SaveData &data, int slot);
    static bool loadGame(SaveData &data, int slot);

    // 检查存档是否存在
    static bool hasSave(int slot);

    // 获取存档信息
    static QString getSaveInfo(int slot);

    // 删除存档
    static bool deleteSave(int slot);

    // 获取存档文件路径
    static QString getSaveFilePath(int slot);

    // 获取存档目录
    static QString getSaveDirectory();

private:
    static const int MAX_SAVE_SLOTS = 3; // 最多3个存档槽位
};

#endif // SAVEMANAGER_H
