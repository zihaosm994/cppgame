#ifndef DATA_H
#define DATA_H

#include "Character.h"
#include <QTextStream>
extern const std::vector<std::vector<int>> basicGrid;
struct EnemyData
{
    Enemy::EnemyType type; // 敌人类型
    std::vector<QPoint> generatePos;// 生成位置
    int hp;// 血量
    int damage;// 伤害
    int CD;// 攻击cd
    std::pair<int,int> speedF;// 移动频率
    int totalNum;// 总数量
    int generateF;// 生成频率
};
struct WeaponData
{
    int level; // 武器等级
    BulletType type; // 子弹类型
    int bulletSpeedF;// 子弹速度
    //重写输入输出,写到哪里去要原本不动的读入
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
    
};
struct PlayerData{
    int hp;// 血量
    int step;        // 移动速度
    WeaponData weaponData;           // 武器数据
    QPoint generatepos;              // 生成位置    
    //重写输入输出,写到哪里去要原本不动的读入
    friend QTextStream& operator>>(QTextStream& in, PlayerData& playerData) {
        int x, y;
        in >> playerData.hp >> playerData.step >> playerData.weaponData >> x >> y;
        playerData.generatepos = QPoint(x, y);
        return in;
    }
    friend QTextStream& operator<<(QTextStream& out, const PlayerData& playerData) {
        out << playerData.hp << " " << playerData.step << " " << playerData.weaponData << " " << playerData.generatepos.x() << " " << playerData.generatepos.y();
        return out;
    }
};
struct PassData{
    int level; // 关卡
    bool isPass; // 是否通过
    std::vector<EnemyData> enemyData;   // 敌人数据
    std::vector<std::vector<int>> grid; // 地图
};
struct GameData{
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