#include "GameWindow.h"
#include <QPoint>
#include <vector>
#include <QMessageBox>
#include <QDebug>
#include "findpath.h"

/*
简化的游戏窗口实现：
- 数据驱动：所有配置通过GameData传入
- 模板化：通用的游戏运行逻辑
- 无硬编码：不包含具体的敌人类型、武器类型等
- 核心流程：加载数据 -> 初始化定时器 -> 生成敌人 -> 处理玩家控制 -> 处理游戏结束
*/

GameWindow::GameWindow(QWidget *parent) : QWidget(parent), gameData(nullptr)
{
    // 设置固定大小
    setFixedSize(1200, 800);

    // 加载图片
    initPicture();

    // 初始化基础定时器
    movementTimer = new QTimer(this);
    deadTimer = new QTimer(this);
    attackCD = new QTimer(this);
    updateTimer = new QTimer(this);
}

void GameWindow::setGameData(GameData *_data)
{
    if (_data == nullptr)
    {
        return;
    }

    gameData = _data;

    // 停止所有定时器
    stopGame();

    // 清理游戏对象
    if (player)
    {
        delete player;
        player = nullptr;
    }

    for (Enemy *enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();

    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

    for (Enemy *enemy : enemyDeadList)
    {
        delete enemy;
    }
    enemyDeadList.clear();

    // 清理敌人生成定时器
    for (QTimer *timer : enemySpawnTimers)
    {
        timer->stop();
        delete timer;
    }
    enemySpawnTimers.clear();
    enemySpawnCounts.clear();

    // 加载图片（从gameData中读取路径）
    initPicture();

    // 创建玩家（使用gameData中的PlayerData）
    player = new Player(gameData->playerData, gameData->playerData.generatePos);

    // 设置玩家子弹数据（使用第一个子弹）
    if (!gameData->bulletDataList.empty())
    {
        player->setBulletData(&gameData->bulletDataList[gameData->playerData.bulletId]);
    }

    player->setEnemiesList(&enemies);
    player->setObstaclesList(&obstacles);

    // 连接玩家的子弹创建信号
    connect(player, &Player::createBullet, this, [this](Bullet *bullet)
            {
        bullets.append(bullet);
        connect(bullet, &Bullet::hitSth, this, [this, bullet]()
                { bullets.removeOne(bullet); }); });

    // 初始化定时器
    disconnect(movementTimer, nullptr, this, nullptr);
    connect(movementTimer, &QTimer::timeout, this, [this]()
            { handleMovement(gameData->playerData.moveStep, gameData->playerData.moveStep * 7 / 10); });
    movementTimer->start(16);

    disconnect(deadTimer, nullptr, this, nullptr);
    connect(deadTimer, &QTimer::timeout, this, &GameWindow::handleEnemyDead);
    connect(deadTimer, &QTimer::timeout, this, &GameWindow::handlePlayerDead);
    deadTimer->start(500);

    disconnect(attackCD, nullptr, this, nullptr);
    connect(attackCD, &QTimer::timeout, this, &GameWindow::handlePlayerAttack);
    attackCD->start(gameData->playerData.attackCD);

    disconnect(updateTimer, nullptr, this, nullptr);
    connect(updateTimer, &QTimer::timeout, this, [this]()
            { this->update(); });
    updateTimer->start(16);

    // 为每个敌人生成配置创建定时器
    for (size_t i = 0; i < gameData->enemySpawnConfigs.size(); i++)
    {
        const EnemySpawnConfig &config = gameData->enemySpawnConfigs[i];

        // 创建定时器
        QTimer *spawnTimer = new QTimer(this);
        connect(spawnTimer, &QTimer::timeout, this, [this, i]()
                { generateEnemy(i); });
        spawnTimer->start(config.spawnFreq);

        enemySpawnTimers.push_back(spawnTimer);
        enemySpawnCounts.push_back(0);
    }

    // 初始化地图和障碍物
    createMapCache(&gameData->grid);
    Enemy::setAttackTarget(player);

    // 创建高分辨率网格用于寻路
    int gridRows = gameData->grid.size();
    int gridCols = gridRows > 0 ? gameData->grid[0].size() : 0;
    int highResRows = gridRows * 2;
    int highResCols = gridCols * 2;

    std::vector<std::vector<int>> highResGrid(highResRows, std::vector<int>(highResCols, 0));

    for (int i = 0; i < gridRows; i++)
    {
        for (int j = 0; j < gridCols; j++)
        {
            int startX = i * 2;
            int startY = j * 2;
            int endX = (i + 1) * 2;
            int endY = (j + 1) * 2;

            for (int x = startX; x < endX && x < highResRows; x++)
            {
                for (int y = startY; y < endY && y < highResCols; y++)
                {
                    highResGrid[x][y] = gameData->grid[i][j];
                }
            }
        }
    }

    Enemy::setGrid(highResGrid);

    // 初始化子弹池（只在第一次初始化）
    if (Bullet::bulletsPool.empty())
    {
        Bullet::initBulletsPool(500);
    }
}

void GameWindow::reset()
{
    // 停止所有定时器
    stopGame();

    // 清理所有游戏对象
    if (player)
    {
        delete player;
        player = nullptr;
    }

    for (Enemy *enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();

    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

    for (Enemy *enemy : enemyDeadList)
    {
        delete enemy;
    }
    enemyDeadList.clear();

    // 清理敌人生成定时器
    for (QTimer *timer : enemySpawnTimers)
    {
        timer->stop();
        delete timer;
    }
    enemySpawnTimers.clear();
    enemySpawnCounts.clear();

    // 注意：不清空keyPressed，避免按键状态丢失导致人物卡死
}

void GameWindow::stopGame()
{
    // 停止所有定时器
    if (movementTimer)
        movementTimer->stop();
    if (deadTimer)
        deadTimer->stop();
    if (attackCD)
        attackCD->stop();
    if (updateTimer)
        updateTimer->stop();

    // 停止所有敌人生成定时器
    for (QTimer *timer : enemySpawnTimers)
    {
        if (timer)
            timer->stop();
    }
}

void GameWindow::clearKeyState()
{
    // 清空按键状态，避免按键残留
    keyPressed.clear();
}
GameWindow::~GameWindow()
{
    if (player)
    {
        delete player;
        player = nullptr;
    }

    for (Enemy *enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();

    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

    for (Enemy *enemy : enemyDeadList)
    {
        delete enemy;
    }
    enemyDeadList.clear();

    for (QTimer *timer : enemySpawnTimers)
    {
        delete timer;
    }
    enemySpawnTimers.clear();
}

QPixmap GameWindow::loadAndProcessImage(const std::string &imagePath, int width, int height)
{
    QImage image(imagePath.c_str());                                    // 加载图像文件
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied); // 转换为 ARGB 格式，支持透明通道
    QPixmap original = QPixmap::fromImage(image);                       // 将 QImage 转换为 QPixmap
    QPixmap scaled = original.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setMask(scaled.createHeuristicMask()); // 创建启发式掩码，用于透明处理
    return scaled;
}
void GameWindow::initPicture()
{
    if (!gameData)
        return;

    // 清空现有图片和映射
    playerImage.clear();
    enemyImages.clear();
    bulletImage.clear();
    imagePathToIndex.clear();

    int imageIndex = 0;

    // 加载玩家图片
    const PlayerData &pData = gameData->playerData;
    for (const auto &path : pData.rightWalkPaths)
    {
        if (imagePathToIndex.find(path) == imagePathToIndex.end())
        {
            playerImage.push_back(loadAndProcessImage(path, pData.width, pData.height));
            imagePathToIndex[path] = imageIndex++;
        }
    }
    for (const auto &path : pData.leftWalkPaths)
    {
        if (imagePathToIndex.find(path) == imagePathToIndex.end())
        {
            playerImage.push_back(loadAndProcessImage(path, pData.width, pData.height));
            imagePathToIndex[path] = imageIndex++;
        }
    }
    for (const auto &path : pData.upWalkPaths)
    {
        if (imagePathToIndex.find(path) == imagePathToIndex.end())
        {
            playerImage.push_back(loadAndProcessImage(path, pData.width, pData.height));
            imagePathToIndex[path] = imageIndex++;
        }
    }
    for (const auto &path : pData.downWalkPaths)
    {
        if (imagePathToIndex.find(path) == imagePathToIndex.end())
        {
            playerImage.push_back(loadAndProcessImage(path, pData.width, pData.height));
            imagePathToIndex[path] = imageIndex++;
        }
    }

    // 加载敌人图片
    for (const auto &config : gameData->enemySpawnConfigs)
    {
        for (const auto &path : config.enemyData.imagePaths)
        {
            if (imagePathToIndex.find(path) == imagePathToIndex.end())
            {
                enemyImages.push_back(loadAndProcessImage(path, config.enemyData.width, config.enemyData.height));
                imagePathToIndex[path] = imageIndex++;
            }
        }
    }

    // 加载子弹图片
    for (const auto &bData : gameData->bulletDataList)
    {
        for (const auto &path : bData.imagePaths)
        {
            if (imagePathToIndex.find(path) == imagePathToIndex.end())
            {
                bulletImage.push_back(loadAndProcessImage(path, bData.width, bData.height));
                imagePathToIndex[path] = imageIndex++;
            }
        }
    }
}
void GameWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    // 更新摄像机位置
    updateCamera();

    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);

    // 用缓存绘制地图（应用摄像机偏移）
    painter.drawPixmap(-cameraOffsetX, -cameraOffsetY, mapCache);

    // 绘制玩家（应用摄像机偏移）
    painter.drawPixmap(player->getX() - cameraOffsetX, player->getY() - cameraOffsetY,
                       playerImage[player->getCurrentImageIndex()]);

    // 绘制敌人（应用摄像机偏移）
    for (Enemy *enemy : enemies)
    {
        int imageIndex = enemy->getCurrentImageIndex();
        if (imageIndex >= 0 && imageIndex < enemyImages.size())
        {
            painter.drawPixmap(enemy->getX() - cameraOffsetX, enemy->getY() - cameraOffsetY,
                               enemyImages[imageIndex]);
        }
    }

    // 绘制子弹（应用摄像机偏移）
    for (Bullet *bullet : bullets)
    {
        int imageIndex = bullet->getCurrentImageIndex();
        if (imageIndex >= 0 && imageIndex < bulletImage.size())
        {
            painter.drawPixmap(bullet->getX() - cameraOffsetX, bullet->getY() - cameraOffsetY,
                               bulletImage[imageIndex]);
        }
    }

    // 绘制玩家血条（固定在屏幕上，不受摄像机影响）
    int maxHP = gameData->playerData.hp;
    int currentHP = player->getHp();
    QRect healthBarRect(100, 10, 100, 20);
    int fillWidth = (currentHP * (healthBarRect.width() - 4)) / maxHP;

    // 绘制血条背景
    painter.setPen(Qt::black);
    painter.setBrush(QColor(50, 50, 50));
    painter.drawRect(healthBarRect);

    // 绘制动态血量
    painter.setBrush(Qt::red);
    painter.drawRect(healthBarRect.x() + 2, healthBarRect.y() + 2,
                     fillWidth, healthBarRect.height() - 4);

    // 绘制血量数值
    painter.setPen(Qt::white);
    painter.drawText(healthBarRect.right() - 45, healthBarRect.y() + 15,
                     QString("%1").arg(currentHP));
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    keyPressed.insert(event->key());
}
void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;
    keyPressed.remove(event->key());
}
void GameWindow::handleMovement(int step, int diagonalStep)
{

    bool up = keyPressed.contains(Qt::Key_W);
    bool down = keyPressed.contains(Qt::Key_S);
    bool left = keyPressed.contains(Qt::Key_A);
    bool right = keyPressed.contains(Qt::Key_D);
    int dx = 0, dy = 0;
    if (up && left)
    {
        dx = -diagonalStep;
        dy = -diagonalStep;
    }
    else if (up && right)
    {
        dx = diagonalStep;
        dy = -diagonalStep;
    }
    else if (down && left)
    {
        dx = -diagonalStep;
        dy = diagonalStep;
    }
    else if (down && right)
    {
        dx = diagonalStep;
        dy = diagonalStep;
    }
    else if (up)
    {
        dy = -step;
    }
    else if (down)
    {
        dy = step;
    }
    else if (left)
    {
        dx = -step;
    }
    else if (right)
    {
        dx = step;
    }
    // 障碍物检测
    QPoint playerPos(player->getX(), player->getY());
    QPoint newPlayerPos(playerPos.x() + dx, playerPos.y() + dy);
    QRect newPlayerRect(newPlayerPos.x(), newPlayerPos.y(), player->width, player->height);
    bool isCollided = false;

    // 边界检测
    if (newPlayerRect.x() < 0 || newPlayerRect.x() + player->width > mapWidth ||
        newPlayerRect.y() < 0 || newPlayerRect.y() + player->height > mapHeight)
    {
        isCollided = true;
    }

    // 障碍物检测
    if (!isCollided)
    {
        for (const QRect &obstacle : obstacles)
        {
            if (newPlayerRect.intersects(obstacle))
            {
                isCollided = true;
                break;
            }
        }
    }

    if (!isCollided)
        player->setPosition(newPlayerPos.x(), newPlayerPos.y());

    if (left && !right)
        player->updateAnimation(Player::Left);
    else if ((right && !left) || down)
        player->updateAnimation(Player::Right);
    else if (up)
        player->updateAnimation(Player::Up);
}

void GameWindow::generateEnemy(int enemyConfigIndex)
{
    // 检查配置索引是否有效
    if (enemyConfigIndex < 0 || enemyConfigIndex >= gameData->enemySpawnConfigs.size())
    {
        qDebug() << "Error: enemyConfigIndex" << enemyConfigIndex << "out of range";
        return;
    }

    const EnemySpawnConfig &config = gameData->enemySpawnConfigs[enemyConfigIndex];

    // 检查是否达到生成上限
    if (enemySpawnCounts[enemyConfigIndex] >= config.totalNum)
    {
        enemySpawnTimers[enemyConfigIndex]->stop();
        return;
    }

    // 更新计数
    enemySpawnCounts[enemyConfigIndex]++;

    // 随机选择生成位置
    if (config.spawnPositions.empty())
    {
        qDebug() << "Error: No spawn positions for enemy config" << enemyConfigIndex;
        return;
    }

    int randomIndex = rand() % config.spawnPositions.size();
    QPoint pos = config.spawnPositions[randomIndex];

    // 创建敌人
    Enemy *newEnemy = new Enemy(config.enemyData, pos);
    newEnemy->setObstaclesList(&obstacles);

    // 如果是远程攻击敌人，设置子弹数据
    if (config.enemyData.hasRangedAttack && config.enemyData.bulletId < gameData->bulletDataList.size())
    {
        newEnemy->setBulletData(&gameData->bulletDataList[config.enemyData.bulletId]);
    }

    enemies.append(newEnemy);

    // 连接敌人的子弹创建信号
    connect(newEnemy, &Enemy::createBullet, this, [this](Bullet *bullet)
            {
        bullets.append(bullet);
        connect(bullet, &Bullet::hitSth, this, [this, bullet]()
                { bullets.removeOne(bullet); }); });
}

bool isInRange(int x1, int y1, int x2, int y2, int range)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= range * range;
}

void GameWindow::handlePlayerAttack()
{
    if (!player || !gameData)
        return;

    // 检查玩家是否可以攻击
    if (!player->canAttack())
        return;

    // 查找范围内的敌人
    Enemy *target = nullptr;
    for (Enemy *enemy : enemies)
    {
        if (isInRange(player->getX(), player->getY(), enemy->getX(), enemy->getY(), gameData->playerData.attackRange))
        {
            target = enemy;
            break;
        }
    }

    if (!target)
        return;

    // 使用Player::attack方法
    player->attack(target);
}

void GameWindow::handleEnemyDead()
{
    // 检查是否所有敌人都已生成且全部死亡
    bool allSpawned = true;
    int totalSpawned = 0;
    int totalMax = 0;

    for (size_t i = 0; i < enemySpawnCounts.size(); i++)
    {
        totalSpawned += enemySpawnCounts[i];
        totalMax += gameData->enemySpawnConfigs[i].totalNum;
        if (enemySpawnCounts[i] < gameData->enemySpawnConfigs[i].totalNum)
        {
            allSpawned = false;
        }
    }

    if (enemies.isEmpty() && allSpawned)
    {
        deadTimer->stop();
        stopGame();
        QMessageBox::information(this, "游戏结束", "消灭了所有敌人！");
        emit gameFinished();
        return;
    }

    if (enemies.isEmpty())
        return;

    // 处理死亡的敌人
    for (auto it = enemies.begin(); it != enemies.end();)
    {
        if ((*it)->getHp() <= 0)
        {
            (*it)->setPosition(0, 0);
            (*it)->stopMove();
            enemyDeadList.append(*it);
            it = enemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
void GameWindow::handlePlayerDead()
{
    if (player && player->getHp() <= 0)
    {
        stopGame();
        QMessageBox::information(this, "游戏结束", "玩家死亡！");
        emit gameFinished();
    }
}

void GameWindow::createMapCache(std::vector<std::vector<int>> *grid)
{
    int tileWidth = 10;
    int tileHeight = 10;

    // 计算地图实际大小（扩大2倍）
    mapWidth = grid->size() * tileWidth * 2;
    mapHeight = grid->at(0).size() * tileHeight * 2;

    floorTile = QPixmap(":/images/floor.png").scaled(200, 200);                            // 地板瓦片也放大2倍
    obstacleTile = QPixmap(":/images/Obstacle.png").scaled(tileWidth * 2, tileHeight * 2); // 障碍物放大2倍

    // 创建障碍物列表（坐标也放大2倍）
    for (int i = 0; i < grid->size(); ++i)
    {
        for (int j = 0; j < grid->at(i).size(); ++j)
        {
            if ((*grid)[i][j])
            {
                obstacles.append(QRect(i * 20, j * 20, 20, 20)); // 放大2倍
            }
        }
    }

    mapCache = QPixmap(mapWidth, mapHeight);
    mapCache.fill(Qt::transparent); // 填充透明背景
    QPainter painter(&mapCache);

    // 绘制地板（放大2倍，数量也增加）
    for (int i = 0; i < mapWidth / 200 + 1; i++)
    {
        for (int j = 0; j < mapHeight / 200 + 1; j++)
        {
            painter.drawPixmap(i * 200, j * 200, floorTile);
        }
    }

    // 绘制障碍物（放大2倍）
    for (int i = 0; i < grid->size(); ++i)
    {
        for (int j = 0; j < grid->at(i).size(); ++j)
        {
            if ((*grid)[i][j] == 1)
            { // 如果是墙壁
                painter.drawPixmap(i * 20, j * 20, obstacleTile);
            }
        }
    }

    // 根据enemyData[2]绘制魔法阵（位置也放大2倍）
    for (int i = 0; i < gameData->enemyData[2].totalNum; i++)
    {
        magicCircleList.append(QRect(gameData->enemyData[2].generatePos[i].x() * 2,
                                     gameData->enemyData[2].generatePos[i].y() * 2, 100, 100)); // 放大2倍
    }
    magicCircleImage = loadAndProcessImage(":/images/MagicCircle.png", 100, 100); // 魔法阵也放大2倍
    for (int i = 0; i < magicCircleList.size(); i++)
    {
        painter.drawPixmap(magicCircleList[i].x(), magicCircleList[i].y(), magicCircleImage);
    }
}

// 更新摄像机位置，使玩家居中
void GameWindow::updateCamera()
{
    if (!player)
        return;

    // 计算摄像机偏移，使玩家在屏幕中心
    cameraOffsetX = player->getX() + player->width / 2 - width() / 2;
    cameraOffsetY = player->getY() + player->height / 2 - height() / 2;

    // 限制摄像机不超出地图边界
    if (cameraOffsetX < 0)
        cameraOffsetX = 0;
    if (cameraOffsetY < 0)
        cameraOffsetY = 0;
    if (cameraOffsetX > mapWidth - width())
        cameraOffsetX = mapWidth - width();
    if (cameraOffsetY > mapHeight - height())
        cameraOffsetY = mapHeight - height();
}

void GameWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    this->deleteLater();
}
