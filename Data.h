#ifndef DATA_H
#define DATA_H

#include <QTextStream>
#include <vector>
#include <string>
#include <QPoint>
#include <utility>

// 子弹数据结构
struct BulletData
{
    int bulletID;
    int width;                           // 宽度
    int height;                          // 高度
    std::vector<std::string> imagePaths; // 图片路径列表（用于动画）
    double damageMultiplier;             // 伤害倍率
    int moveSpeed;                       // 移动速度（定时器间隔）
    int dmoveDis;                         // 移动间距
};

// 敌人数据结构
struct EnemyData
{
    int enemyID;                       // 敌人类型（用整数表示）
    int hp;                              // 血量
    int damage;                          // 伤害
    int width;                           // 宽度
    int height;                          // 高度
    std::vector<std::string> rightWalkPaths; // 右走动画图片路径
    std::vector<std::string> leftWalkPaths;  // 左走动画图片路径

    // 移动相关
    bool canMove;               // 是否可以移动
    int moveStep;               // 移动步长
    std::pair<int, int> speedF; // 移动频率范围
    int pathUpdateFreq;         // 路径更新频率

    // 攻击相关
    BulletData * bulletData; //子弹数据
    bool hasMeleeAttack;  // 是否有近战攻击
    bool hasRangedAttack; // 是否有远程攻击
    bool bulletWithoutObstacles; //远程攻击子弹能否穿墙
    int attackCD;         // 攻击冷却时间
};

// 玩家数据结构
struct PlayerData
{
    int hp;             // 血量
    int width;          // 宽度
    int height;         // 高度
    int moveStep;       // 移动步长
    QPoint generatePos; // 生成位置

    // 图片路径管理（支持多方向动画）
    std::vector<std::string> rightWalkPaths; // 右走动画图片路径
    std::vector<std::string> leftWalkPaths;  // 左走动画图片路径
    std::vector<std::string> upWalkPaths;    // 上走动画图片路径
    std::vector<std::string> downWalkPaths;  // 下走动画图片路径（可选）
    // 攻击相关
    int damage;         // 基础伤害
    int attackCD;       // 攻击冷却时间
    int attackRange;    // 攻击范围
    std::vector<BulletData> bulletData; //使用的子弹数据
};

// 敌人生成配置
struct EnemySpawnConfig
{
    EnemyData enemyData;                // 敌人数据
    std::vector<QPoint> spawnPositions; // 生成位置列表
    int totalNum;                       // 总数量
    int spawnFreq;                      // 生成频率（毫秒）
};

struct ObstacleData
{
    QPoint pos;
    int width;
    int height;
    std::string imagePath;
};
// 地图配置数据
struct MapData
{
    std::string backgroundImgPath;
    std::vector<ObstacleData> obstacles;
    int mapWidth;
    int mapHeight;
};

struct npcData{
    int npcID;
    std::string imagePath;
    int width1,height1;
    std::string greetingImagePath;
    int width2,height2;
    QPoint pos;

    // d-表示给玩家的增益
    int dhp;
    int ddamage;
    int dattackRange;
    int dattackCD;
    int dmoveStep;
    BulletData * bulletData; // 能够多一种子弹
};

struct PlotData{
    std::vector<std::pair<std::string,std::string>> imagePath_texts;
};

// 游戏数据类
class GameData
{
public:
    MapData mapData;                                // 地图数据
    PlayerData playerData;                           // 玩家数据
    std::vector<BulletData> bulletDataList;          // 子弹数据列表
    std::vector<EnemySpawnConfig> enemySpawnConfigs; // 敌人生成配置
    std::vector<npcData> npcDatas;                  //npc数据
    GameData() {}
};

// ==================== 测试数据对象 ====================
// 提供默认的测试数据供外部调用

namespace TestData
{
    // 创建默认子弹数据
    inline BulletData createDefaultBulletData(int bulletId = 0)
    {
        BulletData data;
        data.bulletID = bulletId;
        data.width = 20;
        data.height = 20;

        // 根据bulletId设置图片路径
        if (bulletId == 0)
            data.imagePaths = {":/images/IceBall.png"};
        else if (bulletId == 1)
            data.imagePaths = {":/images/RockBall.png"};
        else if (bulletId == 2)
            data.imagePaths = {":/images/FireBall.png"};
        else
            data.imagePaths = {":/images/IceBall.png"}; // 默认
        data.damageMultiplier = 1.0;
        data.moveSpeed = 10;
        data.dmoveDis = 80;
        return data;
    }

    // 创建默认敌人数据
    inline EnemyData createDefaultEnemyData(int enemyType = 0)
    {
        EnemyData data;
        data.enemyID = enemyType;
        data.hp = 100;
        data.damage = 10;
        data.width = 80;
        data.height = 80;

        // 设置敌人图片路径（4帧动画）
        data.rightWalkPaths = {
            ":/images/goblin1.png",
            ":/images/goblin2.png"};
        data.leftWalkPaths={
            ":/images/goblin3.png",
            ":/images/goblin4.png"};

        data.canMove = true;
        data.moveStep = 2;
        data.speedF = {100, 200};
        data.hasMeleeAttack = true;
        data.hasRangedAttack = false;
        data.attackCD = 1000;
        BulletData bull = createDefaultBulletData();
        data.bulletData=&bull;
        data.bulletWithoutObstacles=false;
        return data;
    }

    // 创建默认玩家数据
    inline PlayerData createDefaultPlayerData()
    {
        PlayerData data;
        data.hp = 3000;
        data.width = 60;
        data.height = 90;
        data.moveStep = 3;
        data.generatePos = QPoint(100, 100);

        // 设置玩家图片路径
        data.rightWalkPaths = {":/images/roxy/RightWalk1.png", ":/images/roxy/RightWalk2.png"};
        data.leftWalkPaths = {":/images/roxy/LeftWalk1.png", ":/images/roxy/LeftWalk2.png"};
        data.upWalkPaths = {":/images/roxy/backWalk1.png", ":/images/roxy/backWalk2.png"};
        data.downWalkPaths = {":/images/roxy/RightWalk1.png", ":/images/roxy/RightWalk2.png"};

        data.damage = 20;
        data.attackCD = 200;
        data.attackRange = 600;
        data.bulletData.push_back(createDefaultBulletData(0));
        data.bulletData.push_back(createDefaultBulletData(1));
        data.bulletData.push_back(createDefaultBulletData(2));
        return data;
    }

    inline std::vector<PlotData> * createDefaultPlotData(){
        std::vector<PlotData> * plotData = new std::vector<PlotData>();
        plotData->push_back({
            PlotData()
        });
        (*plotData)[0].imagePath_texts.push_back({
            ":/images/plot_2BG.png","......................."
        });
        (*plotData)[0].imagePath_texts.push_back({
            ":/images/plot_2BG.png","......................."
        });
        (*plotData)[0].imagePath_texts.push_back({
            ":/images/plot_2BG.png","......................."
        });
        return plotData;
    }
    // 创建默认敌人生成配置
    inline EnemySpawnConfig createDefaultEnemySpawnConfig(int enemyType = 0)
    {
        EnemySpawnConfig config;
        config.enemyData = createDefaultEnemyData(enemyType);
        config.spawnPositions = {QPoint(800, 800), QPoint(1200, 800), QPoint(800, 600)};
        config.totalNum = 20;
        config.spawnFreq = 100;
        return config;
    }
    // 创建默认游戏数据
    inline GameData* createDefaultGameData()
    {
        GameData *data=new GameData();
        data->mapData.backgroundImgPath=std::string(":/images/bg.png");
        data->mapData.mapWidth=2048;
        data->mapData.mapHeight=1042;
        data->mapData.obstacles.push_back({
                                           {200,200},
                                           100,100,

                                           ":/images/Obstacle.png"
        });
        // 使用基础地图
        data->playerData = createDefaultPlayerData();

        // 添加子弹数据
        data->bulletDataList.push_back(createDefaultBulletData(0)); // IceBall
        data->bulletDataList.push_back(createDefaultBulletData(1)); // RockBall
        data->bulletDataList.push_back(createDefaultBulletData(2)); // FireBall

        // 添加敌人配置
        data->enemySpawnConfigs.push_back(createDefaultEnemySpawnConfig(0));
        data->npcDatas.push_back({
            0,
            ":/images/RoxyWhole.png",
            50,50,
            ":/images/MagicCircle.png",
            40,40,
            {400,400},
            100,
            0,
            0,
            0,
            0,
            nullptr
        });
        return data;
    }
}

#endif
