#ifndef ENEMY_H
#define ENEMY_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QPixmap>
#include <vector>
#include "Data.h"
#include "Player.h"
#include "Character.h"

class Enemy : public Character
{
    Q_OBJECT

public:
    // 构造函数：通过数据初始化
    Enemy(QObject *parent = nullptr);
    ~Enemy();
    bool isDead;

    // 基础属性访问
    int getX() const override{ return x; }
    int getY() const override{ return y; }
    int getWidth() const override{ return data->width; }
    int getHeight() const override{ return data->height; }
    int getHp() const override{ return hp; }

    void setPosition(int x, int y);
    void setHP(int hp)override;

    // 游戏逻辑设置
    static void setAttackTarget(Player *target) { attackTarget = target; }
    void setBulletData(BulletData *bulletdata) { this->data->bulletData = bulletdata; }


    // 控制方
    void stopMove();
    void startMove();
    static MapData *mapData;
    static QList<QRect> * obstacles;
    // 动画更新
    int updateAnimation(){
        pictureIndex[curState].curIndex=(pictureIndex[curState].curIndex+1)%pictureIndex[curState].maxCnt;
        return pictureIndex[curState].curIndex;
    }
    Direction getState(){return curState;}
    int getID(){return data->enemyID;}
    static void initEnemiesPool(int num);
    static Enemy* createEnemy(EnemySpawnConfig& config);

    static std::vector<Enemy*>* enemiesPool;
    void reset();
    static QList<Enemy *> *allenemies;
    void avoidOthers();

    void stop(){
        if(AIMoveTimer){
            AIMoveTimer->stop();
        }
        if(moveSpeedChangeTimer){
            moveSpeedChangeTimer->stop();
        }
        if(attackTimer){
            attackTimer->stop();
        }
    }
    void start(){
        if(AIMoveTimer){
            AIMoveTimer->start();
        }
        if(moveSpeedChangeTimer){
            moveSpeedChangeTimer->start();
        }
        if(attackTimer){
            attackTimer->start();
        }
    }

signals:
    void createBullet(Bullet *bullet); // 创建子弹信号

private:
    // 基础属性
    int x,y;
    int hp;
    // 动画相关
    Direction curState;
    std::map<Direction,PictureIndex> pictureIndex;

    int pathIndex;
    QTimer *AIMoveTimer;
    QTimer *moveSpeedChangeTimer;

    QTimer *attackTimer;
    EnemyData *data;

    // 静态成员
    static Player *attackTarget;

    // 私有方法
    void changeMoveSpeed();                        // 改变移动速度
    void AIMove();                                 // AI移动                            // 获取新路径
    void launchRangedAttack();                     // 发射远程攻击
    bool checkCollisionObstacle(int newX, int newY); // 检测碰撞
    bool checkCollisionTarget(int newX,int newY);
};

#endif // ENEMY_H
