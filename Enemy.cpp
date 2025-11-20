#include "Enemy.h"
#include "Bullet.h"
#include "Data.h"
#include <QRandomGenerator>
#include <cmath>

// 静态成员初始化
Player *Enemy::attackTarget = nullptr;
MapData *Enemy::mapData = nullptr;
std::vector<Enemy *> * Enemy::enemiesPool =nullptr;
QList<Enemy *> *Enemy:: allenemies = nullptr;
QList<QRect> * Enemy::obstacles = nullptr;

Enemy::Enemy(QObject *parent)
    : Character(parent)
{

    // 初始化定时器
    AIMoveTimer = new QTimer(this);
    moveSpeedChangeTimer = new QTimer(this);
    attackTimer = new QTimer(this);
    isDead=true;
    curState=Right;
}
void Enemy::initEnemiesPool(int num){
    enemiesPool = new std::vector<Enemy*>();
    for(int i=0;i<num;i++){
        Enemy *enemy=new Enemy;
        enemiesPool->push_back(enemy);
    }
}


Enemy* Enemy::createEnemy( EnemySpawnConfig & config){
    Enemy *res=nullptr;
    for(int i=0;i<enemiesPool->size();i++){
        if((*enemiesPool)[i]->isDead==true){
            res =(*enemiesPool)[i];
            break;
            }
    }
    if(res==nullptr){res= new Enemy;enemiesPool->push_back(res);}
    res->isDead=false;
    res->setHP(config.enemyData.hp);
    res->data=&(config.enemyData);
    int randomIndex = rand() % config.spawnPositions.size();
    QPoint pos = config.spawnPositions[randomIndex];
    res->setPosition(pos.x(),pos.y());
    // 如果可以移动，初始化移动相关定时器
    if (config.enemyData.canMove)
    {
        connect(res->moveSpeedChangeTimer, &QTimer::timeout, res, &Enemy::changeMoveSpeed);
        res->moveSpeedChangeTimer->start(500);
        connect(res->AIMoveTimer, &QTimer::timeout, res, &Enemy::AIMove);
        // 初始移动速度
        res->changeMoveSpeed();
    }
    // 如果有远程攻击，初始化攻击定时器
    if (config.enemyData.hasRangedAttack)
    {
        connect(res->attackTimer, &QTimer::timeout, res, [res](){
            res->launchRangedAttack();
        });
    }
    if(config.enemyData.hasMeleeAttack){
        connect(res->attackTimer,&QTimer::timeout,res,[res](){
            res->checkCollisionTarget(res->getX(),res->getY());
        });
    }
    res->attackTimer->start(res->data->attackCD);


    res->pictureIndex[Left].maxCnt=config.enemyData.leftWalkPaths.size();
    res->pictureIndex[Right].maxCnt=config.enemyData.rightWalkPaths.size();
    return res;
}

Enemy::~Enemy()
{
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
    if (moveSpeedChangeTimer)
        moveSpeedChangeTimer->stop();
}

void Enemy::startMove()
{
    if (data->canMove)
    {
        if (moveSpeedChangeTimer)
            moveSpeedChangeTimer->start(500);
        changeMoveSpeed();
    }
}


void Enemy::changeMoveSpeed()
{
    if (!data->canMove || !AIMoveTimer)
        return;

    int newSpeed = QRandomGenerator::global()->bounded(data->speedF.first, data->speedF.second);
    AIMoveTimer->start(newSpeed);
}

bool Enemy::checkCollisionTarget(int newX, int newY)
{

    QRect enemyRect(newX, newY, data->width, data->height);
    if (attackTarget != nullptr)
    {
        // 检查与目标的碰撞（使用敌人的完整矩形）
        QRect targetRect(attackTarget->getX(), attackTarget->getY(),
                         attackTarget->getWidth(), attackTarget->getHeight());
        if (enemyRect.intersects(targetRect))
        {
            // 近战攻击
            if (data->hasMeleeAttack)
            {
                attackTarget->setHP(attackTarget->getHp() - data->damage);
            }
            return true;
        }
    }
    return false;
}

bool Enemy::checkCollisionObstacle(int newX, int newY){
    QRect enemyRect(newX, newY, data->width, data->height);
    // 检查与障碍物的碰撞
    if (mapData != nullptr)
    {
        for (auto obstacle:mapData->obstacles)
        {
            QRect obstacleRect(obstacle.pos.x(),obstacle.pos.y(),obstacle.width,obstacle.height);
            if (enemyRect.intersects(obstacleRect))
            {
                return true;
            }
        }
    }

    return false;
}

void Enemy::AIMove()
{
    if(attackTarget==nullptr||isDead)return;
    if(x<attackTarget->getX())curState = Right;
    else curState = Left;
    if(!data->canMove)return;
    int step = data->moveStep;
    int dx = attackTarget->getX()-x;
    int dy = attackTarget->getY()-y;
    double dis = std::sqrt(double(dx*dx+dy*dy));
    if(dis>0.01){
        dx = dx /dis * step;
        dy = dy /dis * step;
    }
    int newX = static_cast<int>(x+dx);
    int newY = static_cast<int>(y+dy);
    int loop = 20;
    while(checkCollisionObstacle(newX,newY)&&loop--){
        int dir = QRandomGenerator::global()->bounded(0,10)%2;
        if(dir == 0){
            newX = static_cast<int>(newX-dy);
            newY = static_cast<int>(newY+dx);
        }
        else {
            newX = static_cast<int>(newX+dy);
            newY = static_cast<int>(newY-dx);
        }
    }
    if(loop>0)setPosition(newX,newY);
    avoidOthers();
}

void Enemy::launchRangedAttack()
{
    if (hp <= 0)
    {
        if (attackTimer)
            attackTimer->stop();
        return;
    }

    if (attackTarget == nullptr || data->bulletData == nullptr)
        return;

    // 计算敌人和目标的中心点（使用double提高精度）
    double enemyCenterX = x + data->width / 2.0;
    double enemyCenterY = y + data->height / 2.0;
    double targetCenterX = attackTarget->getX() + attackTarget->getWidth() / 2.0;
    double targetCenterY = attackTarget->getY() + attackTarget->getHeight()/ 2.0;

    // 计算方向向量（使用double）
    double dx = targetCenterX - enemyCenterX;
    double dy = targetCenterY - enemyCenterY;
    double distance = std::sqrt(dx * dx + dy * dy);

    // 避免除以零
    if (distance < 0.001)
        return;

    // 归一化方向向量并乘以子弹速度（保持double精度）
    double bulletDx = (dx / distance) * data->bulletData->dmoveDis;
    double bulletDy = (dy / distance) * data->bulletData->dmoveDis;

    // 使用新的Bullet接口创建子弹
    Bullet *bullet = Bullet::createBullet(
        *(data->bulletData),
        static_cast<int>(enemyCenterX),
        static_cast<int>(enemyCenterY),
        bulletDx*(*(data->bulletData)).dmoveDis,
        bulletDy*(*(data->bulletData)).dmoveDis,
        attackTarget,
        data->damage);
    bullet->setObstaclesList(obstacles);
    bullet->startMove();
    emit createBullet(bullet);
}

void Enemy::reset(){
    this->disconnect();   // 清空所有 signal 连接
    if (AIMoveTimer) AIMoveTimer->stop();
    if (moveSpeedChangeTimer) moveSpeedChangeTimer->stop();
    if (attackTimer) attackTimer->stop();
}

void Enemy::avoidOthers(){
    float pushX = 0.0f;
    float pushY = 0.0f;
    const float minDist = 30.0f;
    const float minDistSq = minDist * minDist;
    for( Enemy * other: *allenemies){
        if(other==this)continue;
        float dx = this->x - other->x;
        float dy = this->y - other->y;
        float distSq = dx * dx + dy * dy;
        if(distSq < minDistSq && distSq > 0.01f){
            float dist = std::sqrt(distSq);

            float overlap = (minDist - dist);

            dx /= dist;
            dy /= dist;

            pushX += dx * overlap *0.8f;
            pushY += dy * overlap *0.8f;
        }
    }
    if(checkCollisionObstacle(x+pushX,y+pushY))return;
    this -> x+= pushX;
    this -> y+= pushY;
}
