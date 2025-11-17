#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QList>
#include <vector>

// 前向声明
class Enemy;
class Bullet;
struct PlayerData;
struct BulletData;

class Player : public QObject
{
    Q_OBJECT

public:
    // 方向枚举
    enum Direction
    {
        Up,
        Down,
        Left,
        Right
    };

    // 构造函数
    Player(const PlayerData &data, QPoint pos, QObject *parent = nullptr);
    ~Player();

    // 基础属性访问
    int getX() const { return static_cast<int>(x); }
    int getY() const { return static_cast<int>(y); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getHp() const { return hp; }
    int getCurrentImageIndex() const { return currentImageIndex; }
    int getPlayerId() const { return playerId; }
    int getDamage() const { return damage; }
    int getAttackRange() const { return attackRange; }

    void setPosition(double x, double y);
    void setHP(int hp);

    // 游戏逻辑设置
    void setEnemiesList(QList<Enemy *> *enemies) { enemiesList = enemies; }
    void setObstaclesList(QList<QRect> *obstacles) { obstaclesList = obstacles; }
    void setBulletData(const BulletData *data) { bulletData = data; }

    // 动画更新
    void updateAnimation(Direction dir);

    // 攻击相关
    bool canAttack() const;     // 检查是否可以攻击
    void attack(Enemy *target); // 攻击指定敌人
    void resetAttackCD();       // 重置攻击冷却

signals:
    void createBullet(Bullet *bullet); // 创建子弹信号

private:
    // 基础属性
    double x, y; // 使用double提高位置精度
    int width, height;
    int hp;
    int playerId;
    int moveStep;

    // 动画相关
    std::vector<int> rightWalkIndices; // 右走动画索引
    std::vector<int> leftWalkIndices;  // 左走动画索引
    std::vector<int> upWalkIndices;    // 上走动画索引
    std::vector<int> downWalkIndices;  // 下走动画索引
    int currentImageIndex;             // 当前图片索引
    int currentFrameInList;            // 当前在列表中的帧
    int FRAME_CNT;                     // 帧计数器阈值
    int frameCnt;                      // 当前帧计数
    Direction currentDirection;        // 当前方向

    // 攻击相关
    int damage;
    int attackCD;
    int attackRange;
    int bulletId;
    double bulletSpeed;
    QTimer *attackCDTimer;        // 攻击冷却定时器
    bool attackReady;             // 是否可以攻击
    const BulletData *bulletData; // 子弹数据指针

    // 游戏逻辑
    QList<Enemy *> *enemiesList;
    QList<QRect> *obstaclesList;

    // 私有方法
    void onAttackCDTimeout(); // 攻击冷却结束
};

#endif // PLAYER_H
