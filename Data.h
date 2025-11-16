#ifndef DATA_H
#define DATA_H

#include "Character.h"
#include <QTextStream>
#include <vector>
#include <string>

// 基础地图数据声明
extern const std::vector<std::vector<int>> basicGrid;

// 敌人数据类
class EnemyData
{
public:
    // 构造函数
    EnemyData() : type(Enemy::EnemyType::Goblin), hp(0), damage(0), CD(0), speedF({0, 0}), totalNum(0), generateF(0) {}

    EnemyData(Enemy::EnemyType t, std::vector<QPoint> pos, int h, int dmg, int cd,
              std::pair<int,int> speed, int num, int freq)
        : type(t), generatePos(pos), hp(h), damage(dmg), CD(cd),
          speedF(speed), totalNum(num), generateF(freq) {}

    // Getter方法
    Enemy::EnemyType getType() const { return type; }
    const std::vector<QPoint>& getGeneratePos() const { return generatePos; }
    int getHp() const { return hp; }
    int getDamage() const { return damage; }
    int getCD() const { return CD; }
    std::pair<int,int> getSpeedF() const { return speedF; }
    int getTotalNum() const { return totalNum; }
    int getGenerateF() const { return generateF; }

    // Setter方法
    void setType(Enemy::EnemyType t) { type = t; }
    void setGeneratePos(const std::vector<QPoint>& pos) { generatePos = pos; }
    void setHp(int h) { hp = h; }
    void setDamage(int dmg) { damage = dmg; }
    void setCD(int cd) { CD = cd; }
    void setSpeedF(std::pair<int,int> speed) { speedF = speed; }
    void setTotalNum(int num) { totalNum = num; }
    void setGenerateF(int freq) { generateF = freq; }

    // 为了兼容性，保留公共成员访问（可以逐步迁移到使用getter/setter）
    Enemy::EnemyType type; // 敌人类型
    std::vector<QPoint> generatePos;// 生成位置
    int hp;// 血量
    int damage;// 伤害
    int CD;// 攻击cd
    std::pair<int,int> speedF;// 移动频率
    int totalNum;// 总数量
    int generateF;// 生成频率
};

// 武器数据类
class WeaponData
{
public:
    // 构造函数
    WeaponData() : level(1), type(IceBall), bulletSpeedF(10) {}

    WeaponData(int lvl, BulletType t, int speed)
        : level(lvl), type(t), bulletSpeedF(speed) {}

    // Getter方法
    int getLevel() const { return level; }
    BulletType getType() const { return type; }
    int getBulletSpeedF() const { return bulletSpeedF; }

    // Setter方法
    void setLevel(int lvl) { level = lvl; }
    void setType(BulletType t) { type = t; }
    void setBulletSpeedF(int speed) { bulletSpeedF = speed; }

    // 流操作符重载
    friend QTextStream& operator>>(QTextStream& in, WeaponData& weaponData) {
        int type;
        in >> weaponData.level >> type >> weaponData.bulletSpeedF;
        weaponData.type = static_cast<BulletType>(type);
        return in;
    }

    friend QTextStream& operator<<(QTextStream& out, const WeaponData& weaponData) {
        int type = static_cast<int>(weaponData.type);
        out << weaponData.level << " " << type << " " << weaponData.bulletSpeedF;
        return out;
    }

    // 为了兼容性，保留公共成员访问
    int level; // 武器等级
    BulletType type; // 子弹类型
    int bulletSpeedF;// 子弹速度
};

// 玩家数据类
class PlayerData
{
public:
    // 构造函数
    PlayerData() : hp(3000), step(3), generatepos(0, 300) {}

    PlayerData(int h, int s, const WeaponData& weapon, const QPoint& pos)
        : hp(h), step(s), weaponData(weapon), generatepos(pos) {}

    // Getter方法
    int getHp() const { return hp; }
    int getStep() const { return step; }
    const WeaponData& getWeaponData() const { return weaponData; }
    QPoint getGeneratePos() const { return generatepos; }

    // Setter方法
    void setHp(int h) { hp = h; }
    void setStep(int s) { step = s; }
    void setWeaponData(const WeaponData& weapon) { weaponData = weapon; }
    void setGeneratePos(const QPoint& pos) { generatepos = pos; }

    // 流操作符重载
    friend QTextStream& operator>>(QTextStream& in, PlayerData& playerData) {
        int x, y;
        in >> playerData.hp >> playerData.step >> playerData.weaponData >> x >> y;
        playerData.generatepos = QPoint(x, y);
        return in;
    }

    friend QTextStream& operator<<(QTextStream& out, const PlayerData& playerData) {
        out << playerData.hp << " " << playerData.step << " " << playerData.weaponData
            << " " << playerData.generatepos.x() << " " << playerData.generatepos.y();
        return out;
    }

    // 为了兼容性，保留公共成员访问
    int hp;// 血量
    int step;// 移动速度
    WeaponData weaponData;// 武器数据
    QPoint generatepos;// 生成位置
};

// 剧情数据类
class PlotData
{
public:
    // 构造函数
    PlotData() : plotId(0) {}

    PlotData(int id, const std::vector<std::string>& texts, const std::string& bgPath)
        : plotId(id), textList(texts), backgroundPath(bgPath) {}

    // Getter方法
    int getPlotId() const { return plotId; }
    const std::vector<std::string>& getTextList() const { return textList; }
    std::vector<std::string>* getTextListPtr() { return &textList; }
    const std::string& getBackgroundPath() const { return backgroundPath; }
    std::string getBackgroundPathCopy() const { return backgroundPath; }

    // Setter方法
    void setPlotId(int id) { plotId = id; }
    void setTextList(const std::vector<std::string>& texts) { textList = texts; }
    void setBackgroundPath(const std::string& path) { backgroundPath = path; }
    void addText(const std::string& text) { textList.push_back(text); }
    void clearText() { textList.clear(); }

private:
    int plotId;                          // 剧情ID
    std::vector<std::string> textList;   // 剧情文本列表
    std::string backgroundPath;          // 背景图片路径
};

// 关卡数据类
class PassData
{
public:
    // 构造函数
    PassData() : level(1), isPass(false) {}

    PassData(int lvl, bool pass, const std::vector<EnemyData>& enemies,
             const std::vector<std::vector<int>>& gridData)
        : level(lvl), isPass(pass), enemyData(enemies), grid(gridData) {}

    // Getter方法
    int getLevel() const { return level; }
    bool getIsPass() const { return isPass; }
    const std::vector<EnemyData>& getEnemyData() const { return enemyData; }
    const std::vector<std::vector<int>>& getGrid() const { return grid; }

    // Setter方法
    void setLevel(int lvl) { level = lvl; }
    void setIsPass(bool pass) { isPass = pass; }
    void setEnemyData(const std::vector<EnemyData>& enemies) { enemyData = enemies; }
    void setGrid(const std::vector<std::vector<int>>& gridData) { grid = gridData; }

    // 为了兼容性，保留公共成员访问
    int level; // 关卡
    bool isPass; // 是否通过
    std::vector<EnemyData> enemyData;   // 敌人数据
    std::vector<std::vector<int>> grid; // 地图
};

// 游戏数据类
class GameData
{
public:
    // 构造函数
    GameData() : level(1) {}

    GameData(int lvl, const std::vector<std::vector<int>>& gridData,
             const PlayerData& player, const std::vector<EnemyData>& enemies)
        : level(lvl), grid(gridData), playerData(player), enemyData(enemies) {}

    // Getter方法
    int getLevel() const { return level; }
    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    const PlayerData& getPlayerData() const { return playerData; }
    const std::vector<EnemyData>& getEnemyData() const { return enemyData; }

    // Setter方法
    void setLevel(int lvl) { level = lvl; }
    void setGrid(const std::vector<std::vector<int>>& gridData) { grid = gridData; }
    void setPlayerData(const PlayerData& player) { playerData = player; }
    void setEnemyData(const std::vector<EnemyData>& enemies) { enemyData = enemies; }

    // 为了兼容性，保留公共成员访问
    int level; // 关卡
    std::vector<std::vector<int>> grid; // 地图
    PlayerData playerData; // 玩家数据
    std::vector<EnemyData> enemyData; // 敌人数据
};

#endif
//(10,20)--(20,20)--(20,10)
//(10,40)--(20,40)--(20,50)
//(60,20)--(70,20),(60,20)--(60,10)
//(60,40)--(70,40),(60,40)--(60,50)