#include "MyApp.h"
#include "plot.h"
#include "store.h"
#include "savemanager.h"
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QDialog>
#include <QComboBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QTextStream>
#include <QListWidget>

MyApp::MyApp(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui_MyApp),
      isPlotBeforeGame(false)

{
    // 初始化剧情数据
    initPlotData();

    loadData();
    ui->setupUi(this);
    this->setWindowTitle("转移迷宫历险记");
    this->setFixedSize(1200, 800); // 放大窗口以适应新的游戏窗口

    // 初始化StackedWidget
    setupStackedWidget();

    // 显示主菜单
    showMainMenu();
}

MyApp::~MyApp()
{
    delete ui;
}

void MyApp::initPlotData()
{
    // 初始化关卡1剧情
    _plotData_1.setPlotId(1);
    _plotData_1.setBackgroundPath(":/images/plot_1BG.jpg");
    _plotData_1.addText("洛琪希:............");
    _plotData_1.addText("明明有好好提醒自己...还是踩到了魔法阵陷阱.....");
    _plotData_1.addText(".....而且还是最危险的随机转移魔法阵.....");
    _plotData_1.addText(".....看来保罗他们应该找不到这里吧.....");
    _plotData_1.addText("我目前呆着的封闭空间,\n只有两个定向转移的魔法阵不知道通往何处");
    _plotData_1.addText("......在这个传闻中最危险的转移迷宫里,我还能活着回去吗......");
    _plotData_1.addText("......还能顺利救出鲁迪的母亲吗......");
    _plotData_1.addText("......上次和爸爸妈妈约好至少20年回家一趟来着......");

    // 初始化关卡2剧情
    _plotData_2.setPlotId(2);
    _plotData_2.setBackgroundPath(":/images/plot_2BG.png");
    _plotData_2.addText("鲁迪:冰结领域!");
    _plotData_2.addText("原本充斥着哥布林血腥味和吼叫声的迷宫在一瞬间变成冰窟,\n魔物都在瞬间被晶莹透亮的冰块封住......");
    _plotData_2.addText("鲁迪:洛琪希师傅......!你没事真是太好了.....");
    _plotData_2.addText("洛琪希:咦?");
}

void MyApp::setupStackedWidget()
{
    // 创建StackedWidget
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setGeometry(0, 0, 1200, 800); // 放大以适应新窗口

    // 创建主菜单Widget
    createMainMenuWidget();

    // 创建剧情Widget
    plotWidget = new Plot(this);
    connect(plotWidget, &Plot::plotFinished, this, &MyApp::onPlotFinished);

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
    // 连接NPC任务奖励信号
    connect(gameWidget, &GameWindow::crystalReward, this, [this](int amount) {
        numOfMagicCrystal += amount;
    });

    // 创建商店Widget
    storeWidget = new Store(this);

    // 添加到StackedWidget
    stackedWidget->addWidget(mainMenuWidget); // index 0
    stackedWidget->addWidget(plotWidget);     // index 1
    stackedWidget->addWidget(gameWidget);     // index 2
}

void MyApp::createMainMenuWidget()
{
    mainMenuWidget = new QWidget(this);
    mainMenuWidget->setFixedSize(1200, 800); // 放大主菜单

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
        {StartGame, {0.85, 0.30}},
        {SaveGame, {0.85, 0.42}},
        {LoadGame, {0.85, 0.54}},
        {ExitGame, {0.85, 0.66}},
        {StoreButton, {0.85, 0.78}}};
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
        case SaveGame:
            connect(button, &QPushButton::clicked, this, &MyApp::onSaveGameClicked);
            break;
        case LoadGame:
            connect(button, &QPushButton::clicked, this, &MyApp::onLoadGameClicked);
            break;
        case ExitGame:
            connect(button, &QPushButton::clicked, this, &MyApp::onExitGameClicked);
            break;
        case StoreButton:
            connect(button, &QPushButton::clicked, this, &MyApp::onStoreClicked);
            break;
        }
        // 设置按钮文本
        switch (config.first)
        {
        case StartGame:
            button->setText("开始冒险");
            break;
        case SaveGame:
            button->setText("保存游戏");
            break;
        case LoadGame:
            button->setText("读取存档");
            break;
        case ExitGame:
            button->setText("退出");
            break;
        case StoreButton:
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
            showPlot(&_plotData_1);
        }
        else
        {
            // 直接显示游戏
            showGame(&_data);
        }
    }
}
void MyApp::onSaveGameClicked()
{
    // 显示保存游戏对话框
    showSaveLoadDialog(true);
}

void MyApp::onLoadGameClicked()
{
    // 显示读取存档对话框
    showSaveLoadDialog(false);
}

void MyApp::onExitGameClicked()
{
    QApplication::quit();
}

void MyApp::onStoreClicked()
{
    // 设置商店数据并显示
    storeWidget->setStoreData(&numOfMagicCrystal, &_weaponData_1, &_weaponData_2);
    storeWidget->exec();
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

// 新存档系统实现
void MyApp::saveDataToSlot(int slot)
{
    SaveData saveData;
    saveData.saveName = QString("存档 %1").arg(slot + 1);
    saveData.magicCrystal = numOfMagicCrystal;
    saveData.playerData = _playerData;
    saveData.weaponData_1 = _weaponData_1;
    saveData.weaponData_2 = _weaponData_2;
    saveData.passData_1_isPass = _passData_1.isPass;
    saveData.passData_2_isPass = _passData_2.isPass;

    if (SaveManager::saveGame(saveData, slot))
    {
        currentSaveSlot = slot;
        QMessageBox::information(this, "保存成功", QString("游戏已保存到槽位 %1").arg(slot + 1));
    }
    else
    {
        QMessageBox::warning(this, "保存失败", "无法保存游戏数据");
    }
}

bool MyApp::loadDataFromSlot(int slot)
{
    SaveData saveData;
    if (SaveManager::loadGame(saveData, slot))
    {
        numOfMagicCrystal = saveData.magicCrystal;
        _playerData = saveData.playerData;
        _weaponData_1 = saveData.weaponData_1;
        _weaponData_2 = saveData.weaponData_2;
        _passData_1.isPass = saveData.passData_1_isPass;
        _passData_2.isPass = saveData.passData_2_isPass;
        currentSaveSlot = slot;

        QMessageBox::information(this, "读取成功", QString("已从槽位 %1 读取存档").arg(slot + 1));
        return true;
    }
    else
    {
        QMessageBox::warning(this, "读取失败", "无法读取存档数据");
        return false;
    }
}

void MyApp::showSaveLoadDialog(bool isSave)
{
    QDialog dialog(this);
    dialog.setWindowTitle(isSave ? "保存游戏" : "读取存档");
    dialog.setFixedSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titleLabel = new QLabel(isSave ? "选择存档槽位保存:" : "选择存档槽位读取:", &dialog);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    layout->addWidget(titleLabel);

    QListWidget *listWidget = new QListWidget(&dialog);

    // 添加3个存档槽位
    for (int i = 0; i < 3; i++)
    {
        QString itemText = QString("槽位 %1: ").arg(i + 1);
        if (SaveManager::hasSave(i))
        {
            itemText += SaveManager::getSaveInfo(i);
        }
        else
        {
            itemText += "空存档";
        }
        listWidget->addItem(itemText);
    }

    layout->addWidget(listWidget);

    QPushButton *confirmBtn = new QPushButton(isSave ? "保存" : "读取", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(confirmBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(confirmBtn, &QPushButton::clicked, [&]()
    {
        int selectedSlot = listWidget->currentRow();
        if (selectedSlot >= 0)
        {
            if (isSave)
            {
                saveDataToSlot(selectedSlot);
            }
            else
            {
                if (SaveManager::hasSave(selectedSlot))
                {
                    loadDataFromSlot(selectedSlot);
                }
                else
                {
                    QMessageBox::warning(this, "读取失败", "该槽位没有存档");
                    return;
                }
            }
            dialog.accept();
        }
        else
        {
            QMessageBox::warning(this, "提示", "请选择一个存档槽位");
        }
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MyApp::closeEvent(QCloseEvent *event)
{
    // 保存数据到文件（使用旧系统保持兼容）
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

void MyApp::showPlot(PlotData *plotData, bool beforeGame)
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
    plotWidget->setPlotContent(plotData->getTextListPtr(), plotData->getBackgroundPathCopy());
    // 切换到剧情页面
    stackedWidget->setCurrentIndex(1);
}

void MyApp::showGame(GameData *data)
{
    // 设置游戏数据
    gameWidget->setGameData(data);
    // 切换到游戏页面
    stackedWidget->setCurrentIndex(2);
    // 设置焦点到游戏窗口，确保能接收键盘事件
    gameWidget->setFocus();
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
        showPlot(&_plotData_2, false);
    }
    else
    {
        // 返回主菜单
        showMainMenu();
    }
}
