#include "Enemy.h"
#include "Character.h"
#include "Bullet.h"
#include "Data.h"
#include "findpath.h"
#include <QRandomGenerator>
#include <cmath>

// 静态成员初始化
Player *Enemy::attackTarget = nullptr;
std::vector<std::vector<int>> Enemy::grid;

Enemy::Enemy(const EnemyData &data, QPoint pos, QObject *parent)
    : QObject(parent),
      x(pos.x()), y(pos.y()),
      width(data.width), height(data.height),
      hp(data.hp), damage(data.damage),
      enemyType(data.enemyType),
      imageIndices(data.imageIndices),
      currentImageIndex(0), currentFrameInList(0),
      FRAME_CNT(data.frameCnt), frameCnt(0),
      canMove(data.canMove), moveStep(data.moveStep),
      speedF(data.speedF), pathUpdateFreq(data.pathUpdateFreq),
      pathIndex(0), obstaclesList(nullptr),
      hasMeleeAttack(data.hasMeleeAttack),
      hasRangedAttack(data.hasRangedAttack),
      attackCD(data.attackCD), bulletId(data.bulletId),
      bulletSpeed(data.bulletSpeed), bulletData(nullptr)
{
    // 初始化当前图片索引
    if (!imageIndices.empty())
    {
        currentImageIndex = imageIndices[0];
    }

    // 初始化定时器
    AIMoveTimer = nullptr;
    pathTimer = nullptr;
    moveSpeedChangeTimer = nullptr;
    attackTimer = nullptr;

    // 如果可以移动，初始化移动相关定时器
    if (canMove)
    {
        moveSpeedChangeTimer = new QTimer(this);
        connect(moveSpeedChangeTimer, &QTimer::timeout, this, &Enemy::changeMoveSpeed);
        moveSpeedChangeTimer->start(500);

        pathTimer = new QTimer(this);
        connect(pathTimer, &QTimer::timeout, this, &Enemy::getNewPath);
        pathTimer->start(pathUpdateFreq);

        AIMoveTimer = new QTimer(this);
        connect(AIMoveTimer, &QTimer::timeout, this, &Enemy::AIMove);
        // 初始移动速度
        changeMoveSpeed();
    }

    // 如果有远程攻击，初始化攻击定时器
    if (hasRangedAttack)
    {
        attackTimer = new QTimer(this);
        connect(attackTimer, &QTimer::timeout, this, &Enemy::launchRangedAttack);
        attackTimer->start(attackCD);
    }
}

Enemy::~Enemy()
{
    // 停止所有定时器
    if (AIMoveTimer)
    {
        AIMoveTimer->stop();
        delete AIMoveTimer;
    }
    if (pathTimer)
    {
        pathTimer->stop();
        delete pathTimer;
    }
    if (moveSpeedChangeTimer)
    {
        moveSpeedChangeTimer->stop();
        delete moveSpeedChangeTimer;
    }
    if (attackTimer)
    {
        attackTimer->stop();
        delete attackTimer;
    }

    this->disconnect();
}

void Enemy::setPosition(int newX, int newY)
{
    x = newX;
    y = newY;
}

void Enemy::setHP(int newHp)
{
    hp = newHp;
}

void Enemy::stopMove()
{
    if (AIMoveTimer)
        AIMoveTimer->stop();
    if (pathTimer)
        pathTimer->stop();
    if (moveSpeedChangeTimer)
        moveSpeedChangeTimer->stop();
}

void Enemy::startMove()
{
    if (canMove)
    {
        if (pathTimer)
            pathTimer->start(pathUpdateFreq);
        if (moveSpeedChangeTimer)
            moveSpeedChangeTimer->start(500);
        changeMoveSpeed();
    }
}

void Enemy::updateAnimation()
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

void Enemy::changeMoveSpeed()
{
    if (!canMove || !AIMoveTimer)
        return;

    int newSpeed = QRandomGenerator::global()->bounded(speedF.first, speedF.second);
    AIMoveTimer->start(newSpeed);
}

void Enemy::getNewPath()
{
    if (attackTarget == nullptr)
        return;

    path.clear();

    // 计算敌人中心点在网格中的位置（考虑敌人大小）
    int enemyCenterX = static_cast<int>((x + width / 2.0) / moveStep);
    int enemyCenterY = static_cast<int>((y + height / 2.0) / moveStep);

    // 计算目标中心点在网格中的位置
    int targetCenterX = (attackTarget->getX() + attackTarget->width / 2) / moveStep;
    int targetCenterY = (attackTarget->getY() + attackTarget->height / 2) / moveStep;

    path = AStar(grid,
                 std::make_pair(enemyCenterX, enemyCenterY),
                 std::make_pair(targetCenterX, targetCenterY));
    pathIndex = 0;
}

bool Enemy::checkCollision(double newX, double newY)
{
    if (attackTarget == nullptr)
        return false;

    // 检查与目标的碰撞（使用敌人的完整矩形）
    QRect enemyRect(static_cast<int>(newX), static_cast<int>(newY), width, height);
    QRect targetRect(attackTarget->getX(), attackTarget->getY(),
                     attackTarget->width, attackTarget->height);

    if (enemyRect.intersects(targetRect))
    {
        // 近战攻击
        if (hasMeleeAttack)
        {
            attackTarget->setHP(attackTarget->getHp() - damage);
        }
        return true;
    }

    // 检查与障碍物的碰撞
    if (obstaclesList != nullptr)
    {
        for (const QRect &obstacle : *obstaclesList)
        {
            if (enemyRect.intersects(obstacle))
            {
                return true;
            }
        }
    }

    return false;
}

void Enemy::AIMove()
{
    if (!canMove || attackTarget == nullptr)
        return;

    // 如果路径为空或已到达终点，获取新路径
    if (path.empty() || pathIndex >= path.size() - 1)
    {
        getNewPath();
        pathIndex = 0;
        return;
    }

    pathIndex++;

    // 计算目标位置（将网格坐标转换为实际坐标，并考虑敌人大小使其居中）
    double targetX = path[pathIndex].first * moveStep - width / 2.0;
    double targetY = path[pathIndex].second * moveStep - height / 2.0;

    // 检测碰撞
    if (checkCollision(targetX, targetY))
    {
        // 发生碰撞，重新规划路径
        getNewPath();
        pathIndex = 0;
        return;
    }

    // 更新位置
    x = targetX;
    y = targetY;

    // 更新动画（根据移动方向）
    if (!imageIndices.empty() && imageIndices.size() >= 4)
    {
        // 假设图片索引：0-1为右，2-3为左
        if (attackTarget->getX() > x)
        {
            // 向右移动
            updateAnimation();
        }
        else if (attackTarget->getX() < x)
        {
            // 向左移动
            updateAnimation();
        }
    }
}

void Enemy::launchRangedAttack()
{
    if (hp <= 0)
    {
        if (attackTimer)
            attackTimer->stop();
        return;
    }

    if (attackTarget == nullptr || bulletData == nullptr)
        return;

    // 计算敌人和目标的中心点（使用double提高精度）
    double enemyCenterX = x + width / 2.0;
    double enemyCenterY = y + height / 2.0;
    double targetCenterX = attackTarget->getX() + attackTarget->width / 2.0;
    double targetCenterY = attackTarget->getY() + attackTarget->height / 2.0;

    // 计算方向向量（使用double）
    double dx = targetCenterX - enemyCenterX;
    double dy = targetCenterY - enemyCenterY;
    double distance = std::sqrt(dx * dx + dy * dy);

    // 避免除以零
    if (distance < 0.001)
        return;

    // 归一化方向向量并乘以子弹速度（保持double精度）
    double bulletDx = (dx / distance) * bulletSpeed;
    double bulletDy = (dy / distance) * bulletSpeed;

    // 使用新的Bullet接口创建子弹
    Bullet *bullet = Bullet::createBullet(
        *bulletData,
        static_cast<int>(enemyCenterX),
        static_cast<int>(enemyCenterY),
        bulletDx,
        bulletDy,
        attackTarget,
        damage);

    bullet->startMove();
    emit createBullet(bullet);
}
