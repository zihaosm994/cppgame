#include "Bullet.h"
#include "Character.h"
#include "Enemy.h"
#include "Data.h"
#include <cmath>

// 静态成员初始化
std::vector<Bullet *> Bullet::bulletsPool;

Bullet::Bullet(QObject *parent)
    : QObject(parent),
      x(0), y(0), dx(0), dy(0),
      width(20), height(20),
      bulletId(0), damage(0),
      currentImageIndex(0), currentFrameInList(0),
      FRAME_CNT(8), frameCnt(0),
      moveSpeed(10),
      target(nullptr), targetWidth(60), targetHeight(90),
      enemiesList(nullptr), obstaclesList(nullptr)
{
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &Bullet::move);
}

Bullet::~Bullet()
{
    if (moveTimer)
    {
        moveTimer->stop();
        delete moveTimer;
    }
    this->disconnect();
}

void Bullet::setPosition(double newX, double newY)
{
    x = newX;
    y = newY;
}

void Bullet::startMove()
{
    if (moveTimer)
    {
        moveTimer->start(moveSpeed);
    }
}

void Bullet::stopMove()
{
    if (moveTimer)
    {
        moveTimer->stop();
    }
}

void Bullet::updateAnimation()
{
    if (imageIndices.empty())
        return;

    frameCnt = (frameCnt + 1) % FRAME_CNT;
    if (frameCnt == 0)
    {
        currentFrameInList = (currentFrameInList + 1) % imageIndices.size();
        currentImageIndex = imageIndices[currentFrameInList];
    }
}

void Bullet::initialize(const BulletData &data, int startX, int startY,
                        double deltaX, double deltaY, Character *tgt, int baseDamage)
{
    // 基础属性
    x = startX;
    y = startY;
    dx = deltaX;
    dy = deltaY;
    width = data.width;
    height = data.height;
    bulletId = data.bulletId;
    damage = static_cast<int>(baseDamage * data.damageMultiplier);

    // 动画相关
    imageIndices = data.imageIndices;
    currentFrameInList = 0;
    frameCnt = 0;
    FRAME_CNT = data.frameCnt;
    if (!imageIndices.empty())
    {
        currentImageIndex = imageIndices[0];
    }
    else
    {
        currentImageIndex = 0;
    }

    // 移动相关
    moveSpeed = data.moveSpeed;

    // 目标相关
    target = tgt;
    targetWidth = data.targetWidth;
    targetHeight = data.targetHeight;

    // 重置列表（需要外部设置）
    enemiesList = nullptr;
    obstaclesList = nullptr;
}

void Bullet::move()
{
    // 更新动画
    updateAnimation();

    // 检测与目标的碰撞
    if (target != nullptr)
    {
        QRect bulletRect(static_cast<int>(x), static_cast<int>(y), width, height);
        QRect targetRect(target->getX(), target->getY(), targetWidth, targetHeight);

        if (bulletRect.intersects(targetRect))
        {
            target->setHP(target->getHp() - damage);
            moveTimer->stop();
            emit hitSth();
            deleteBullet(this);
            return;
        }
    }

    // 边界检测（使用较大的地图边界）
    if (x + dx >= 3200 || x + dx <= 0 || y + dy >= 2400 || y + dy <= 0)
    {
        moveTimer->stop();
        emit hitSth();
        deleteBullet(this);
        return;
    }

    // 检测与敌人的碰撞（如果是玩家的子弹）
    if (enemiesList != nullptr)
    {
        QRect bulletRect(static_cast<int>(x), static_cast<int>(y), width, height);
        for (auto enemy : *enemiesList)
        {
            QRect enemyRect(enemy->getX(), enemy->getY(), enemy->getWidth(), enemy->getHeight());
            if (bulletRect.intersects(enemyRect))
            {
                moveTimer->stop();
                emit hitSth();
                enemy->setHP(enemy->getHp() - damage);
                deleteBullet(this);
                return;
            }
        }
    }

    // 检测与障碍物的碰撞
    if (obstaclesList != nullptr)
    {
        QRect bulletRect(static_cast<int>(x), static_cast<int>(y), width, height);
        for (const QRect &obstacle : *obstaclesList)
        {
            if (bulletRect.intersects(obstacle))
            {
                moveTimer->stop();
                emit hitSth();
                deleteBullet(this);
                return;
            }
        }
    }

    // 更新位置（使用double精度）
    x += dx;
    y += dy;
}

// 子弹池管理
void Bullet::initBulletsPool(int totalCount)
{
    for (int i = 0; i < totalCount; i++)
    {
        Bullet *bullet = new Bullet();
        bulletsPool.push_back(bullet);
    }
}

Bullet *Bullet::createBullet(const BulletData &data, int x, int y,
                             double dx, double dy, Character *target,
                             int baseDamage)
{
    // 从子弹池中查找未使用的子弹
    for (auto bullet : bulletsPool)
    {
        if (!bullet->isUsed)
        {
            bullet->isUsed = true;
            bullet->initialize(data, x, y, dx, dy, target, baseDamage);
            return bullet;
        }
    }

    // 如果池中没有可用子弹，创建新的
    Bullet *newBullet = new Bullet();
    newBullet->isUsed = true;
    bulletsPool.push_back(newBullet);
    newBullet->initialize(data, x, y, dx, dy, target, baseDamage);
    return newBullet;
}

void Bullet::deleteBullet(Bullet *bullet)
{
    if (bullet)
    {
        bullet->isUsed = false;
        bullet->stopMove();
    }
}
