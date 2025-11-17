#ifndef BULLET_H
#define BULLET_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QList>
#include <vector>

// 前向声明
class Character;
class Enemy;
struct BulletData;

class Bullet : public QObject
{
    Q_OBJECT

public:
    // 构造函数（用于子弹池）
    Bullet(QObject *parent = nullptr);
    ~Bullet();

    // 基础属性访问
    int getX() const { return static_cast<int>(x); }
    int getY() const { return static_cast<int>(y); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getCurrentImageIndex() const { return currentImageIndex; }
    int getBulletId() const { return bulletId; }

    void setPosition(double x, double y);

    // 子弹池管理
    bool isUsed = false; // 是否正在使用
    static std::vector<Bullet *> bulletsPool;
    static void initBulletsPool(int totalCount);
    static Bullet *createBullet(const BulletData &data, int x, int y,
                                double dx, double dy, Character *target,
                                int baseDamage);
    static void deleteBullet(Bullet *bullet);

    // 游戏逻辑设置
    void setEnemiesList(QList<Enemy *> *enemies) { enemiesList = enemies; }
    void setObstaclesList(QList<QRect> *obstacles) { obstaclesList = obstacles; }

    // 控制方法
    void startMove();
    void stopMove();
    void updateAnimation(); // 更新动画

signals:
    void hitSth(); // 击中信号

private:
    // 基础属性
    double x, y;   // 使用double提高位置精度
    double dx, dy; // 使用double提高速度精度
    int width, height;
    int bulletId;
    int damage; // 实际伤害（基础伤害 * 倍率）

    // 动画相关
    std::vector<int> imageIndices; // 图片索引列表
    int currentImageIndex;         // 当前图片索引
    int currentFrameInList;        // 当前在列表中的帧
    int FRAME_CNT;                 // 帧计数器阈值
    int frameCnt;                  // 当前帧计数

    // 移动相关
    int moveSpeed;     // 移动速度（定时器间隔）
    QTimer *moveTimer; // 移动定时器

    // 目标和碰撞检测
    Character *target;             // 目标
    int targetWidth, targetHeight; // 目标尺寸
    QList<Enemy *> *enemiesList;   // 敌人列表
    QList<QRect> *obstaclesList;   // 障碍物列表

    // 私有方法
    void move();                            // 移动函数
    void initialize(const BulletData &data, // 初始化子弹
                    int x, int y, double dx, double dy,
                    Character *target, int baseDamage);
};

#endif // BULLET_H
