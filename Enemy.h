#ifndef ENEMY_H
#define ENEMY_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QPixmap>
#include <vector>
#include <utility>

// 前向声明
class Player;
class Bullet;
struct EnemyData;
struct BulletData;

class Enemy : public QObject
{
    Q_OBJECT

public:
    // 构造函数：通过数据初始化
    Enemy(const EnemyData &data, QPoint pos, QObject *parent = nullptr);
    ~Enemy();

    // 基础属性访问
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getHp() const { return hp; }
    int getCurrentImageIndex() const { return currentImageIndex; }
    int getEnemyType() const { return enemyType; }

    void setPosition(int x, int y);
    void setHP(int hp);

    // 游戏逻辑设置
    static void setAttackTarget(Player *target) { attackTarget = target; }
    static void setGrid(const std::vector<std::vector<int>> &gridData) { grid = gridData; }
    void setObstaclesList(QList<QRect> *obstacles) { obstaclesList = obstacles; }
    void setBulletData(const BulletData *data) { bulletData = data; }

    // 控制方法
    void stopMove();
    void startMove();
    void updateAnimation(); // 更新动画

signals:
    void createBullet(Bullet *bullet); // 创建子弹信号

private:
    // 基础属性
    double x, y; // 使用double提高位置精度
    int width, height;
    int hp;
    int damage;
    int enemyType;

    // 动画相关
    std::vector<int> imageIndices; // 图片索引列表
    int currentImageIndex;         // 当前图片索引
    int currentFrameInList;        // 当前在列表中的帧
    int FRAME_CNT;                 // 帧计数器阈值
    int frameCnt;                  // 当前帧计数

    // 移动相关
    bool canMove;
    int moveStep;
    std::pair<int, int> speedF;
    int pathUpdateFreq;
    std::vector<std::pair<int, int>> path; // 路径
    int pathIndex;
    QTimer *AIMoveTimer;
    QTimer *pathTimer;
    QTimer *moveSpeedChangeTimer;
    QList<QRect> *obstaclesList;

    // 攻击相关
    bool hasMeleeAttack;
    bool hasRangedAttack;
    int attackCD;
    int bulletId;
    double bulletSpeed;
    QTimer *attackTimer;
    const BulletData *bulletData; // 子弹数据指针

    // 静态成员
    static Player *attackTarget;
    static std::vector<std::vector<int>> grid;

    // 私有方法
    void changeMoveSpeed();                        // 改变移动速度
    void AIMove();                                 // AI移动
    void getNewPath();                             // 获取新路径
    void launchRangedAttack();                     // 发射远程攻击
    bool checkCollision(double newX, double newY); // 检测碰撞（考虑敌人大小）
};

#endif // ENEMY_H
