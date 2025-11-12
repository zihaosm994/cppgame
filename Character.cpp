#include "Character.h"
#include <QImage>
#include <QBitmap>
#include <QObject>
#include <QRandomGenerator>
#include <QMovie>
#include "findpath.h"

//Character类只实现基础的动画和移动
Character::Character() : x(0), y(0) {};
Character::Character(int x, int y) : x(x), y(y) {};
Character::~Character() {};
int Character::getX() const { return x; }
int Character::getY() const { return y; }
void Character::setHP(int hp) { this->hp = hp; }
int Character::getHp() const { return hp; }
int Character::getCurrentImage() const { return currentImage; }
void Character::setFrameCnt(int frameCnt) { this->FRAME_CNT = frameCnt; }
void Character::setPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}
void Character::updateAnimation(Direction dir)
{
    frameCnt = (frameCnt + 1) % FRAME_CNT;
    if (frameCnt == 0)
    {
        if(dir==Up)currentImage=4+(++currentFrameIndex)%2;
        else if(dir==Left)currentImage=2+(++currentFrameIndex)%2;
        else if(dir==Right)currentImage=0+(++currentFrameIndex)%2;
    }
}

// 玩家类的实现,目前只有Roxy
Player::Player(PlayerRole role, QPoint pos) : role(role), Character(pos.x(), pos.y())
{
    if(role==Roxy){
        currentImage=4;
    }
}
Player::~Player() {};
void Player::setWeapon(Weapon *weapon) { this->weapon = weapon; }
Weapon *Player::getWeapon() const { return weapon; }

// 敌人类的实现,敌人类也用于实现转移魔法阵和掉落物系统
Player *Enemy::attackTarget = nullptr;
std::vector<std::vector<int>> Enemy::grid;//寻路需要的静态变量
Enemy::Enemy(EnemyType type, QPoint pos) : type(type), Character(pos.x(), pos.y())
{

    currentImage=0;
    if (type == UndeadMage)//不死族魔法师需要定时发动攻击
    {
        attackTimer = new QTimer(this);
        connect(attackTimer, &QTimer::timeout, this, &Enemy::launchAttack);
        attackTimer->start(attackCD);
    }
    //通用的移动设置
    moveSpeedChangeTimer = new QTimer(this); 
    connect(moveSpeedChangeTimer, &QTimer::timeout, this, &Enemy::changeMoveSpeed);
    moveSpeedChangeTimer->start(500); 
    pathTimer = new QTimer(this);      
    AIMoveTimer = new QTimer(this);    
    connect(pathTimer, &QTimer::timeout, this, &Enemy::getNewPath);
    pathTimer->start(pathF);
    connect(AIMoveTimer, &QTimer::timeout, this, &Enemy::AIMove);
    //动画设置
    if(type==drop_healing){
        currentImage=0;
    }else if(type==drop_attack){
        currentImage=1;
    }else if(type==drop_speed){
        currentImage=2;
    }
    setFrameCnt(1);
}
Enemy::~Enemy()
{
    this->disconnect();
}
//敌人移动不定速, 在移动过程中不会黏在一块
void Enemy::changeMoveSpeed()
{
    AIMoveF = QRandomGenerator::global()->bounded(speedF.first, speedF.second);
    AIMoveTimer->start(AIMoveF);                                                
}
//本来想要实现启发式搜索,但运行起来太卡顿且因碰撞检测问题并没有发挥优势,遂放弃
void Enemy::getNewPath()
{
    if (attackTarget == nullptr)
        return;
    path.clear(); // 清空路径
    path = AStar(grid, std::make_pair(x / moveStep, y / moveStep), std::make_pair(attackTarget->getX() / moveStep, attackTarget->getY() / moveStep));
    pathIndex = 0;
}
//移动过程中检测碰撞
void Enemy::AIMove()
{
    if (path.empty() || pathIndex == path.size() - 1)
    {
        getNewPath();
        pathIndex = 0;
        return;
    }
    pathIndex++;
    bool collision = false;
    QRect enemyRect(path[pathIndex].first * moveStep, path[pathIndex].second * moveStep, width, height);
    QRect targetRect(attackTarget->getX(), attackTarget->getY(), attackTarget->width, attackTarget->height);
    if (enemyRect.intersects(targetRect))
    {                                                        
        attackTarget->setHP(attackTarget->getHp() - damage);//掉落物和魔法阵攻击为0
        return;
    }
    if (obstaclesList!= nullptr){
        for (const QRect &obstacle : *obstaclesList) { 
            if (enemyRect.intersects(obstacle)) {      
                collision = true;                      
                break;                                 
            }
        }
    }
    if (collision){
        getNewPath();
        pathIndex = 0;
        return;
    }
    setPosition(path[pathIndex].first * moveStep, path[pathIndex].second * moveStep);
    if (type == Goblin)
    {

        if (x < attackTarget->getX())
            updateAnimation(Right);
        else
            updateAnimation(Left);
    }
}
//为不死族魔法师实现攻击
void Enemy::launchAttack()
{
    if (hp <= 0)
    {
        attackTimer->stop();
        return;
    }
    if (attackTarget == nullptr)
        return;                               
    int dx = attackTarget->getX() + attackTarget->width/2 - (x+width/2);    
    int dy = attackTarget->getY() + attackTarget->height/2 - (y+width/2); 
    double distance = sqrt(dx * dx + dy * dy); 
    dx = static_cast<int>((dx / distance) * 8); 
    dy = static_cast<int>((dy / distance) * 8);

    Bullet *bullet =Bullet::createBullet(BulletType::FireBall, x + width / 2, y + height / 2, dx, dy, attackTarget);
    bullet->setInformation(damage);     
    bullet->startMove(10);
    emit createBullet(bullet);       
}
void Enemy::setSpeedF(std::pair<int, int> *speedF) {
     this->speedF = *speedF; 
     if(this->speedF.first==20){
        setFrameCnt(4);
     }
    } 
void Enemy::setDamage(int damage) { this->damage = damage; }                   
void Enemy::setAttackCD(int CD) { this->attackCD; }                            
void Enemy::setObstaclesList(QList<QRect> *obstaclesList)                     
{                                                                              
    this->obstaclesList = obstaclesList;
}
Enemy::EnemyType Enemy::getEnemyType() const { return type; }
void Enemy::stopMove() 
{ AIMoveTimer->stop(); 
pathTimer->stop();
moveSpeedChangeTimer->stop();}
// 武器类的实现
Weapon::Weapon(int level, BulletType bulletType) : level(level), bulletType(bulletType)
{
    if(bulletType==IceBall){
    switch (level)
    {
    case 1:
        damage = 20;
        coolDownCD = 200;
        break;
    case 2:
        damage = 30;
        coolDownCD = 150;
        break;
    case 3:
        damage = 60;
        coolDownCD = 100;
        break;
    }
    range=300;}
    else if(bulletType==RockBall){
        switch (level)
        {
         case 1:
        damage = 30;
        coolDownCD = 250;
        break;
    case 2:
        damage = 50;
        coolDownCD = 170;
        break;
    case 3:
        damage = 80;
        coolDownCD = 120;
        break;
    }
    range=300;
        }
    }

Weapon::~Weapon() {};
int Weapon::getRange() const { return range; }
int Weapon::getDamage() const { return damage; }
int Weapon::getCoolDownCD() const { return coolDownCD; }
BulletType Weapon::getBulletType() const { return bulletType; }

// 子弹类的实现
std::vector<Bullet*> Bullet::bulletsPool={};
Bullet::Bullet()
{
    moveTimer = new QTimer(this);                             
    connect(moveTimer, &QTimer::timeout, this, &Bullet::move); 
}
Bullet::~Bullet()
{
    delete moveTimer;
    this->disconnect();
};
void Bullet::move()
{
    if (target != nullptr)
    {
    //碰撞检测检测是否打到Target
    QRect bulletRect(x, y, width, height);
    QRect targetRect(target->getX(), target->getY(), 30, 45);
    if (bulletRect.intersects(targetRect))
    {
        target->setHP(target->getHp() - damage); 
        moveTimer->stop();   
        emit hitSth();       
        deleteBullet(this);
        return;
    }
}
    if (x + dx >= 800 || x + dx <= 0 || y + dy >= 600 || y + dy <= 0)
    {                        
        moveTimer->stop();   
        emit hitSth();       
        deleteBullet(this); 
        return;
    }
    else
        setPosition(x + dx, y + dy); 
    //如果是玩家,则检测敌人碰撞
    if (enemiesList != nullptr)
    {
        QRect bulletRect(x, y, width, height);
        for (auto enemy : *enemiesList)
        {
            QRect enemyRect(enemy->getX(), enemy->getY(), enemy->width, enemy->height);
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
    // 检测障碍物碰撞
    if (obstaclesList != nullptr)
    {
        QRect bulletRect(x, y, width, height); 
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
}
void Bullet::setInformation(int damage,QList<Enemy*>* enemiesList,QList<QRect>* obstaclesList){
    this->enemiesList=enemiesList;
    this->obstaclesList=obstaclesList;
    this->damage=damage; 
}
void Bullet::startMove(int spped) { moveTimer->start(spped); }
//子弹池辅助实现
BulletType Bullet::getBulletType() const { return type; } 
Bullet* Bullet::createBullet(BulletType type,int x, int y,int dx,int dy,Character* target){
    for (auto bullet : bulletsPool) {
        if(!bullet->isUsed){
            bullet->isUsed=true;
            bullet->setType(type);
            bullet->setPosition(x,y);
            bullet->target=target;
            bullet->dx=dx;
            bullet->dy=dy;
            return bullet;
        }
    }
    Bullet* newBullet=new Bullet();
    newBullet->setType(type);
    newBullet->isUsed=true;
    bulletsPool.push_back(newBullet);
    newBullet->setPosition(x,y);
    newBullet->target=target;
    newBullet->dx=dx;
    newBullet->dy=dy;
    return newBullet;
}

void Bullet::deleteBullet(Bullet* bullet){
    bullet->isUsed=false;
    bullet->moveTimer->stop();
}
void Bullet::initBulletsPool(int totalCount){
    for (int i = 0; i < totalCount; i++) {
        Bullet* bullet=new Bullet();
        bulletsPool.push_back(bullet);
    } 
}
void Bullet::setType(BulletType type){
    this->type=type;
    switch (type) {
    case BulletType::IceBall:
        currentImage=0;
        break; 
    case BulletType::RockBall:
        currentImage=1;
        break;
    case BulletType::FireBall:
        currentImage=2;
        break;
    }
}