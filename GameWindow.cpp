#include "GameWindow.h"
#include <QPoint>
#include <vector>
#include <QMessageBox> // Ensure this include is present for QMessageBox
/*
生成玩家在gameWindow的构造函数中初始化, player构造函数只负责初始化默认的动画, 其他性质由gameData初始化.
生成敌人在generateEnemy函数中初始化并赋值, 敌人构造函数只负责动画和移动,自动发动攻击, 但移动频率, 攻击cd,攻击伤害由gameData初始化.
武器只用来记录数据, 只根据level和type初始化并设定range和damage.只装备在玩家身上.
子弹继承自Character, 继承其移动和动画的实现,构造函数初始化子弹的动画和移动,伤害由玩家的武器和Enemy的damage决定,移动频率不变.目标为player即UndeadMage施法时无视其他敌人和障碍物, 没有范围.
因为需要交互, 玩家的移动和攻击在gameWindow中实现, Enemy的移动和攻击必须各自实现, 需要传入部分游戏数据
*/

GameWindow::GameWindow(QWidget *parent) : QWidget(parent), gameData(nullptr)
{
    // 设置固定大小
    setFixedSize(800, 600);
    // 加载图片
    initPicture();

    // 初始化定时器（但不启动）
    movementTimer = new QTimer(this);
    goblinTimer = new QTimer(this);
    undeadMageTimer = new QTimer(this);
    deadTimer = new QTimer(this);
    attackCD = new QTimer(this);
    updateTimer = new QTimer(this);
    updateDropTimer = new QTimer(this);
    bgmSound = new QSoundEffect(this);
}

void GameWindow::setGameData(GameData *_data)
{
    if (_data == nullptr)
    {
        return;
    }

    gameData = _data;

    // 重置游戏状态
    reset();

    // 初始化玩家数据
    if (player)
    {
        delete player;
    }
    player = new Player(Player::Roxy, gameData->playerData.generatepos);
    player->setHP(gameData->playerData.hp);
    player->setWeapon(new Weapon(gameData->playerData.weaponData.level, gameData->playerData.weaponData.type));

    // 断开旧连接并重新连接定时器
    disconnect(movementTimer, nullptr, this, nullptr);
    connect(movementTimer, &QTimer::timeout, this, [this]()
            { handleMovement(gameData->playerData.step, gameData->playerData.step * 7 / 10); });
    movementTimer->start(16);

    // 生成敌人,用两个定时器
    disconnect(goblinTimer, nullptr, this, nullptr);
    connect(goblinTimer, &QTimer::timeout, this, [this]()
            { generateEnemy(Enemy::Goblin, &gameData->enemyData[0].generatePos); });
    goblinTimer->start(gameData->enemyData[0].generateF);

    disconnect(undeadMageTimer, nullptr, this, nullptr);
    connect(undeadMageTimer, &QTimer::timeout, this, [this]()
            { generateEnemy(Enemy::UndeadMage, &gameData->enemyData[1].generatePos); });
    undeadMageTimer->start(gameData->enemyData[1].generateF);

    disconnect(deadTimer, nullptr, this, nullptr);
    connect(deadTimer, &QTimer::timeout, this, &GameWindow::handleEnemyDead);  // 连接定时器和处理敌人死亡的函数
    connect(deadTimer, &QTimer::timeout, this, &GameWindow::handlePlayerDead); // 连接定时器和处理玩家死亡的函数
    deadTimer->start(500);

    disconnect(attackCD, nullptr, this, nullptr);
    connect(attackCD, &QTimer::timeout, this, &GameWindow::handlePlayerAttack);
    attackCD->start(player->getWeapon()->getCoolDownCD());

    // 定时绘图
    disconnect(updateTimer, nullptr, this, nullptr);
    connect(updateTimer, &QTimer::timeout, this, [this]()
            { this->update(); });
    updateTimer->start(16);

    disconnect(updateDropTimer, nullptr, this, nullptr);
    connect(updateDropTimer, &QTimer::timeout, this, [this]()
            { generateEnemy(Enemy::drop_healing, &gameData->enemyData[3].generatePos); });
    connect(updateDropTimer, &QTimer::timeout, this, [this]()
            { generateEnemy(Enemy::drop_attack, &gameData->enemyData[4].generatePos); });
    connect(updateDropTimer, &QTimer::timeout, this, [this]()
            { generateEnemy(Enemy::drop_speed, &gameData->enemyData[5].generatePos); });
    updateDropTimer->start(gameData->enemyData[3].generateF);

    enemies.clear();
    goblinCount = 0;
    undeadMageCount = 0;

    // 初始化障碍物
    createMapCache(&gameData->grid);
    Enemy::attackTarget = player;

    // 创建更高分辨率的网格
    std::vector<std::vector<int>> highResGrid(200, std::vector<int>(150, 0));
    // 将原始网格映射到高分辨率网格
    for (int i = 0; i < gameData->grid.size(); i++)
    {
        for (int j = 0; j < gameData->grid[i].size(); j++)
        {
            // 计算在高分辨率网格中对应的区域
            int startX = i * 200 / 80;
            int startY = j * 150 / 60;
            int endX = (i + 1) * 200 / 80;
            int endY = (j + 1) * 150 / 60;

            // 填充对应区域
            for (int x = startX; x < endX; x++)
            {
                for (int y = startY; y < endY; y++)
                {
                    if (x < 200 && y < 150)
                    {
                        highResGrid[x][y] = gameData->grid[i][j];
                    }
                }
            }
        }
    }
    // 将高分辨率网格传递给Enemy
    Enemy::grid = highResGrid;

    // 初始化子弹池（只在第一次初始化）
    if (Bullet::bulletsPool.empty())
    {
        Bullet::initBulletsPool(500);
    }

    // 初始化背景音乐
    bgmSound->setSource(QUrl("qrc:/audio/bgm.wav")); // 注意：QSoundEffect最好使用WAV格式
    bgmSound->setLoopCount(QSoundEffect::Infinite);
    bgmSound->setVolume(0.3f); // 设置音量，范围0.0-1.0

    // 使用QTimer延迟一小段时间后再播放音乐
    QTimer::singleShot(500, this, [this]()
                       {
        if (bgmSound && bgmSound->status() == QSoundEffect::Ready) {
            bgmSound->play();
        } });
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

    for (Enemy *enemy : dropList)
    {
        delete enemy;
    }
    dropList.clear();

    magicCircleList.clear();
    keyPressed.clear();
    goblinCount = 0;
    undeadMageCount = 0;
}

void GameWindow::stopGame()
{
    // 停止所有定时器
    if (movementTimer)
        movementTimer->stop();
    if (goblinTimer)
        goblinTimer->stop();
    if (undeadMageTimer)
        undeadMageTimer->stop();
    if (deadTimer)
        deadTimer->stop();
    if (attackCD)
        attackCD->stop();
    if (updateTimer)
        updateTimer->stop();
    if (updateDropTimer)
        updateDropTimer->stop();

    // 停止背景音乐
    if (bgmSound && bgmSound->isPlaying())
    {
        bgmSound->stop();
    }
}
GameWindow::~GameWindow()
{
    if (player)
    {
        delete player; // 释放玩家指针
        player = nullptr;
    }
    for (Enemy *enemy : enemies)
    {
        delete enemy; // 释放敌人指针
    }
    enemies.clear();

    for (Bullet *bullet : bullets)
    {
        Bullet::deleteBullet(bullet);
    }
    bullets.clear();

    // 注意：不要清空子弹池！bulletsPool是静态成员，会被复用
    // 子弹池应该在程序结束时由操作系统自动回收

    for (Enemy *enemy : enemyDeadList)
    {
        delete enemy; // 释放敌人指针
    }
    enemyDeadList.clear();

    for (Enemy *enemy : dropList)
    {
        delete enemy; // 释放敌人指针
    }
    dropList.clear();

    if (bgmSound)
    {
        bgmSound->stop();
        delete bgmSound;
        bgmSound = nullptr;
    }
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
    // 初始化图片资源
    int playerWidth = 30;
    int playerHeight = 45;
    playerImage.push_back(loadAndProcessImage(":/images/roxy/RightWalk1.png", playerWidth, playerHeight)); // 玩家图片
    playerImage.push_back(loadAndProcessImage(":/images/roxy/RightWalk2.png", playerWidth, playerHeight)); // 玩家图片
    playerImage.push_back(loadAndProcessImage(":/images/roxy/LeftWalk1.png", playerWidth, playerHeight));  // 玩家图片
    playerImage.push_back(loadAndProcessImage(":/images/roxy/LeftWalk2.png", playerWidth, playerHeight));  // 玩家图片
    playerImage.push_back(loadAndProcessImage(":/images/roxy/backWalk1.png", playerWidth, playerHeight));  // 玩家图片
    playerImage.push_back(loadAndProcessImage(":/images/roxy/backWalk2.png", playerWidth, playerHeight));  // 玩家图片
    goblinImage.push_back(loadAndProcessImage(":/images/goblin1.png", 40, 40));                            // 右1
    goblinImage.push_back(loadAndProcessImage(":/images/goblin2.png", 40, 40));                            // 右2
    goblinImage.push_back(loadAndProcessImage(":/images/goblin3.png", 40, 40));                            // 左1
    goblinImage.push_back(loadAndProcessImage(":/images/goblin4.png", 40, 40));                            // 左2
    bulletImage.push_back(loadAndProcessImage(":/images/IceBall.png", 10, 10));                            // 冰
    bulletImage.push_back(loadAndProcessImage(":/images/RockBall.png", 10, 10));                           // 石头
    bulletImage.push_back(loadAndProcessImage(":/images/FireBall.png", 10, 10));                           // 火
    undeadMageImage = loadAndProcessImage(":/images/UndeadMage.png", 40, 40);                              // 敌人图片
    dropImage.push_back(loadAndProcessImage(":/images/drop_healing.png", 40, 40));                         // 掉落物图片
    dropImage.push_back(loadAndProcessImage(":/images/drop_attack.png", 40, 40));                          // 掉落物图片
    dropImage.push_back(loadAndProcessImage(":/images/drop_speed.png", 40, 40));                           // 掉落物图片
}
void GameWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);
    // 用缓存绘制地图
    painter.drawPixmap(0, 0, mapCache);

    painter.drawPixmap(player->getX(), player->getY(), playerImage[player->getCurrentImage()]);

    // 批量绘制敌人
    for (Enemy *enemy : enemies)
    {
        if (enemy->getEnemyType() == Enemy::Goblin)
            painter.drawPixmap(enemy->getX(), enemy->getY(), goblinImage[enemy->getCurrentImage()]);
        else if (enemy->getEnemyType() == Enemy::UndeadMage)
            painter.drawPixmap(enemy->getX(), enemy->getY(), undeadMageImage);
    }

    for (Bullet *bullet : bullets)
    {
        painter.drawPixmap(bullet->getX(), bullet->getY(), bulletImage[bullet->getCurrentImage()]);
    }

    // 绘制玩家血条（固定边框版本）
    int maxHP = gameData->playerData.hp;
    int currentHP = player->getHp();
    QRect healthBarRect(100, 10, 100, 20);                             // 固定边框尺寸
    int fillWidth = (currentHP * (healthBarRect.width() - 4)) / maxHP; // 计算填充宽度（保留2像素边距）

    // 绘制血条背景（固定边框）
    painter.setPen(Qt::black);
    painter.setBrush(QColor(50, 50, 50));
    painter.drawRect(healthBarRect);

    // 绘制动态血量（红色填充部分）
    painter.setBrush(Qt::red);
    painter.drawRect(healthBarRect.x() + 2, healthBarRect.y() + 2,
                     fillWidth, healthBarRect.height() - 4);

    // 绘制血量数值（保持右侧对齐）
    painter.setPen(Qt::white);
    painter.drawText(healthBarRect.right() - 45, healthBarRect.y() + 15,
                     QString("%1").arg(currentHP));

    // 绘制掉落物
    for (auto drop : dropList)
    {
        if (drop->getEnemyType() == Enemy::drop_healing)
            painter.drawPixmap(drop->getX(), drop->getY(), dropImage[0]);
        else if (drop->getEnemyType() == Enemy::drop_attack)
            painter.drawPixmap(drop->getX(), drop->getY(), dropImage[1]);
        else if (drop->getEnemyType() == Enemy::drop_speed)
            painter.drawPixmap(drop->getX(), drop->getY(), dropImage[2]);
    }
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;                      // 忽略重复按键事件
    keyPressed.insert(event->key()); // 记录按下的键
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
    bool isCollided = false; // 碰撞标志
    // 检查是否碰到转移魔法阵九宫格中间一格
    int magicCircleIndex = 0;
    for (const QRect &magicCircle : magicCircleList)
    {
        QRect NewMRect(magicCircle.x() + magicCircle.width() / 3, magicCircle.y() + magicCircle.height() / 3, magicCircle.width() / 3, magicCircle.height() / 3);
        if (newPlayerRect.intersects(NewMRect))
        {
            isCollided = true;
            break;
        }
        magicCircleIndex++;
    }
    if (isCollided)
    { // 转移到下一个列表中魔法阵的(奇数左边,偶数右边)
        int nextIndex = (magicCircleIndex + 1) % magicCircleList.size();
        if (nextIndex % 2 == 1)
            player->setPosition(magicCircleList[nextIndex].x() + magicCircleList[nextIndex].width() + 5, magicCircleList[nextIndex].y());
        else
            player->setPosition(magicCircleList[nextIndex].x() - player->width - 5, magicCircleList[nextIndex].y());
        return;
    }
    // 检查是否碰到掉落物
    for (auto drop : dropList)
    {
        QRect dropRect(drop->getX(), drop->getY(), drop->width, drop->height);
        if (newPlayerRect.intersects(dropRect) && drop->getHp() > 0)
        {
            isCollided = true;
            drop->setHP(0);
            handleDrop(drop);
            return;
        }
    }
    if (newPlayerRect.x() < 0 || newPlayerRect.x() + player->width > width() || newPlayerRect.y() < 0 || newPlayerRect.y() + player->height > height())
    {
        isCollided = true;
    }
    for (const QRect &obstacle : obstacles)
    {
        if (newPlayerRect.intersects(obstacle))
        {
            isCollided = true;
            break;
        }
    }
    if (!isCollided)
        player->setPosition(newPlayerPos.x(), newPlayerPos.y());

    if (left && !right)
        player->updateAnimation(Character::Left);
    else if ((right && !left) || down)
        player->updateAnimation(Character::Right);
    else if (up)
        player->updateAnimation(Character::Up);
}

void GameWindow::generateEnemy(Enemy::EnemyType type, std::vector<QPoint> *bornPos)
{
    if (type == Enemy::Goblin && goblinCount >= gameData->enemyData[0].totalNum)
    {
        goblinTimer->stop();
        return;
    }
    if (type == Enemy::UndeadMage && undeadMageCount >= gameData->enemyData[1].totalNum)
    {
        undeadMageTimer->stop();
        return;
    }
    if (type == Enemy::Goblin)
        goblinCount++;
    else if (type == Enemy::UndeadMage)
        undeadMageCount++;
    int randomIndex = rand() % bornPos->size();
    QPoint pos = (*bornPos)[randomIndex];
    int enemyIndex = 0;
    if (type == Enemy::Goblin)
        enemyIndex = 0;
    else if (type == Enemy::UndeadMage)
        enemyIndex = 1;
    else if (type == Enemy::drop_healing)
        enemyIndex = 3;
    else if (type == Enemy::drop_attack)
        enemyIndex = 4;
    else if (type == Enemy::drop_speed)
        enemyIndex = 5;
    Enemy *newEnemy = new Enemy(type, pos);
    newEnemy->setHP(gameData->enemyData[enemyIndex].hp);
    newEnemy->setDamage(gameData->enemyData[enemyIndex].damage);
    newEnemy->setAttackCD(gameData->enemyData[enemyIndex].CD);
    newEnemy->setSpeedF(&(gameData->enemyData[enemyIndex].speedF));
    newEnemy->setObstaclesList(&obstacles);
    if (type == Enemy::UndeadMage)
    {
        newEnemy->stopMove();
    }
    if (enemyIndex < 2)
        enemies.append(newEnemy);
    else
    {
        dropList.append(newEnemy);
    }
    // 连接敌人的位置改变信号到游戏窗口的更新函数
    connect(newEnemy, &Enemy::createBullet, this, [this](Bullet *bullet)
            {bullets.append(bullet);
    connect(bullet, &Bullet::hitSth, this, [this, bullet](){bullets.removeOne(bullet);}); });
}

bool isInRange(int x1, int y1, int x2, int y2, int range)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= range * range;
}

void GameWindow::handlePlayerAttack()
{
    Enemy *target = nullptr;
    for (Enemy *enemy : enemies)
    {
        if (isInRange(player->getX(), player->getY(), enemy->getX(), enemy->getY(), player->getWeapon()->getRange()))
        {
            target = enemy;
            break;
        }
    }
    if (!target)
        return;

    int step = 8;
    double deltaX = target->getX() - player->getX();
    double deltaY = target->getY() - player->getY();
    double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
    double cosTheta = deltaX / distance;
    double sinTheta = deltaY / distance;

    int dx = step * cosTheta; // 速度的水平分量
    int dy = step * sinTheta; // 速度的垂直分量

    Bullet *bullet = Bullet::createBullet(player->getWeapon()->getBulletType(), player->getX() + player->width / 2, player->getY() + player->height / 2, dx, dy, target);
    bullet->setInformation(player->getWeapon()->getDamage(), &enemies, &obstacles);
    bullet->startMove(gameData->playerData.weaponData.bulletSpeedF);
    bullets.append(bullet); // 将子弹添加到列表中
    connect(bullet, &Bullet::hitSth, this, [this, bullet]()
            { bullets.removeOne(bullet); });
}

void GameWindow::handleEnemyDead()
{
    if (enemies.isEmpty() && goblinCount == gameData->enemyData[0].totalNum && undeadMageCount == gameData->enemyData[1].totalNum)
    {
        deadTimer->stop();
        QMessageBox::information(this, "消灭了所有敌人", "这里也没有出口....");
        if (gameData->level == 1)
        {
            emit pass_1();
        }
        this->close();
        this->deleteLater();
        return;
    }
    if (enemies.isEmpty())
        return;
    for (auto it = enemies.begin(); it != enemies.end();)
    {
        if ((*it)->getHp() <= 0)
        {
            (*it)->setPosition(0, 0);
            (*it)->stopMove();
            enemyDeadList.append(*it); // 将敌人添加到死亡列表中
            it = enemies.erase(it);    // 从列表中移除敌人
        }
        else
        {
            ++it; // 继续遍历下一个敌人
        }
    }
    // 处理已利用道具, 移到deadlist
    // 处理重复的掉落物, 移到deadlist
    int numhealing = 0, numattack = 0, numspeed = 0;
    for (auto it = dropList.begin(); it != dropList.end();)
    {
        if ((*it)->getEnemyType() == Enemy::drop_healing)
            numhealing++;
        else if ((*it)->getEnemyType() == Enemy::drop_attack)
            numattack++;
        else if ((*it)->getEnemyType() == Enemy::drop_speed)
            numspeed++;
        if ((*it)->getHp() <= 0 || (numhealing > 1 || numattack > 1 || numspeed > 1))
        { // 如果敌人血量小于等于0
            (*it)->setPosition(0, 0);
            (*it)->stopMove();
            enemyDeadList.append(*it); // 将敌人添加到死亡列表中
            if ((*it)->getEnemyType() == Enemy::drop_healing)
            {
                numhealing--;
            }
            else if ((*it)->getEnemyType() == Enemy::drop_attack)
            {
                numattack--;
            }
            else if ((*it)->getEnemyType() == Enemy::drop_speed)
            {
                numspeed--;
            }
            it = dropList.erase(it); // 从列表中移除敌人
        }
        else
        {
            ++it; // 继续遍历下一个敌人
        }
    }
}
void GameWindow::handlePlayerDead()
{
    if (gameData->level == 1 && player && player->getHp() <= 0)
    {
        stopGame();
        QMessageBox::information(this, "成功逃脱", "好险,差一点就死透了"); // 显示游戏结束消息
        emit pass_1();
        emit gameFinished();
    }
    else if (gameData->level == 2 && player && player->getHp() <= 100)
    {
        // 场上所有敌人立即死亡
        for (Enemy *enemy : enemies)
        {
            enemy->setHP(0);
        }
        // 清空场上所有子弹
        for (auto it = bullets.begin(); it != bullets.end();)
        {
            Bullet::deleteBullet(*it);
            it = bullets.erase(it);
        }
        handleEnemyDead();
        stopGame();
        // 3秒后发送通关信号
        QTimer::singleShot(3000, this, [this]()
                           {
                            emit pass_2();
                            emit gameFinished(); });
    }
}

void GameWindow::createMapCache(std::vector<std::vector<int>> *grid)
{
    int tileWidth = 10;
    int tileHeight = 10;
    floorTile = QPixmap(":/images/floor.png").scaled(100, 100);
    obstacleTile = QPixmap(":/images/Obstacle.png").scaled(tileWidth, tileHeight);
    for (int i = 0; i < 80; ++i)
    {
        for (int j = 0; j < 60; ++j)
        {
            if ((*grid)[i][j])
            {
                obstacles.append(QRect(i * 10, j * 10, 10, 10));
            }
        }
    }
    mapCache = QPixmap(grid->size() * tileWidth, grid->at(0).size() * tileHeight);
    mapCache.fill(Qt::transparent); // 填充透明背景
    QPainter painter(&mapCache);

    // 绘制16*12的地板
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            painter.drawPixmap(i * 100, j * 100, floorTile);
        }
    }
    for (int i = 0; i < grid->size(); ++i)
    {
        for (int j = 0; j < grid->at(i).size(); ++j)
        {
            if ((*grid)[i][j] == 1)
            { // 如果是墙壁
                painter.drawPixmap(i * tileWidth, j * tileHeight, obstacleTile);
            }
        }
    }
    // 根据enemyData[2]绘制魔法阵
    for (int i = 0; i < gameData->enemyData[2].totalNum; i++)
    {
        magicCircleList.append(QRect(gameData->enemyData[2].generatePos[i].x(), gameData->enemyData[2].generatePos[i].y(), 50, 50));
    }
    magicCircleImage = loadAndProcessImage(":/images/MagicCircle.png", 50, 50); // 加载并处理魔法阵图片
    for (int i = 0; i < magicCircleList.size(); i++)
    {
        painter.drawPixmap(magicCircleList[i].x(), magicCircleList[i].y(), magicCircleImage);
    }
}

void GameWindow::handleDrop(Enemy *drop)
{
    if (drop->getEnemyType() == Enemy::drop_healing)
    {
        player->setHP(player->getHp() + 100);
    }
    else if (drop->getEnemyType() == Enemy::drop_attack)
    {
        QTimer *timer = new QTimer(this);
        // 每100ms发动一次attack
        connect(timer, &QTimer::timeout, this, [this]()
                { attackByDrop(); });
        timer->start(100);
        // 4秒后结束
        QTimer::singleShot(4000, timer, &QTimer::stop);
    }
    else if (drop->getEnemyType() == Enemy::drop_speed)
    {
        movementTimer->start(10);
        QTimer::singleShot(7000, movementTimer, [this]()
                           { movementTimer->start(16); });
    }
    drop->setHP(0);
}
void GameWindow::attackByDrop()
{
    int dir[8][2] = {
        {8, 0},
        {6, 6},
        {0, 8},
        {-6, 6},
        {-8, 0},
        {-6, -6},
        {0, -8},
        {6, -6}};

    for (int i = 0; i < 8; ++i)
    {
        Bullet *bullet = Bullet::createBullet(player->getWeapon()->getBulletType(), player->getX() + player->width / 2, player->getY() + player->height / 2, dir[i][0], dir[i][1], nullptr);
        bullet->setInformation(player->getWeapon()->getDamage(), &enemies, nullptr);
        bullet->startMove(10);
        bullets.append(bullet);
        connect(bullet, &Bullet::hitSth, this, [this, bullet]()
                { bullets.removeOne(bullet); });
    }
}

void GameWindow::closeEvent(QCloseEvent *event)
{
    if (bgmSound)
    {
        bgmSound->stop();
    }

    QWidget::closeEvent(event);

    // 在事件循环的下一个周期删除自己
    this->deleteLater();
}
