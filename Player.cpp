#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Data.h"
#include <cmath>

Player::Player(const PlayerData &data, QPoint pos, QObject *parent)
    : QObject(parent),
      x(pos.x()), y(pos.y()),
      width(data.width), height(data.height),
      hp(data.hp), playerId(data.playerId),
      moveStep(data.moveStep),
      rightWalkIndices(data.rightWalkIndices),
      leftWalkIndices(data.leftWalkIndices),
      upWalkIndices(data.upWalkIndices),
      downWalkIndices(data.downWalkIndices),
      currentImageIndex(0), currentFrameInList(0),
      FRAME_CNT(data.frameCnt), frameCnt(0),
      currentDirection(Right),
      damage(data.damage), attackCD(data.attackCD),
      attackRange(data.attackRange), bulletId(data.bulletId),
      bulletSpeed(data.bulletSpeed),
      attackReady(true), bulletData(nullptr),
      enemiesList(nullptr), obstaclesList(nullptr)
{
    // 初始化当前图片索引（默认使用右走的第一帧）
    if (!rightWalkIndices.empty())
    {
        currentImageIndex = rightWalkIndices[0];
    }

    // 初始化攻击冷却定时器
    attackCDTimer = new QTimer(this);
    attackCDTimer->setSingleShot(true);
    connect(attackCDTimer, &QTimer::timeout, this, &Player::onAttackCDTimeout);
}

Player::~Player()
{
    if (attackCDTimer)
    {
        attackCDTimer->stop();
        delete attackCDTimer;
    }
    this->disconnect();
}

void Player::setPosition(double newX, double newY)
{
    x = newX;
    y = newY;
}

void Player::setHP(int newHp)
{
    hp = newHp;
}

void Player::updateAnimation(Direction dir)
{
    currentDirection = dir;

    // 更新帧计数
    frameCnt = (frameCnt + 1) % FRAME_CNT;
    if (frameCnt == 0)
    {
        // 根据方向选择对应的图片索引列表
        std::vector<int> *currentIndices = nullptr;

        switch (dir)
        {
        case Up:
            if (!upWalkIndices.empty())
                currentIndices = &upWalkIndices;
            break;
        case Down:
            if (!downWalkIndices.empty())
                currentIndices = &downWalkIndices;
            break;
        case Left:
            if (!leftWalkIndices.empty())
                currentIndices = &leftWalkIndices;
            break;
        case Right:
            if (!rightWalkIndices.empty())
                currentIndices = &rightWalkIndices;
            break;
        }

        // 更新图片索引
        if (currentIndices != nullptr && !currentIndices->empty())
        {
            currentFrameInList = (currentFrameInList + 1) % currentIndices->size();
            currentImageIndex = (*currentIndices)[currentFrameInList];
        }
    }
}

bool Player::canAttack() const
{
    return attackReady;
}

void Player::attack(Enemy *target)
{
    if (!attackReady || target == nullptr || bulletData == nullptr)
        return;

    // 计算玩家和目标的中心点（使用double提高精度）
    double playerCenterX = x + width / 2.0;
    double playerCenterY = y + height / 2.0;
    double targetCenterX = target->getX() + target->getWidth() / 2.0;
    double targetCenterY = target->getY() + target->getHeight() / 2.0;

    // 计算距离
    double dx = targetCenterX - playerCenterX;
    double dy = targetCenterY - playerCenterY;
    double distance = std::sqrt(dx * dx + dy * dy);

    // 检查是否在攻击范围内
    if (distance > attackRange)
        return;

    // 避免除以零
    if (distance < 0.001)
        return;

    // 归一化方向向量并乘以子弹速度（保持double精度）
    double bulletDx = (dx / distance) * bulletSpeed;
    double bulletDy = (dy / distance) * bulletSpeed;

    // 创建子弹
    Bullet *bullet = Bullet::createBullet(
        *bulletData,
        static_cast<int>(playerCenterX),
        static_cast<int>(playerCenterY),
        bulletDx,
        bulletDy,
        target,
        damage);

    // 设置子弹的敌人列表和障碍物列表
    bullet->setEnemiesList(enemiesList);
    bullet->setObstaclesList(obstaclesList);

    bullet->startMove();
    emit createBullet(bullet);

    // 开始冷却
    attackReady = false;
    attackCDTimer->start(attackCD);
}

void Player::resetAttackCD()
{
    attackReady = true;
    if (attackCDTimer)
        attackCDTimer->stop();
}

void Player::onAttackCDTimeout()
{
    attackReady = true;
}
