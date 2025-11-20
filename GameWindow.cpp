#include "GameWindow.h"
#include <QPoint>
#include <vector>
#include <QMessageBox>
#include <QDebug>



GameWindow::GameWindow(QWidget *parent) : QWidget(parent), gameData(nullptr)
{
    // 设置固定大小
    setMinimumSize(800, 600);

    // 初始化基础定时器
    movementTimer = new QTimer(this);
    deadTimer = new QTimer(this);
    attackCD = new QTimer(this);
    updateTimer = new QTimer(this);
    setFocusPolicy(Qt::StrongFocus);
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


    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

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


    player->setEnemiesList(&enemies);

    QList<QRect> *obstacles = new QList<QRect>;
    for (const auto &obstacle :gameData-> mapData.obstacles)
    {
        QRect rect(obstacle.pos.x(), obstacle.pos.y(), obstacle.width, obstacle.height);
        obstacles->append(rect);
    }
    player->setObstaclesList(obstacles);


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

    Enemy::setAttackTarget(player);
    Enemy::mapData=&(_data->mapData);
    Enemy::obstacles=obstacles;
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


    // 初始化子弹池（只在第一次初始化）
    if (Bullet::bulletsPool.empty())
    {
        Bullet::initBulletsPool(500);
    }
    Enemy::initEnemiesPool(5000);
    Enemy::allenemies=&enemies;
    // 初始化map大小
    mapHeight=_data->mapData.mapHeight;
    mapWidth=_data->mapData.mapWidth;
    createMapCache();
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

    for (Enemy *enemy : *(Enemy::enemiesPool))
    {
        delete enemy;
    }
    enemies.clear();
    enemyDeadList.clear();
    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

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
    // 停止PLayer
    // 停止所有enemy
    for(auto enemy:enemies){
        enemy->stop();
    }
    // 停止所有bullet
    for(auto bullet:bullets){
        bullet->stopMove();
    }
}
void GameWindow::startGame()
{
    if(movementTimer){
        movementTimer->start(16);
    }
    if(deadTimer){
        deadTimer->start(500);
    }
    if(updateTimer){
        updateTimer->start(16);
    }
    attackCD->start(player->getAttackCD());
    for (int i=0;i<enemySpawnTimers.size();i++)
    {
        enemySpawnTimers[i]->start();
    }
    for(auto enemy:enemies){
        enemy->start();
    }
    for(auto bullet:bullets){
        bullet->startMove();
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

QPixmap GameWindow::loadAndProcessImage(const std::string &imagePath, int width=0, int height=0)
{
    QImage image(imagePath.c_str());                                    // 加载图像文件
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied); // 转换为 ARGB 格式，支持透明通道
    QPixmap original = QPixmap::fromImage(image);
    if(width!=0){    // 将 QImage 转换为 QPixmap
    QPixmap scaled = original.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setMask(scaled.createHeuristicMask()); // 创建启发式掩码，用于透明处理
    return scaled;}
    else {
        original.setMask(original.createHeuristicMask());
        return original;
    }
}
void GameWindow::initPicture()
{
    if (!gameData)
        return;

    // 清空现有图片和映射
    playerImage.clear();
    enemyImages.clear();
    bulletImage.clear();

    // 加载玩家图片
    const PlayerData &pData = gameData->playerData;
    for (const auto &path : pData.rightWalkPaths)
    {
        playerImage[Right].push_back(loadAndProcessImage(path,pData.width,pData.height));
    }
    for (const auto &path : pData.leftWalkPaths)
    {
        playerImage[Left].push_back(loadAndProcessImage(path,pData.width,pData.height));
    }
    for (const auto &path : pData.upWalkPaths)
    {
        playerImage[Up].push_back(loadAndProcessImage(path,pData.width,pData.height));
    }
    for (const auto &path : pData.downWalkPaths)
    {
        playerImage[Down].push_back(loadAndProcessImage(path,pData.width,pData.height));
    }


    // 加载敌人图片
    for (const auto &config : gameData->enemySpawnConfigs)
    {
        enemyImages.push_back(std::map<Direction,std::vector<QPixmap>>());
        for (const auto &path : config.enemyData.rightWalkPaths)
        {
            enemyImages[config.enemyData.enemyID][Right].push_back(loadAndProcessImage(path,config.enemyData.width,config.enemyData.height));
        }
        for (const auto &path : config.enemyData.leftWalkPaths)
        {
            enemyImages[config.enemyData.enemyID][Left].push_back(loadAndProcessImage(path,config.enemyData.width,config.enemyData.height));
        }
    }

    // 加载子弹图片
    for (const auto &bData : gameData->bulletDataList)
    {
        bulletImage.push_back(std::vector<QPixmap>());
        for (const auto &path : bData.imagePaths)
        {
            bulletImage[bData.bulletID].push_back(loadAndProcessImage(path,bData.width,bData.height));
        }
    }

    //加载npc图片
    isGreeting = std::vector<bool>(gameData->npcDatas.size(),false);
    for(const auto &npc : gameData->npcDatas){
        npcImage.push_back({{
            loadAndProcessImage(npc.imagePath,npc.width1,npc.height1),
            loadAndProcessImage(npc.greetingImagePath,npc.width2,npc.height2)
        },
            false
        });
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
                       playerImage[player->getState()][player->pictureIndex[player->getState()].curIndex]);

    // 绘制敌人（应用摄像机偏移）
    for (int i=0;i<enemies.size();i++)
    {
        Enemy *enemy = enemies[i];
        qDebug() << "enemyImages[state].size=" << enemyImages[enemy->getID()][enemy->getState()].size();

        painter.drawPixmap(enemy->getX() - cameraOffsetX, enemy->getY() - cameraOffsetY,
                               enemyImages[enemy->getID()][enemy->getState()][enemy->updateAnimation()]);
    }

    //绘制npc（应用摄像机偏移）
    for(int i=0;i<npcImage.size();i++){
        painter.drawPixmap(gameData->npcDatas[i].pos.x() - cameraOffsetX,gameData->npcDatas[i].pos.y() - cameraOffsetY,
                           npcImage[i].second?npcImage[i].first.second:npcImage[i].first.first);
    }

    // 绘制子弹（应用摄像机偏移）
    for (Bullet *bullet : bullets)
    {
        painter.drawPixmap(bullet->getX()-cameraOffsetX,bullet->getY()-cameraOffsetY,
                           bulletImage[bullet->getID()][bullet->updatePictureIndex()]);
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
double dis(int x1, int y1, int x2, int y2)
{
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) );
}

void GameWindow::handleMovement(int step, int diagonalStep)
{
    //检查是否与npc相遇
    for(int i=0;i<gameData->npcDatas.size();i++){
        if(dis(player->getX(),player->getY(),gameData->npcDatas[i].pos.x(),gameData->npcDatas[i].pos.y())<player->getAttackRange()/3){
            npcImage[i].second=true;
            handleNPCGreeting(i);
            break;
        }else npcImage[i].second = false;
    }

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
    QRect newPlayerRect(newPlayerPos.x(), newPlayerPos.y(), player->getWidth(), player->getHeight());
    bool isCollided = false;

    // 边界检测
    if (newPlayerRect.x() < 0 || newPlayerRect.x() + player->getWidth() > mapWidth ||
        newPlayerRect.y() < 0 || newPlayerRect.y() + player->getHeight() > mapHeight)
    {
        isCollided = true;
    }

    // 障碍物检测
    if (!isCollided)
    {
        for(const auto &obstacle :gameData->mapData.obstacles)
        {
            if (newPlayerRect.intersects(QRect(obstacle.pos.x(), obstacle.pos.y(), obstacle.width, obstacle.height)))
            {
                isCollided = true;
                break;
            }
        }
    }

    if (!isCollided)
        player->setPosition(newPlayerPos.x(), newPlayerPos.y());
    if (left && !right)
    {player->setState(Left);
    }
    else if ((right && !left) || down)
        player->setState(Right);
    else if (up && !down)
        player->setState(Up);
    else if(!up &&down)player->setState(Down);
    if(left||right||up||down)player->updateAnimation();
}

void GameWindow::generateEnemy(int enemyConfigIndex)
{

    // 检查配置索引是否有效
    if (enemyConfigIndex < 0 || enemyConfigIndex >= gameData->enemySpawnConfigs.size())
    {
        qDebug() << "Error: enemyConfigIndex" << enemyConfigIndex << "out of range";
        return;
    }
        qDebug() << "generateEnemy called, index:" << enemyConfigIndex;

    EnemySpawnConfig &config = gameData->enemySpawnConfigs[enemyConfigIndex];
        qDebug() << "spawn freq =" << config.spawnFreq
                 << "totalNum =" << config.totalNum
                 << "spawned =" << enemySpawnCounts[enemyConfigIndex];

    // 检查是否达到生成上限
    if (enemySpawnCounts[enemyConfigIndex] >= config.totalNum)
    {
        enemySpawnTimers[enemyConfigIndex]->stop();
        return;
    }

    // 更新计数
    enemySpawnCounts[enemyConfigIndex]++;

    Enemy *newEnemy =Enemy::createEnemy(config);
    if (!newEnemy) {
        qDebug() << "createEnemy FAILED";
        return;
    }
    enemies.append(newEnemy);

    // 连接敌人的子弹创建信号
    connect(newEnemy, &Enemy::createBullet, this, [this](Bullet *bullet)
            {
        bullets.append(bullet);
        connect(bullet, &Bullet::hitSth, this, [this, bullet]()
                { bullets.removeOne(bullet); }); });
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
    double minDis=-1.0;
    for (Enemy *enemy : enemies)
    {
        if(minDis==-1.0){minDis=dis(player->getX(),player->getY(),enemy->getX(),enemy->getY());target=enemy;}
        else {
            double _dis = dis(player->getX(),player->getY(),enemy->getX(),enemy->getY());
            if (minDis>_dis){
                minDis=_dis;
                target=enemy;
            }
        }
    }

    if (!target||minDis>player->getAttackRange())
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
            (*it)->isDead=true;
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

void GameWindow::createMapCache()
{
    //绘制背景
    mapCache = QPixmap(loadAndProcessImage( gameData->mapData.backgroundImgPath));
    if (mapCache.isNull())
    {
        qDebug() << "Error loading map background image:" << gameData->mapData.backgroundImgPath;
        return;
    }
    // 绘制障碍物
    for (const auto &obstacle :gameData-> mapData.obstacles)
    {
        QRect rect(obstacle.pos.x(), obstacle.pos.y(), obstacle.width, obstacle.height);
        // 加载障碍物图片
        QPixmap obstacleImg=loadAndProcessImage(obstacle.imagePath,obstacle.width,obstacle.height);
        if (obstacleImg.isNull())
        {
            qDebug() << "Error loading obstacle image:" << obstacle.imagePath;
            continue;
        }
        QPainter painter(&mapCache);
        painter.drawPixmap(obstacle.pos.x(),obstacle.pos.y(), loadAndProcessImage(obstacle.imagePath,obstacle.width,obstacle.height));
    }

}

// 更新摄像机位置，使玩家居中
void GameWindow::updateCamera()
{
    if (!player)
        return;

    // 计算摄像机偏移，使玩家在屏幕中心
    cameraOffsetX = player->getX() + player->getWidth() / 2 - width() / 2;
    cameraOffsetY = player->getY() + player->getHeight() / 2 - height() / 2;

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

void GameWindow::handleNPCGreeting(int id){
    if(isGreeting[id]==true)return;
    if(keyPressed.contains(Qt::Key_Enter)){
        emit plotStart(id);
    npcData &npc = gameData->npcDatas[id];
    player->upgrade(npc.dhp,npc.ddamage,npc.dattackRange,npc.dattackCD,npc.dmoveStep,npc.bulletData);
    isGreeting[id]=true;
    stopGame();
    }
}
