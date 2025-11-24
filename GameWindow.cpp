#include "GameWindow.h"
#include <QPoint>
#include <qlabel.h>
#include <vector>
#include <QMessageBox>
#include <QDebug>
#include <qboxlayout.h>


GameWindow::GameWindow(QWidget *parent) : QWidget(parent), gameData(nullptr)
{
    // 设置固定大小
    setMinimumSize(800, 600);

    // 初始化基础定时器
    movementTimer = new QTimer(this);
    deadTimer = new QTimer(this);
    attackCD = new QTimer(this);
    updateTimer = new QTimer(this);
    powerEnemy = new QTimer(this);

    warningTimer = new QTimer(this);
    setFocusPolicy(Qt::StrongFocus);
}


void GameWindow::setGameData(GameData *_data)
{
    if (_data == nullptr)
    {
        return;
    }

    gameData = _data;
    taskList = _data->taskList;
    bossNum = _data->bossNum;
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
    disconnect(powerEnemy,nullptr,this,nullptr);
    connect(powerEnemy, &QTimer::timeout, this, [this]() {
        for (auto &config : gameData->enemySpawnConfigs) {
            config.enemyData.damage *= 1.1;
            config.enemyData.hp *= 1.1;
        }
        warningText = "⚠️ 警告：敌人已增强！生命值与攻击力提升！";
        showWarning = true;
        warningTimer->start(5000);
    });
    powerEnemy->start(60000);
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

    disconnect(warningTimer,nullptr,this,nullptr);
    warningTimer->setSingleShot(true); // 设置为单次触发（只响一次）
    connect(warningTimer, &QTimer::timeout, this, [this](){
        showWarning = false; // 时间到了，关闭显示
    });
    Enemy::setAttackTarget(player);
    Enemy::mapData=&(_data->mapData);
    Enemy::obstacles=obstacles;
    // 为每个敌人生成配置创建定时器
    for (size_t i = 0; i < gameData->enemySpawnConfigs.size(); i++)
    {        const EnemySpawnConfig &config = gameData->enemySpawnConfigs[i];

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
    if (powerEnemy) powerEnemy->stop();
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
    if (powerEnemy) powerEnemy->start();
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

    // 绘制任务栏标题
    if (!taskList.empty()) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(10, 35, QString::fromLocal8Bit("TaskBar"));

        // 绘制任务栏
        int taskBarX = 10;
        int taskBarY = 40; // Position below health bar
        int taskItemHeight = 20;
        int taskItemWidth = 300;
        int maxTasksToShow = 5; // Limit number of tasks shown

        for (size_t i = 0; i < taskList.size() && i < maxTasksToShow; i++) {
            const TaskData& task = taskList[i];

            // Only show valid tasks
            if (!task.isValid||task.isComplete) continue;

            QRect taskRect(taskBarX, taskBarY + i * taskItemHeight + 20, taskItemWidth, taskItemHeight);

            // Draw task background based on completion status
            if (task.isComplete) {
                painter.setBrush(QColor(50, 150, 50, 200)); // Green for completed tasks
            } else {
                painter.setBrush(QColor(70, 70, 100, 200)); // Blue for incomplete tasks
            }

            painter.setPen(Qt::black);
            painter.drawRect(taskRect);

            // Draw task text
            QString taskText = QString::fromStdString(task.taskName + ": " + task.taskDiscrubtion);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 8));

            // Truncate text if too long
            QFontMetrics metrics(painter.font());
            if (metrics.horizontalAdvance(taskText) > taskItemWidth - 10) {
                taskText = metrics.elidedText(taskText, Qt::ElideRight, taskItemWidth - 10);
            }

            painter.drawText(taskRect.x() + 5, taskRect.y() + 13, taskText);
        }
    }
    // 绘制警告
    if (showWarning) {
        painter.save(); // 保存画笔状态

        // 1. 设置字体
        QFont font("Microsoft YaHei", 24, QFont::Bold);
        painter.setFont(font);
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(warningText);
        int textHeight = fm.height();

        // 2. 计算居中位置 (屏幕上方 100像素处)
        int x = (width() - textWidth) / 2;
        int y = 100;

        // 3. 画一个半透明黑底背景 (让文字更清晰)
        painter.setBrush(QColor(0, 0, 0, 150)); // 黑色，透明度150
        painter.setPen(Qt::NoPen);
        // 背景框稍微比文字大一点
        painter.drawRoundedRect(x - 20, y, textWidth + 40, textHeight + 10, 10, 10);

        // 4. 画文字 (红色)
        painter.setPen(QColor(255, 50, 50)); // 亮红色
        // 注意：drawText 的 y 坐标通常是基线，所以要往下移一点，或者用 rect 对齐
        painter.drawText(x, y + fm.ascent() + 5, warningText);

        painter.restore(); // 恢复画笔状态
    }
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
    bool isPlot = false;
    for(int i=0;i<gameData->npcDatas.size();i++){
        if(dis(player->getX(),player->getY(),gameData->npcDatas[i].pos.x(),gameData->npcDatas[i].pos.y())<player->getAttackRange()/3){
            npcImage[i].second=true;
            isPlot =handleNPCGreeting(i);
            break;
        }else npcImage[i].second = false;
    }
    if(isPlot)return;
    // 是否遇见boss
    for(int i=0;i<bossList.size();i++){
        if(!bossList[i]->isDead&&dis(player->getX(),player->getY(),bossList[i]->getX(),bossList[i]->getY())<player->getAttackRange()){
            bossList[i]->data->canMove=true;
            bossList[i]->startMove();
        }
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

    // 检查是否是boss
    if(enemyConfigIndex>=gameData->enemySpawnConfigs.size()-bossNum)
    {       bossList.push_back(newEnemy);
           QMessageBox::information(this, "危险提醒", "新的boss已生成");
    }

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

    /*if (enemies.isEmpty() && allSpawned)
    {
        deadTimer->stop();
        stopGame();
        QMessageBox::information(this, "游戏结束", "消灭了所有敌人！");
        emit gameFinished();
        return;
    }
    */
    // 改成触发完所有npc即可结束游戏
    bool flag = true;
    for(int i=0;i<isGreeting.size();i++){
        if(isGreeting[i]==false){
            flag = false;
            break;
        }
    }
    checkTask();

    if(flag == true){
        deadTimer->stop();
        stopGame();
        QMessageBox::information(this, "游戏结束","恭喜你完成了所有任务！");
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

bool GameWindow::handleNPCGreeting(int id){
    bool ifPlot = true;
    for(int i=0;i<id;i++){
        if(isGreeting[i]==false){
            ifPlot=false;
            break;
        }
    }
    if(isGreeting[id]==true)return false;
    if(keyPressed.contains(Qt::Key_Enter)){
        if(!ifPlot){
            showWarning=true;
            warningText = "前置剧情未触发！";
            warningTimer->start(3000);
            return false;}
        emit plotStart(id);
    npcData &npc = gameData->npcDatas[id];
    player->upgrade(npc.dhp,npc.ddamage,npc.dattackRange,npc.dattackCD,npc.dmoveStep,npc.bulletData);
    isGreeting[id]=true;
    stopGame();
    return true;
    }
    return false;
}
QString getBuffDescription(const TaskData::Buff& buff) {
    QStringList parts;
    if (buff.dhp != 0) parts << QString("生命值 +%1").arg(buff.dhp);
    if (buff.ddamage != 0) parts << QString("攻击力 +%1").arg(buff.ddamage);
    if (buff.dattackRange != 0) parts << QString("攻击范围 +%1").arg(buff.dattackRange);
    if (buff.dattackCD != 0) parts << QString("攻击冷却 +%1").arg(buff.dattackCD); // 注意：负数通常是增益
    if (buff.dmoveStep != 0) parts << QString("移动速度 +%1").arg(buff.dmoveStep);
    if (buff.bulletData != nullptr) parts << "获得新子弹效果";

    return parts.join("，");
}

void GameWindow::showTaskRewardDialog(const TaskData &task)
{
    // 1. 暂停游戏
    stopGame();

    // 2. 创建对话框
    QDialog dlg(this);
    dlg.setWindowTitle("任务完成！");
    dlg.setMinimumWidth(400);
    // 设置窗口标志：去掉关闭按钮，强制玩家必须选择一个奖励
    dlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    // 3. 设置垂直布局
    QHBoxLayout *layout = new QHBoxLayout(&dlg);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // 4. 添加标题和任务描述
    // 注意：这里使用了你结构体中的命名 taskDiscrubtion
    QLabel *titleLabel = new QLabel(QString::fromStdString(task.taskName), &dlg);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFD700;"); // 金色标题
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel(QString::fromStdString(task.taskDiscrubtion), &dlg);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 14px; color: #333;");
    descLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descLabel);

    layout->addSpacing(10);

    // 5. 遍历 buffs[3] 生成按钮
    bool hasOption = false;
    for (int i = 0; i < 3; ++i) {
        const TaskData::Buff& currentBuff = task.buffs[i];
        hasOption = true;

        // 生成按钮文字
        QString btnText = QString("选项 %1:\n%2").arg(i + 1).arg(getBuffDescription(currentBuff));

        QPushButton *btn = new QPushButton(btnText, &dlg);
        // 美化按钮样式
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: #4CAF50; color: white; border-radius: 8px; "
            "   padding: 10px; font-size: 14px; text-align: left; border: 1px solid #3e8e41;"
            "}"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #3e8e41; border: 2px solid white; }"
            );

        // 连接点击信号
        // 注意：Lambda按值捕获 currentBuff，这样每个按钮都保存了自己那份数据
        connect(btn, &QPushButton::clicked, this, [this, currentBuff, &dlg]() {
            // 应用属性升级
            this->player->upgrade(
                currentBuff.dhp,
                currentBuff.ddamage,
                currentBuff.dattackRange,
                currentBuff.dattackCD,
                currentBuff.dmoveStep,
                currentBuff.bulletData
                );
            // 关闭对话框
            dlg.accept();
        });

        layout->addWidget(btn);
    }

    // 6. 兜底处理：如果配置全空，没有任何按钮，为了防止死锁，加一个确认键
    if (!hasOption) {
        QPushButton *closeBtn = new QPushButton("领取奖励 (无)", &dlg);
        connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        layout->addWidget(closeBtn);
    }

    // 7. 模态运行 (阻塞直到用户点击)
    dlg.exec();

    // 8. 恢复游戏
    startGame();
}
// GameWindow.cpp

void GameWindow::checkTask() {
    for (int i = 0; i < taskList.size(); i++) {
        TaskData &task = taskList[i];

        // 如果任务已经完成，直接跳过（避免重复弹窗）
        if (task.isComplete) continue;

        // 1. 检查任务开启条件 (isValid)
        if (!task.isValid) {
            bool reqBossMet = true;
            bool reqNpcMet = true;
            bool reqEnemyMet = true;

            // --- 检查前置 Boss ---
            if (task.bossID_request != -1) {
                // 安全计算 Boss 索引
                int bossStartIndex = gameData->enemySpawnConfigs.size() - bossNum;
                // 假设 bossID 对应的是 enemyID，我们需要找到对应这个 enemyID 的 boss 实例
                // 这里简化逻辑：假设 bossList 是按生成顺序存储的，且对应配置文件的后几个
                int bossIndexInList = task.bossID_request - bossStartIndex; // 这种计算依赖于 ID 和 数组顺序完全对应，比较脆弱

                // 更稳妥的方式：遍历 bossList 找对应的 enemyID
                bool bossFoundAndDead = false;
                for(auto* boss : bossList) {
                    if(boss->getID() == task.bossID_request && boss->isDead) {
                        bossFoundAndDead = true;
                        break;
                    }
                }
                if (!bossFoundAndDead) reqBossMet = false;
            }

            // --- 检查前置 NPC ---
            if (task.npcID_request != -1) {
                // 增加越界检查
                if (task.npcID_request >= 0 && task.npcID_request < isGreeting.size()) {
                    if (!isGreeting[task.npcID_request]) reqNpcMet = false;
                } else {
                    reqNpcMet = false; // ID 无效
                }
            }

            // --- 检查前置杀怪数 ---
            if (task.enemyCnt_request != -1) {
                if (enemyDeadList.size() < task.enemyCnt_request) reqEnemyMet = false;
            }

            if (reqBossMet && reqNpcMet && reqEnemyMet) {
                task.isValid = true;
                // qDebug() << "Task Validated:" << QString::fromStdString(task.taskName);
            }
        }

        // 2. 检查任务完成条件 (isComplete)
        // 注意：这里用 else，表示 isValid 变为 true 的下一帧才会检查完成，或者如果逻辑允许当帧检查也可以去掉 else
        else {
            bool targetBossMet = true;
            bool targetNpcMet = true;
            bool targetEnemyMet = true;

            // === 检查 Boss 目标 ===
            if (task.bossID_target != -1) {
                bool bossKilled = false;
                // 遍历 bossList 查找指定 ID 的 Boss 是否死亡
                for(auto* boss : bossList) {
                    if(boss->getID() == task.bossID_target) {
                        if(boss->isDead) {
                            bossKilled = true;
                        }
                        break; // 找到对应ID的Boss及其状态
                    }
                }
                // 如果没找到该 Boss 或者该 Boss 没死，则目标未达成
                if (!bossKilled) targetBossMet = false;
            }

            // === 检查 NPC 目标 ===
            if (task.npcID_target != -1) {
                if (task.npcID_target >= 0 && task.npcID_target < isGreeting.size()) {
                    if (!isGreeting[task.npcID_target]) targetNpcMet = false;
                } else {
                    targetNpcMet = false;
                }
            }

            // === 检查杀怪数量目标 ===
            // 注意：你的逻辑是检查总死亡数，而不是特定怪物的死亡数
            // 如果任务要求杀“5只哥布林”，这里只检查了“死了5个任意怪”
            // 暂时按你的原逻辑保留
            if (enemyDeadList.size() < task.enemyCnt_target) {
                targetEnemyMet = false;
            }

            // 当所有目标都满足时
            if (targetBossMet && targetNpcMet && targetEnemyMet) {
                task.isComplete = true;
                showTaskRewardDialog(task);
            }
        }
    }
}
void GameWindow::focusOutEvent(QFocusEvent *event){
    keyPressed.clear();
    QWidget::focusOutEvent(event);
}

