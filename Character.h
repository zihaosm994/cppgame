#ifndef CHARACTER_H
#define CHARACTER_H
#include <string>
#include <QMap>
#include <QKeyEvent>
#include <QPixmap>
#include <QTimer>
#include <QObject>
#include <vector>
#include <utility> 
//前向声明
class Character;
class Enemy;
class Bullet;
enum BulletType{
    IceBall,//冰球
    RockBall,//石头
    FireBall//火球
};


class Weapon;
class Player;



class Character : public QObject
{
    Q_OBJECT
public:
    enum Direction //角色方向
    { 
        Up, Down, Left, Right
    };
    ~Character();
    Character(int x, int y);
    Character();
    int getX() const;//获取坐标X
    int getY() const;// 获取坐标Y
    void setPosition(int x,int y);//设置坐标
    int getCurrentImage() const;//获取当前图片编号
    void updateAnimation(Direction dir);//更新动画
    void setHP(int hp);//设置血量
    int getHp()const;//获取血量
    void setFrameCnt(int frameCnt);
protected:
    int x, y;// 角色的坐标
    int currentImage;// 当前图片编号
    int currentFrameIndex=0;// 当前帧索引
    int FRAME_CNT = 8;//每8帧更新一次
    int frameCnt=0;// 用于计数的变量
    int hp=0;// 角色的血量
};
    
class Player : public Character
{   
    Q_OBJECT
public:
    enum PlayerRole//角色
    {
    Roxy
    };
    Player(PlayerRole role,QPoint pos);
    ~Player();
    const int width = 30; // 角色的宽度
    const int height =45; // 角色的高度
    Weapon* getWeapon() const; // 获取武器指针
    void setWeapon(Weapon* weapon); // 设置武器指针
private:
    PlayerRole role;   // 角色类型
    Weapon* weapon=nullptr; // 武器指针，用于指向武器对象;
};
class Enemy : public Character {
    Q_OBJECT
public:
    enum EnemyType  //敌人类型
    {
        Goblin,
        UndeadMage,
        MagicCircle,
        drop_healing,
        drop_attack,
        drop_speed,
    };
    Enemy(EnemyType type,QPoint pos);
    ~Enemy();
    const int width = 40; // 角色的宽度
    const int height = 40; // 角色的高度
    static Player* attackTarget;// 攻击目标
    static std::vector<std::vector<int>> grid;// 地图

    void setSpeedF(std::pair<int,int>* speedF); // 设置移动频率
    void setDamage(int damage); // 设置攻击伤害  
    EnemyType getEnemyType() const; // 获取敌人类型
    void setAttackCD(int CD); // 设置攻击cd//UndeadMage专用
    void setObstaclesList(QList<QRect>* obstaclesList); // 设置障碍物列表指针
    void stopMove();//停止移动
private:
    
    EnemyType type;// 敌人类型
    QTimer *AIMoveTimer; // 用于处理AI移动的定时器
    int AIMoveF=48;   //定时触发AI移动
    QTimer *pathTimer;    // 用于生成路径的定时器
    int pathF=500;   //定时触发路径生成
    QTimer *moveSpeedChangeTimer; // 用于处理移动速度改变的定时器
    void changeMoveSpeed(); // 改变移动速度的函数
    void AIMove();  //AI移动
    int moveStep=4; //移动步长
    std::vector<std::pair<int,int>> path={};//路径
    void getNewPath();//获取新路径
    int pathIndex=0;//路径索引
    QList<QRect>* obstaclesList=nullptr;//障碍物列表

    int damage=30;//攻击伤害
    //UndeadMage专用
    QTimer *attackTimer; // 用于处理攻击的定时器
    void launchAttack(); //发射
    int attackCD=1000; //攻击cd
    std::pair<int,int> speedF={30,60};//移动频率
signals:
    void createBullet(Bullet *bullet);//创建子弹的信号

};

class Weapon {
public:
    Weapon(int level,BulletType bulletType); // 构造函数，接受伤害和范围作为参数
    ~Weapon();
    int getDamage() const; // 获取武器的伤害
    int getRange() const; // 获取武器的范围
    int getCoolDownCD() const; // 获取武器的冷却时间
    BulletType getBulletType() const; // 获取子弹类型
private:
    int level; // 武器的等级
    int damage; // 武器的伤害
    int range; // 武器的范围
    int coolDownCD=100; // 武器的冷却时间
    BulletType bulletType; // 子弹类型
};

class Bullet: public Character{
    Q_OBJECT
public:
    Bullet(); 
    ~Bullet();
    const int width = 10; // 子弹的宽度
    const int height = 10;// 子弹的高度
    int targetWidth=30,targetHeight=45;//目标宽度和高度
    void move(); // 移动函数
    void setType(BulletType type); // 设置子弹类型
    void startMove(int spped=10);// 启动移动
    void setInformation(int damage,QList<Enemy*>* enemiesList=nullptr,QList<QRect>* obstaclesList=nullptr);// 设置子弹信息
    BulletType getBulletType() const; // 获取子弹类型
    bool isUsed=false;//是否使用(子弹池)
    static std::vector<Bullet*>bulletsPool;// 子弹池
    static Bullet* createBullet(BulletType type,int x, int y,int dx,int dy,Character* target);// 创建子弹(子弹池)
    static void deleteBullet(Bullet* bullet);// 删除子弹(子弹池)
    static void initBulletsPool(int totalCount);// 初始化子弹池
private:
    BulletType type;
    int dx=0, dy=0; // 子弹的移动速度
    QTimer *moveTimer; // 用于处理移动的定时器
    Character* target=nullptr;//目标
    
    QList<Enemy*>* enemiesList=nullptr;//敌人列表
    QList<QRect>* obstaclesList=nullptr;//障碍物列表
    int damage=0;//伤害
signals:
    void hitSth();//击中信号
};

#endif