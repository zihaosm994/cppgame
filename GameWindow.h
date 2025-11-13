#pragma once
#include <QWidget>
#include <QPushButton>
#include <Qpainter>
#include <QPen>
#include <QBrush>
#include <QResizeEvent>
#include <QPalette>
#include <QColor>
#include "Character.h"
#include "Data.h"
#include <QKeyEvent>
#include <QSet>
#include <QTimer>
#include <QSoundEffect>

class GameWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

    // 设置游戏数据并初始化游戏
    void setGameData(GameData *data);
    // 重置游戏状态
    void reset();
    // 停止游戏（停止所有定时器和音乐）
    void stopGame();
    // 清理按键状态（在返回主菜单时调用）
    void clearKeyState();

    QList<QRect> getObstaclesList() const { return obstacles; };
    QList<Enemy *> getEnemiesList() const { return enemies; };

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSet<int> keyPressed;

    QTimer *attackCD;      // 攻击冷却定时器
    QTimer *movementTimer; // 用于处理移动的定时器
    QTimer *goblinTimer;   // 用于生成敌人的定时器
    QTimer *undeadMageTimer;
    int goblinCount = 0; // 敌人数量
    int undeadMageCount = 0;
    QTimer *deadTimer;   // 用于处理死亡的定时器
    QTimer *updateTimer; // 用于处理更新的定时器
    GameData *gameData;  // 游戏数据指针

    Player *player = nullptr; // 玩家指针

    QList<QRect> obstacles = {};       // 障碍物列表
    QList<Bullet *> bullets = {};      // 子弹列表
    QList<Enemy *> enemies = {};       // 敌人指针列表
    QList<Enemy *> enemyDeadList = {}; // 敌人死亡列表

    QPixmap mapCache;                  // 地图缓存
    QPixmap floorTile;                 // 地板瓦片
    QPixmap obstacleTile;              // 墙壁瓦片
    std::vector<QPixmap> playerImage;  // 0右1,1右2,2左1,3左2,4上1,5上2
    std::vector<QPixmap> goblinImage;  // 敌人图片
    QPixmap undeadMageImage;           // 敌人图片
    std::vector<QPixmap> bulletImage;  // 0Ice,1Rock,2Fire
    QPixmap magicCircleImage;          // 转移魔法阵图片
    QList<QRect> magicCircleList = {}; // 魔法阵列表
    QList<Enemy *> dropList = {};      // 掉落物列表
    std::vector<QPixmap> dropImage;    // 0Health,1Attack,2Speed
    QPixmap loadAndProcessImage(const std::string &imagePath, int width, int height);
    QTimer *updateDropTimer; // 用于处理掉落物的定时器

    QSoundEffect *bgmSound = nullptr;

    void initPicture();
    void createMapCache(std::vector<std::vector<int>> *grid); // 创建地图缓存

    void paintEvent(QPaintEvent *event) override;                            // 重写paintEvent事件
    void keyPressEvent(QKeyEvent *event) override;                           // 重写keyPressEvent事件
    void keyReleaseEvent(QKeyEvent *event) override;                         // 重写keyReleaseEvent事件
    void handleMovement(int step, int diagonalStep);                         // 处理移动的函数
    void generateEnemy(Enemy::EnemyType type, std::vector<QPoint> *bornPos); // 生成敌人的函数
    void handlePlayerAttack();                                               // 处理攻击的函数
    void handleEnemyDead();                                                  // 处理敌人死亡的函数
    void handlePlayerDead();                                                 // 处理玩家死亡的函数
    void handleDrop(Enemy *drop);                                            // 处理掉落物的函数
    void attackByDrop();                                                     // 处理掉落物攻击的函数
signals:
    void pass_1();       // 第一关通关信号
    void pass_2();       // 第二关通关信号
    void gameFinished(); // 游戏结束信号（统一的）
};
