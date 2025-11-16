#pragma once
#include "ui_MyApp.h"
#include "GameWindow.h"
#include "store.h"
#include <QMainWindow>
#include <QPushButton>
#include <QVboxLayout>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QFile>
#include <QStyleOption>
#include <QObject>
#include <QStackedWidget>

// 前向声明
class Plot;

class MyApp : public QMainWindow
{
    Q_OBJECT

public:
    MyApp(QWidget *parent = nullptr);
    ~MyApp();

private:
    Ui_MyApp *ui;

    // 页面管理
    QStackedWidget *stackedWidget; // 用于管理不同页面的切换
    QWidget *mainMenuWidget;       // 主菜单页面
    Plot *plotWidget;              // 剧情页面
    GameWindow *gameWidget;        // 游戏页面
    Store *storeWidget;            // 商店页面

    bool isPlotBeforeGame; // 标记剧情是在游戏前(true)还是游戏后(false)

    void setupStackedWidget();   // 初始化StackedWidget
    void createMainMenuWidget(); // 创建主菜单Widget
    void updateBackground();

    // 存档管理（新系统）
    void saveDataToSlot(int slot);      // 保存到指定槽位
    bool loadDataFromSlot(int slot);    // 从指定槽位加载
    void showSaveLoadDialog(bool isSave); // 显示存档/读档对话框

    // 旧存档系统（兼容性保留）
    void saveData();
    void loadData();

    void closeEvent(QCloseEvent *event) override;

    int currentSaveSlot = 0; // 当前使用的存档槽位

    enum ButtonType
    {
        StartGame,   // 开始游戏
        SaveGame,    // 保存游戏
        LoadGame,    // 读取存档
        ExitGame,    // 退出游戏
        StoreButton  // 商店
    };
    QMap<ButtonType, QPushButton *> gameButtons;
    void createGameButtons();

    // 按钮点击事件
    void onStartGameClicked();
    void onSaveGameClicked();
    void onLoadGameClicked();
    void onExitGameClicked();
    void onStoreClicked();

    // 页面切换槽函数
    void showMainMenu();                                                                           // 显示主菜单
    void showPlot(std::vector<std::string> *textList, std::string bgPath, bool beforeGame = true); // 显示剧情
    void showPlot(PlotData *plotData, bool beforeGame = true);                                     // 显示剧情（使用PlotData）
    void showGame(GameData *data);                                                                 // 显示游戏
    void onPlotFinished();                                                                         // 剧情结束
    void onGameFinished();                                                                         // 游戏结束

    // 剧情数据
    PlotData _plotData_1;  // 关卡1剧情
    PlotData _plotData_2;  // 关卡2剧情
    void initPlotData();   // 初始化剧情数据

    // 数据板块
    GameData _data;
    WeaponData _weaponData_1 = {
        1,
        IceBall,
        10};
    WeaponData _weaponData_2 = {
        1,
        RockBall,
        10};
    PlayerData _playerData = {
        3000, 3, _weaponData_1, {0, 300}};
    PassData _passData_1 = {
        1,
        false,
        {
            {Enemy::EnemyType::Goblin, {{750, 200}, {750, 300}}, 100, 10, 0, {80, 160}, 40, 1000},
            {Enemy::EnemyType::UndeadMage, {{0, 0}, {750, 0}, {400, 0}, {0, 550}, {750, 550}, {400, 550}}, 100, 5, 1000, {2000, 4000}, 30, 1000},
            {Enemy::EnemyType::MagicCircle, {{140, 140}, {600, 400}, {140, 400}, {600, 140}}, 30000, 0, 0, {10000000, 20000000}, 4, 5},
            {Enemy::EnemyType::drop_healing, {{200, 200}}, 300, 0, 0, {10000000, 20000000}, 4, 15000},
            {Enemy::EnemyType::drop_attack, {{600, 300}}, 300, 0, 0, {10000000, 20000000}, 4, 8000},
            {Enemy::EnemyType::drop_speed, {{200, 360}}, 300, 0, 0, {10000000, 20000000}, 4, 8000},

        },
        basicGrid};
    PassData _passData_2{
        2,
        false,
        {
            {Enemy::EnemyType::Goblin, {{750, 200}, {750, 300}}, 1000, 10, 0, {20, 40}, 50, 1000},
            {Enemy::EnemyType::UndeadMage, {{0, 0}, {750, 0}, {400, 0}, {0, 550}, {750, 550}, {400, 550}}, 500, 20, 100, {2000, 4000}, 50, 1000},
            {Enemy::EnemyType::MagicCircle, {{140, 140}, {600, 400}, {140, 400}, {600, 140}}, 30000, 0, 0, {10000000, 20000000}, 4, 5},
            {Enemy::EnemyType::drop_healing, {{200, 200}}, 300, 0, 0, {10000000, 20000000}, 4, 15000},
            {Enemy::EnemyType::drop_attack, {{400, 300}}, 300, 0, 0, {10000000, 20000000}, 4, 8000},
            {Enemy::EnemyType::drop_speed, {{200, 360}}, 300, 0, 0, {10000000, 20000000}, 4, 8000},
        },
        basicGrid};

    int numOfMagicCrystal = 0;
};