#ifndef DATA_H
#define DATA_H

#include <QTextStream>
#include <vector>
#include <string>
#include <QPoint>
#include <utility>

// 基础地图数据声明
extern const std::vector<std::vector<int>> basicGrid;

// 子弹数据结构
struct BulletData
{
    int bulletId;                        // 子弹ID（用于区分不同子弹类型）
    int width;                           // 宽度
    int height;                          // 高度
    std::vector<std::string> imagePaths; // 图片路径列表（用于动画）
    int frameCnt;                        // 动画帧计数
    double damageMultiplier;             // 伤害倍率
    int moveSpeed;                       // 移动速度（定时器间隔）
    int targetWidth;                     // 目标宽度（用于碰撞检测）
    int targetHeight;                    // 目标高度（用于碰撞检测）

    BulletData()
        : bulletId(0), width(20), height(20),
          frameCnt(8), damageMultiplier(1.0), moveSpeed(10),
          targetWidth(60), targetHeight(90) {}
};

// 敌人数据结构
struct EnemyData
{
    int enemyType;                       // 敌人类型（用整数表示）
    int hp;                              // 血量
    int damage;                          // 伤害
    int width;                           // 宽度
    int height;                          // 高度
    std::vector<std::string> imagePaths; // 图片路径列表（用于动画）
    int frameCnt;                        // 动画帧计数

    // 移动相关
    bool canMove;               // 是否可以移动
    int moveStep;               // 移动步长
    std::pair<int, int> speedF; // 移动频率范围
    int pathUpdateFreq;         // 路径更新频率

    // 攻击相关
    bool hasMeleeAttack;  // 是否有近战攻击
    bool hasRangedAttack; // 是否有远程攻击
    int attackCD;         // 攻击冷却时间
    int bulletId;         // 子弹ID（如果有远程攻击）
    double bulletSpeed;   // 子弹速度（使用double提高精度）

    EnemyData()
        : enemyType(0), hp(100), damage(10), width(80), height(80),
          frameCnt(8), canMove(true), moveStep(8), speedF({30, 60}),
          pathUpdateFreq(500), hasMeleeAttack(true), hasRangedAttack(false),
          attackCD(1000), bulletId(0), bulletSpeed(16.0) {}
};

// 玩家数据结构
struct PlayerData
{
    int playerId;       // 玩家ID（用于区分不同角色）
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
    int frameCnt;                            // 动画帧计数

    // 攻击相关
    int damage;         // 基础伤害
    int attackCD;       // 攻击冷却时间
    int attackRange;    // 攻击范围
    int bulletId;       // 子弹ID
    double bulletSpeed; // 子弹速度

    PlayerData()
        : playerId(0), hp(3000), width(60), height(90), moveStep(3),
          generatePos(0, 300), frameCnt(8), damage(20), attackCD(200),
          attackRange(600), bulletId(0), bulletSpeed(10.0) {}
};

// 敌人生成配置
struct EnemySpawnConfig
{
    EnemyData enemyData;                // 敌人数据
    std::vector<QPoint> spawnPositions; // 生成位置列表
    int totalNum;                       // 总数量
    int spawnFreq;                      // 生成频率（毫秒）

    EnemySpawnConfig()
        : totalNum(0), spawnFreq(1000) {}
};

// 武器数据结构
struct WeaponData
{
    int weaponId;                        // 武器ID（用于区分不同武器类型）
    std::vector<std::string> imagePaths; // 武器图片路径列表
    double damageMultiplier;             // 伤害倍率（实际伤害 = 基础伤害 * 倍率）

    WeaponData()
        : weaponId(0), damageMultiplier(1.0) {}
};

// 地图配置数据
struct MapConfig
{
    std::string floorTilePath;    // 地板瓦片图片路径
    std::string obstacleTilePath; // 障碍物瓦片图片路径
    int tileWidth;                // 瓦片宽度
    int tileHeight;               // 瓦片高度

    MapConfig()
        : floorTilePath(":/images/floor.png"),
          obstacleTilePath(":/images/Obstacle.png"),
          tileWidth(10), tileHeight(10) {}
};

// 剧情数据类
class PlotData
{
public:
    // 构造函数
    PlotData() : plotId(0) {}

    PlotData(int id, const std::vector<std::string> &texts, const std::vector<std::string> &images, const std::string &bgPath)
        : plotId(id), textList(texts),npcimageList(images) backgroundPath(bgPath) {}

    // Getter方法
    int getPlotId() const { return plotId; }
    const std::vector<std::string> &getTextList() const { return textList; }
    std::vector<std::string> *getTextListPtr() { return &textList; }
    const std::string &getBackgroundPath() const { return backgroundPath; }
    std::string getBackgroundPathCopy() const { return backgroundPath; }

    // Setter方法
    void setPlotId(int id) { plotId = id; }
    void setTextList(const std::vector<std::string> &texts) { textList = texts; }
    void setnpcimageList(const std::vector<std::string> &images) { npcimageList = images; }
    void setBackgroundPath(const std::string &path) { backgroundPath = path; }
    void addText(const std::string &text) { textList.push_back(text); }
    void addnpcimage(const std::string &image) { npcimageList.push_back(image);}
    void clearText() { textList.clear(); }
    void clearimages() {npcimageList.clear();}

private:
    int plotId;                           // 剧情ID
    std::vector<std::string> textList;    // 剧情文本列表
    std::vector<std::string> npcimageList; // npc图片路径列表
    std::string backgroundPath;       // 背景图片路径
};

// 游戏数据类
class GameData
{
public:
    std::vector<std::vector<int>> grid;              // 地图网格
    MapConfig mapConfig;                             // 地图配置
    PlayerData playerData;                           // 玩家数据
    std::vector<BulletData> bulletDataList;          // 子弹数据列表
    std::vector<EnemySpawnConfig> enemySpawnConfigs; // 敌人生成配置

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
        data.bulletId = bulletId;
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

        data.frameCnt = 8;
        data.damageMultiplier = 1.0;
        data.moveSpeed = 10;
        data.targetWidth = 60;
        data.targetHeight = 90;
        return data;
    }

    // 创建默认敌人数据
    inline EnemyData createDefaultEnemyData(int enemyType = 0)
    {
        EnemyData data;
        data.enemyType = enemyType;
        data.hp = 100;
        data.damage = 10;
        data.width = 80;
        data.height = 80;

        // 设置敌人图片路径（4帧动画）
        data.imagePaths = {
            ":/images/goblin1.png",
            ":/images/goblin2.png",
            ":/images/goblin3.png",
            ":/images/goblin4.png"};

        data.frameCnt = 8;
        data.canMove = true;
        data.moveStep = 8;
        data.speedF = {30, 60};
        data.pathUpdateFreq = 500;
        data.hasMeleeAttack = true;
        data.hasRangedAttack = false;
        data.attackCD = 1000;
        data.bulletId = 0;
        data.bulletSpeed = 16.0;
        return data;
    }

    // 创建默认玩家数据
    inline PlayerData createDefaultPlayerData()
    {
        PlayerData data;
        data.playerId = 0;
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

        data.frameCnt = 8;
        data.damage = 20;
        data.attackCD = 200;
        data.attackRange = 600;
        data.bulletId = 0;
        data.bulletSpeed = 10.0;
        return data;
    }

    // 创建默认武器数据
    inline WeaponData createDefaultWeaponData()
    {
        WeaponData data;
        data.weaponId = 0;
        data.imagePaths = {":/images/weapon_default.png"};
        data.damageMultiplier = 1.0;
        return data;
    }

    // 创建默认敌人生成配置
    inline EnemySpawnConfig createDefaultEnemySpawnConfig(int enemyType = 0)
    {
        EnemySpawnConfig config;
        config.enemyData = createDefaultEnemyData(enemyType);
        config.spawnPositions = {QPoint(800, 800), QPoint(1200, 800), QPoint(800, 1200)};
        config.totalNum = 20;
        config.spawnFreq = 2000;
        return config;
    }

    // 创建默认游戏数据
    inline GameData createDefaultGameData()
    {
        GameData data;
        data.grid = basicGrid; // 使用基础地图
        data.playerData = createDefaultPlayerData();

        // 添加子弹数据
        data.bulletDataList.push_back(createDefaultBulletData(0)); // IceBall
        data.bulletDataList.push_back(createDefaultBulletData(1)); // RockBall
        data.bulletDataList.push_back(createDefaultBulletData(2)); // FireBall

        // 添加敌人配置
        data.enemySpawnConfigs.push_back(createDefaultEnemySpawnConfig(0));

        return data;
    }
}

#endif