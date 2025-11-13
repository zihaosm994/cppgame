#include "MyApp.h"
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QDialog>
#include <QComboBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include "plot.h"
#include <QTextStream>
std::vector<std::string> plot_1Text = {
    "洛琪希:............",
    "明明有好好提醒自己...还是踩到了魔法阵陷阱.....",
    ".....而且还是最危险的随机转移魔法阵.....",
    ".....看来保罗他们应该找不到这里吧.....",
    "我目前呆着的封闭空间,\n只有两个定向转移的魔法阵不知道通往何处",
    "......在这个传闻中最危险的转移迷宫里,我还能活着回去吗......",
    "......还能顺利救出鲁迪的母亲吗......",
    "......上次和爸爸妈妈约好至少20年回家一趟来着......"};
std::string plot_1ImagePath = ":/images/plot_1BG.jpg";
std::vector<std::string> plot_2Text = {
    "鲁迪:冰结领域!",
    "原本充斥着哥布林血腥味和吼叫声的迷宫在一瞬间变成冰窟,\n魔物都在瞬间被晶莹透亮的冰块封住......",
    "鲁迪:洛琪希师傅......!你没事真是太好了.....",
    "洛琪希:咦?"};
std::string plot_2ImagePath = ":/images/plot_2BG.png";

MyApp::MyApp(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui_MyApp),
      isPlotBeforeGame(false)

{
    loadData();
    ui->setupUi(this);
    this->setWindowTitle("转移迷宫历险记");
    this->setFixedSize(800, 600);

    // 初始化StackedWidget
    setupStackedWidget();

    // 显示主菜单
    showMainMenu();
}

MyApp::~MyApp()
{
    delete ui;
}

void MyApp::setupStackedWidget()
{
    // 创建StackedWidget
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setGeometry(0, 0, 800, 600);

    // 创建主菜单Widget
    createMainMenuWidget();

    // 创建剧情Widget
    plotWidget = new Plot(this);
    connect(plotWidget, &Plot::plotFinished, this, &MyApp::onPlotFinished);
`
    // 创建游戏Widget
    gameWidget = new GameWindow(this);
    connect(gameWidget, &GameWindow::pass_1, this, [this]()
            {
        _passData_1.isPass = true;
        numOfMagicCrystal += 1145; });
    connect(gameWidget, &GameWindow::pass_2, this, [this]()
            {
        _passData_2.isPass = true;
        numOfMagicCrystal += 114514; });
    connect(gameWidget, &GameWindow::gameFinished, this, &MyApp::onGameFinished);

    // 添加到StackedWidget
    stackedWidget->addWidget(mainMenuWidget); // index 0
    stackedWidget->addWidget(plotWidget);     // index 1
    stackedWidget->addWidget(gameWidget);     // index 2
}

void MyApp::createMainMenuWidget()
{
    mainMenuWidget = new QWidget(this);
    mainMenuWidget->setFixedSize(800, 600);

    // 设置背景
    updateBackground();

    // 创建按钮
    createGameButtons();
}

void MyApp::updateBackground()
{
    // 加载背景图片
    QPixmap bg(":/images/Background.png");
    // 按比例缩放至填满窗口（可能裁剪边缘）
    QPixmap scaledBg = bg.scaled(mainMenuWidget->size(), Qt::KeepAspectRatioByExpanding);
    // 创建调色板并设置背景
    QPalette palette;
    palette.setBrush(QPalette::Window, scaledBg);
    mainMenuWidget->setPalette(palette);
    // 启用自动填充背景
    mainMenuWidget->setAutoFillBackground(true);
}

// 创建游戏按钮
void MyApp::createGameButtons()
{
    // 按键配置参数:按键类型，x坐标比例，y坐标比例
    const QList<QPair<ButtonType, QPair<double, double>>> buttonConfig = {
        {StartGame, {0.85, 0.4}},
        {ExitGame, {0.85, 0.55}},
        {Store, {0.85, 0.7}}};
    for (const auto &config : buttonConfig)
    {
        QPushButton *button = new QPushButton(mainMenuWidget);
        // 设置按钮通用样式
        button->setFocusPolicy(Qt::NoFocus);
        button->setStyleSheet(R"(
        QPushButton {
            font-size:2.2vh;    //字体大小
            min-width: 120px;   //最小宽度
            min-height: 40px;   //最小高度
            border-radius: 8px; //圆角半径
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,  //渐变背景
                stop:0 #6C8EBF, stop:1 #4B6A9D);             //起始颜色和终止颜色
            color: white;                                         //字体颜色
            padding: 8px;                                          //内边距
        }
        QPushButton:hover { background: #5B7CAD; }               //鼠标悬停时的背景颜色
        QPushButton:pressed { background: #4A6B9C; }             //鼠标按下时的背景颜色
    )");
        // 设置按钮大小和位置
        const int btnWidth = mainMenuWidget->width() * 0.25; // 按钮宽度
        const int btnHeight = mainMenuWidget->height() * 0.08;
        const int xPos = mainMenuWidget->width() * config.second.first - btnWidth / 2;    // 按钮x坐标
        const int yPos = mainMenuWidget->height() * config.second.second - btnHeight / 2; // 按钮y坐标
        button->setGeometry(xPos, yPos, btnWidth, btnHeight);                             // 设置按钮位置和大小
        // 按钮点击事件
        switch (config.first)
        {
        case StartGame:
            connect(button, &QPushButton::clicked, this, &MyApp::onStartGameClicked);
            break;
        case ExitGame:
            connect(button, &QPushButton::clicked, this, &MyApp::onExitGameClicked);
            break;
        case Store:
            connect(button, &QPushButton::clicked, this, &MyApp::onStoreClicked);
            break;
        }
        // 设置按钮文本
        switch (config.first)
        {
        case StartGame:
            button->setText("开始冒险");
            break;
        case ExitGame:
            button->setText("退出");
            break;
        case Store:
            button->setText("魔法商店");
            break;
        }

        gameButtons.insert(config.first, button);
    }
}

// 按钮点击事件函数实现
void MyApp::onStartGameClicked()
{
    // 新增选择对话框
    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle("一定要在迷宫中活下来!");
    selectionDialog.setFixedSize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(&selectionDialog);

    // 关卡选择
    QLabel *levelLabel = new QLabel("选择前路");
    QComboBox *levelCombo = new QComboBox();
    levelCombo->addItem("眼前的转移魔法阵");
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(levelCombo->model());
    if (_passData_1.isPass)
    {
        levelCombo->addItem("远处的魔法阵");
    }
    else
    {
        QStandardItem *item = new QStandardItem("还是先去眼前的魔法阵吧");
        item->setEnabled(false);
        model->appendRow(item);
    }

    // 子弹类型选择
    QLabel *bulletLabel = new QLabel("选择武器:");
    QComboBox *bulletCombo = new QComboBox();
    bulletCombo->addItem("洛琪希的法杖(冰霜击)", QVariant(IceBall));
    bulletCombo->addItem("傲慢水龙王(岩炮弹)", QVariant(RockBall));

    QPushButton *confirmBtn = new QPushButton("踏上转移魔法阵");

    // 添加控件到布局
    layout->addWidget(levelLabel);
    layout->addWidget(levelCombo);
    layout->addWidget(bulletLabel);
    layout->addWidget(bulletCombo);
    layout->addWidget(confirmBtn);

    // 处理确认选择
    connect(confirmBtn, &QPushButton::clicked, [&]()
            {
        if(levelCombo->currentIndex() == 1&&!_passData_1.isPass){
            QMessageBox::warning(this, "感觉有点危险", "还是先去眼前的魔法阵吧");
            return;
        }
        // 更新武器类型
        if(bulletCombo->currentIndex() == 0){
            _playerData.weaponData=_weaponData_1;
        } else {
            _playerData.weaponData=_weaponData_2;
        }
        // 根据关卡选择设置数据
        if(levelCombo->currentIndex() == 0){
            _data = {
                1,
                _passData_1.grid,
                _playerData,
                _passData_1.enemyData
            };
        } else {
            _data = {
                2,
                _passData_2.grid,
                _playerData,
                _passData_2.enemyData
            };
        }
        
        selectionDialog.accept(); });

    if (selectionDialog.exec() == QDialog::Accepted)
    {
        // 如果选择近处的魔法阵, 则先显示剧情
        if (levelCombo->currentIndex() == 0)
        {
            // 先显示剧情，剧情结束后显示游戏
            showPlot(&plot_1Text, plot_1ImagePath);
        }
        else
        {
            // 直接显示游戏
            showGame(&_data);
        }
    }
}
void MyApp::onExitGameClicked()
{
    QApplication::quit();
}

void MyApp::onStoreClicked()
{
    // 创建商店对话框
    QDialog storeDialog(this);
    storeDialog.setWindowTitle("魔法商店");
    storeDialog.setFixedSize(600, 400);

    QHBoxLayout *mainlayout = new QHBoxLayout(&storeDialog);

    QLabel *imageLabel = new QLabel(&storeDialog);
    QPixmap roxyImage(":/images/RoxyWhole.png");
    imageLabel->setPixmap(roxyImage.scaled(200, 400, Qt::KeepAspectRatio));
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setFixedSize(200, 400);
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // 显示当前水晶数量
    QLabel *crystalLabel = new QLabel(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
    rightLayout->addWidget(crystalLabel);

    // 角色升级按钮
    QPushButton *upgradeHealthBtn = new QPushButton("升级生命值 (100水晶)");
    QPushButton *upgradeSpeedBtn = new QPushButton("提升速度 (120水晶)");
    QPushButton *upgradeWeaponBtn_1 = new QPushButton("升级洛琪希的法杖 (150水晶)(cd更短)");
    QPushButton *upgradeWeaponBtn_2 = new QPushButton("升级傲慢水龙王 (150水晶)(伤害更高)");
    QPushButton *upgradeBulletSpeedBtn = new QPushButton("提升射速 (180水晶)");

    // 添加按钮到布局
    rightLayout->addWidget(upgradeHealthBtn);
    rightLayout->addWidget(upgradeSpeedBtn);
    rightLayout->addWidget(upgradeWeaponBtn_1);
    rightLayout->addWidget(upgradeWeaponBtn_2);
    rightLayout->addWidget(upgradeBulletSpeedBtn);
    mainlayout->addWidget(imageLabel);
    mainlayout->addLayout(rightLayout);
    // 连接按钮信号
    connect(upgradeHealthBtn, &QPushButton::clicked, [&]()
            {
        if(numOfMagicCrystal >= 100){
            numOfMagicCrystal -= 100;
            _playerData.hp += 1145; 
            crystalLabel->setText(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
            QMessageBox::information(this, "升级成功", "最大生命值+1145！");
        }
        else{
            QMessageBox::warning(this, "购买失败", "魔法水晶不足！");
        } });

    connect(upgradeSpeedBtn, &QPushButton::clicked, [&]()
            {
        if(numOfMagicCrystal >= 120&&_playerData.step<8){
            numOfMagicCrystal -= 120;
            _playerData.step += 1;
            crystalLabel->setText(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
            QMessageBox::information(this, "升级成功", "移动速度+1！");
        }
        else{
            QMessageBox::warning(this, "购买失败", "魔法水晶不足！(或移动速度已满)");
        } });
    connect(upgradeWeaponBtn_1, &QPushButton::clicked, [&]()
            {
        if(numOfMagicCrystal >= 150&&_weaponData_1.level<3){
            numOfMagicCrystal -= 150;
            _weaponData_1.level += 1;
            crystalLabel->setText(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
            QMessageBox::information(this, "升级成功", "武器等级+1！");
        }
        else{
            QMessageBox::warning(this, "购买失败", "魔法水晶不足！(或武器等级已满)");
        } });
    connect(upgradeWeaponBtn_2, &QPushButton::clicked, [&]()
            {
        if(numOfMagicCrystal >= 150&&_weaponData_2.level<3){
            numOfMagicCrystal -= 150;
            _weaponData_2.level += 1;
            crystalLabel->setText(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
            QMessageBox::information(this, "升级成功", "武器等级+1！");
        }
        else{
            QMessageBox::warning(this, "购买失败", "魔法水晶不足！(或武器等级已满)");
        } });
    connect(upgradeBulletSpeedBtn, &QPushButton::clicked, [&]()
            {
        if(numOfMagicCrystal >= 180&&_playerData.weaponData.bulletSpeedF>5){
            numOfMagicCrystal -= 180;
            _playerData.weaponData.bulletSpeedF -= 1;
            crystalLabel->setText(QString("当前魔法水晶: %1").arg(numOfMagicCrystal));
            QMessageBox::information(this, "升级成功", "子弹射速+1！");
        }
        else{
            QMessageBox::warning(this, "购买失败", "魔法水晶不足！(或子弹射速已满)");
        } });
    // 显示对话框
    storeDialog.exec();
}

void MyApp::saveData()
{
    QFile file("data.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法打开文件";
        return;
    }
    QTextStream out(&file);
    out << numOfMagicCrystal << "\n";
    out << _playerData << "\n";
    out << _weaponData_1 << "\n";
    out << _weaponData_2 << "\n";
    // 存储通关数据将布尔变量转换为整数
    out << static_cast<int>(_passData_1.isPass) << "\n";
    out << static_cast<int>(_passData_2.isPass) << "\n";
    // 关闭文件
    file.close();
}
void MyApp::loadData()
{
    QFile file("data.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "无法打开文件";
        return;
    }
    QTextStream in(&file);
    in >> numOfMagicCrystal;
    in >> _playerData;
    in >> _weaponData_1;
    in >> _weaponData_2;
    // 读取布尔变量
    int isPass1, isPass2;
    in >> isPass1 >> isPass2;
    _passData_1.isPass = static_cast<bool>(isPass1);
    _passData_2.isPass = static_cast<bool>(isPass2);
    // 关闭文件
    file.close();
}
void MyApp::closeEvent(QCloseEvent *event)
{
    // 保存数据到文件
    saveData();
    event->accept(); // 允许关闭事件
}

// 页面切换槽函数实现
void MyApp::showMainMenu()
{
    // 确保游戏已停止
    gameWidget->stopGame();
    // 清理按键状态，避免残留
    gameWidget->clearKeyState();
    // 切换到主菜单
    stackedWidget->setCurrentIndex(0);
}

void MyApp::showPlot(std::vector<std::string> *textList, std::string bgPath, bool beforeGame)
{
    // 如果是游戏后的剧情，确保游戏已停止
    if (!beforeGame)
    {
        gameWidget->stopGame();
    }

    // 记录剧情是在游戏前还是游戏后
    isPlotBeforeGame = beforeGame;

    // 重置并设置剧情内容
    plotWidget->reset();
    plotWidget->setPlotContent(textList, bgPath);
    // 切换到剧情页面
    stackedWidget->setCurrentIndex(1);
}

void MyApp::showGame(GameData *data)
{
    // 设置游戏数据
    gameWidget->setGameData(data);
    // 切换到游戏页面
    stackedWidget->setCurrentIndex(2);
}

void MyApp::onPlotFinished()
{
    // 根据标志判断剧情结束后的行为
    if (isPlotBeforeGame)
    {
        // 如果是游戏前的剧情，则显示游戏
        showGame(&_data);
    }
    else
    {
        // 如果是游戏后的剧情，则返回主菜单
        showMainMenu();
    }
}

void MyApp::onGameFinished()
{
    // 游戏结束后，根据关卡判断是否显示剧情2
    if (_data.level == 2 && _passData_2.isPass)
    {
        // 第二关通关后显示剧情2（传入false表示这是游戏后的剧情）
        showPlot(&plot_2Text, plot_2ImagePath, false);
    }
    else
    {
        // 返回主菜单
        showMainMenu();
    }
}
