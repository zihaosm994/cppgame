#pragma once
#include "ui_MyApp.h"
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QObject>
#include <QStackedWidget>
#include "Data.h"

class MainMenu: public QWidget
{
    Q_OBJECT
public:
    MainMenu(QWidget * parent = nullptr);
    ~MainMenu();
private:
    void setBackground(QString path);
    void createButtons();
    std::vector<QPushButton *> buttonList;
    void resizeEvent(QResizeEvent * event)override;
    std::vector<PlotData> *plots;
    void showPlot(int id);
signals:
    void startGame();
};


class MyApp: public QMainWindow
{
    Q_OBJECT

public:
    MyApp(QWidget *parent = nullptr);
    ~MyApp();
private:
    Ui_MyApp *ui;
    QStackedWidget *stack;
};

