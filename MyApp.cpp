#include "MyApp.h"
#include <QString>
#include "GameWindow.h"
#include "plot.h"
#include "JsonLoader.h"
MyApp::MyApp(QWidget *parent)
    :QMainWindow(parent),
    ui(new Ui_MyApp)
{
    ui->setupUi(this);
    this->setWindowTitle("??????");
    this->setMinimumSize(800,600);
    stack =new QStackedWidget(this);
    setCentralWidget(stack);
    Plot *plot = new Plot();
    stack->addWidget(plot);
    MainMenu * menu = new MainMenu();
    stack->addWidget(menu);
    GameWindow *game = new GameWindow();
    connect(game,&GameWindow::gameFinished,this,[=](){
        stack->setCurrentWidget(menu);
        menu->setFocus();
    });
    connect(game,&GameWindow::plotStart,this,[=](int id){
        game->stopGame();
        static std::vector<PlotData> plots = *JsonPlotLoader::loadPlot(":/allGameData/PlotData.json"); //JsonPlotLoader::loadPlot(":/allGameData/PlotData.json")*TestData::createDefaultPlotData()
        plot->setPlotContent(&((plots)[id].imagePath_texts));
        stack->setCurrentWidget(plot);
        plot->setFocus();
    });
    stack->addWidget(game);
    connect(menu,&MainMenu::startGame,this,[=](){
        game->setGameData(TestData::createDefaultGameData()); //JsonGameLoader::loadGame(":/allGameData/GameData.json"
        stack->setCurrentWidget(game);
        game->setFocus();
    });
    connect(plot,&Plot::plotFinished,this,[=](){
        plot->reset();
        stack->setCurrentWidget(game);
        game->startGame();
    });

    stack->setCurrentWidget(menu);
}
MyApp::~MyApp(){
    delete ui;
}

MainMenu::MainMenu(QWidget * parent):
    QWidget(parent)
{
    this->setWindowTitle("????");
    this->setMinimumSize(800,600);
    setBackground(":/images/Background.png");
    createButtons();
}
void MainMenu::setBackground(QString path)
{
    // 加载背景图片
    QPixmap bg((QString(path)));
    // 按比例缩放至填满窗口（可能裁剪边缘）
    QPixmap scaledBg = bg.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
    // 创建调色板并设置背景
    QPalette palette;
    palette.setBrush(QPalette::Window, scaledBg);
    this->setPalette(palette);
    // 启用自动填充背景
    this->setAutoFillBackground(true);
    this->setFocusPolicy(Qt::StrongFocus);
}
MainMenu::~MainMenu(){
    for(auto button:buttonList){
        delete button;
    }
}
void MainMenu:: createButtons(){
    enum ButtonType{
        StartGame
    };

    // 按键配置参数:按键类型，x坐标比例，y坐标比例
    const QList<QPair<ButtonType, QPair<double, double>>> buttonConfig =
        {{StartGame, {0.85, 0.4}}};
    for (const auto &config : buttonConfig)
    {
        QPushButton *button = new QPushButton(this);
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
        const int btnWidth = width() * 0.25; // 按钮宽度
        const int btnHeight = height() * 0.08;
        const int xPos = width() * config.second.first - btnWidth / 2;    // 按钮x坐标
        const int yPos = height() * config.second.second - btnHeight / 2; // 按钮y坐标
        button->setGeometry(xPos, yPos, btnWidth, btnHeight);             // 设置按钮位置和大小
        // 按钮点击事件
        switch (config.first)
        {
        case StartGame:
            button->setText("开始冒险");
            connect(button, &QPushButton::clicked, this, [=](){
                emit startGame();
            });
            break;
        }
        buttonList.push_back(button);
    }
}

void MainMenu:: resizeEvent(QResizeEvent * event){
    enum ButtonType{
        StartGame
    };
    // 按键配置参数:按键类型，x坐标比例，y坐标比例
    const QList<QPair<ButtonType, QPair<double, double>>> buttonConfig =
        {{StartGame, {0.85, 0.4}}};
    int i=0;
    for (const auto &button : buttonList)
    {

        // 设置按钮大小和位置
        const int btnWidth = width() * 0.25; // 按钮宽度
        const int btnHeight = height() * 0.08;
        const int xPos = width() *buttonConfig[i].second.first - btnWidth / 2;    // 按钮x坐标
        const int yPos = height() * buttonConfig[i].second.second - btnHeight / 2; // 按钮y坐标
        button->setGeometry(xPos, yPos, btnWidth, btnHeight);             // 设置按钮位置和大小
        i++;
    }
}


