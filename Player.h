#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QList>
#include "Character.h"
#include "Data.h"
// 前向声明
class Enemy;
class Bullet;
struct PlayerData;

class Player : public Character
{
    Q_OBJECT

public:

    // 构造函数
    Player(const PlayerData &data, QPoint pos, QObject *parent = nullptr);
    ~Player();

    // 基础属性访问
    int getX()  const override{ return x; }
    int getY() const override{ return y; }
    int getWidth() const override{ return width; }
    int getHeight() const override{ return height; }
    int getHp() const override{ return hp; }
    int getDamage() const { return damage; }
    int getAttackRange() const { return attackRange; }
    int getAttackCD()const{return attackCD;}

    void setPosition(double x, double y);
    void setHP(int hp)override;

    // 游戏逻辑设置
    void setEnemiesList(QList<Enemy *> *enemies) { enemiesList = enemies; }
    void setObstaclesList(QList<QRect> *obstacles) { obstaclesList = obstacles; }
    void addBulletData(BulletData &data) { bulletData.push_back(data); }

    // 攻击相关
    bool canAttack() const;     // 检查是否可以攻击
    void attack(Enemy *target); // 攻击指定敌人
    void resetAttackCD();       // 重置攻击冷却
    void upgrade(int dhp = 0,int ddamage = 0,int dattackRange =0,int dattackCD =0,int dmoveStep =0,BulletData * data = nullptr){
        hp+=dhp;
        damage+=ddamage;
        attackRange+=dattackRange;
        attackCD+=dattackCD;
        moveStep+=dmoveStep;
        if(data){addBulletData(*data);}
    }
    //更新动画
    int updateAnimation();
    void setState(Direction state){curState=state;}
    Direction getState(){return curState;}
    std::map<Direction,PictureIndex> pictureIndex;

signals:
    void createBullet(Bullet *bullet); // 创建子弹信号

private:
    // 基础属性
    int x, y;
    int width, height;
    int hp;
    int moveStep;
    // 动画相关
    Direction curState;

    // 攻击相关
    int damage;
    int attackCD;
    int attackRange;
    QTimer *attackCDTimer;        // 攻击冷却定时器
    bool attackReady;             // 是否可以攻击
    std::vector< BulletData > bulletData; // 子弹数据指针

    // 游戏逻辑
    QList<Enemy *> *enemiesList;
    QList<QRect> *obstaclesList;

    // 私有方法
    void onAttackCDTimeout(); // 攻击冷却结束
};

#endif // PLAYER_H
