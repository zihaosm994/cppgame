#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Data.h"
#include <cmath>
#include <qobject>

Player::Player(const PlayerData &data, QPoint pos, QObject *parent)
    :   Character(parent),
      x(pos.x()), y(pos.y()),
      width(data.width), height(data.height),
      hp(data.hp),
      moveStep(data.moveStep),
      damage(data.damage), attackCD(data.attackCD),
      attackRange(data.attackRange),
      attackReady(true), bulletData(data.bulletData),
      enemiesList(nullptr), obstaclesList(nullptr)
{
    // 初始化攻击冷却定时器
    attackCDTimer = new QTimer(this);
    attackCDTimer->setSingleShot(true);
    connect(attackCDTimer, &QTimer::timeout, this, &Player::onAttackCDTimeout);
    // 初始化pictureIndex
    curState=Up;
    pictureIndex[Up]=PictureIndex(0,static_cast<int>(data.upWalkPaths.size()));
    pictureIndex[Right]=PictureIndex(0,static_cast<int>(data.rightWalkPaths.size()));
    pictureIndex[Down]=PictureIndex(0,static_cast<int>(data.downWalkPaths.size()));
    pictureIndex[Left]=PictureIndex(0,static_cast<int>(data.leftWalkPaths.size()));
}

Player::~Player()
{
    if (attackCDTimer)
    {
        attackCDTimer->stop();
        delete attackCDTimer;
    }
    this->disconnect();
    delete obstaclesList;
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

bool Player::canAttack() const
{
    return attackReady;
}

void Player::attack(Enemy *target)
{
    if (!attackReady || target == nullptr || bulletData.empty())
        return;

    // 计算玩家和目标的中心点（使用double提高精度）
    double playerCenterX = x + (double)width / 2.0;
    double playerCenterY = y + (double)height / 2.0;
    double targetCenterX = target->getX() + (double)target->getWidth() / 2.0;
    double targetCenterY = target->getY() + (double)target->getHeight() / 2.0;

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

    double Dx = (dx / distance);
    double Dy = (dy / distance);
    // 创建子弹
    int num = bulletData.size();
    float baseAngle = std::atan2(Dy,Dx);
    float spreadAngle = (M_PI / 6);
    if(num>1)spreadAngle/=(num-1);
    for(int i=(-1)*num+1;i<num;i++){
        // 归一化方向向量并乘以子弹速度（保持double精度）
        float angle = baseAngle + spreadAngle * i;
        Dx = std::cos(angle);
        Dy = std::sin(angle);


    Bullet *bullet = Bullet::createBullet(
        bulletData[std::abs(i)],
        static_cast<int>(playerCenterX),
        static_cast<int>(playerCenterY),
            Dx*bulletData[std::abs(i)].dmoveDis,
        Dy*bulletData[std::abs(i)].dmoveDis,
        target,
        damage);

    // 设置子弹的敌人列表和障碍物列表
    bullet->setEnemiesList(enemiesList);
    bullet->setObstaclesList(obstaclesList);

    bullet->startMove();
    emit createBullet(bullet);}

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

int Player::updateAnimation(){
    pictureIndex[curState].curIndex=(pictureIndex[curState].curIndex+1)%pictureIndex[curState].maxCnt;
    return pictureIndex[curState].curIndex;
}
