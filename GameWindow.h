#pragma once
#include <QWidget>
#include <QPushButton>
#include <Qpainter>
#include <QPen>
#include <QBrush>
#include <QResizeEvent>
#include <QPalette>
#include <QColor>
#include "Enemy.h"
#include "Bullet.h"
#include "Player.h"
#include "Data.h"
#include <QKeyEvent>
#include <QSet>
#include <QTimer>
#include <map>

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

    // 定时器
    QTimer *attackCD;      // 攻击冷却定时器
    QTimer *movementTimer; // 用于处理移动的定时器
    QTimer *deadTimer;     // 用于处理死亡的定时器
    QTimer *updateTimer;   // 用于处理更新的定时器

    // 敌人生成定时器（动态创建）
    std::vector<QTimer *> enemySpawnTimers;
    std::vector<int> enemySpawnCounts; // 每种敌人的生成计数

    GameData *gameData;       // 游戏数据指针
    Player *player = nullptr; // 玩家指针

    // 摄像机系统
    int cameraOffsetX = 0; // 摄像机X偏移
    int cameraOffsetY = 0; // 摄像机Y偏移
    int mapWidth = 0;      // 地图实际宽度
    int mapHeight = 0;     // 地图实际高度
    void updateCamera();   // 更新摄像机位置

    // 游戏对象列表
    QList<QRect> obstacles = {};       // 障碍物列表
    QList<Bullet *> bullets = {};      // 子弹列表
    QList<Enemy *> enemies = {};       // 敌人指针列表
    QList<Enemy *> enemyDeadList = {}; // 敌人死亡列表

    // 图片资源
    QPixmap mapCache;                 // 地图缓存
    QPixmap floorTile;                // 地板瓦片
    QPixmap obstacleTile;             // 墙壁瓦片
    std::vector<QPixmap> playerImage; // 玩家图片
    std::vector<QPixmap> enemyImages; // 敌人图片（统一管理）
    std::vector<QPixmap> bulletImage; // 子弹图片

    QPixmap loadAndProcessImage(const std::string &imagePath, int width, int height);

    void initPicture();
    void createMapCache(std::vector<std::vector<int>> *grid); // 创建地图缓存

    // 事件处理
    void paintEvent(QPaintEvent *event) override;    // 重写paintEvent事件
    void keyPressEvent(QKeyEvent *event) override;   // 重写keyPressEvent事件
    void keyReleaseEvent(QKeyEvent *event) override; // 重写keyReleaseEvent事件

    // 游戏逻辑
    void handleMovement(int step, int diagonalStep); // 处理移动的函数
    void generateEnemy(int enemyConfigIndex);        // 生成敌人的函数
    void handlePlayerAttack();                       // 处理攻击的函数
    void handleEnemyDead();                          // 处理敌人死亡的函数
    void handlePlayerDead();                         // 处理玩家死亡的函数

    // 图片索引映射（从路径到索引）
    std::map<std::string, int> imagePathToIndex;

signals:
    void gameFinished(); // 游戏结束信号
};
