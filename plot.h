#ifndef PLOT_H
#define PLOT_H

#include <QObject>
#include <QWidget>
#include <QTextBrowser>
#include "ui_plot.h"
#include <vector>
#include <string>
#include <QTimer>
#include<QString>

class Plot : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr, std::vector<std::string> *textList = nullptr, std::string bgPath = "");
    ~Plot();


private:
    Ui::Plot *ui;
    QPixmap backgroundImage;
    QTextBrowser *textBrowser; // 用于显示文本的 QTextBrowser 控件
    std::vector<std::string> textList; // 存储文本的向量 
    std::string bgPath;// 背景图片路径 
    int currentTextIndex = 0; // 当前显示的文本索引

    void init();
    //重写键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    // 显示下一个文本的函数
    void showNextText();
    //重写paintEvent事件
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

    QTimer *typeTimer;
    int currentCharIndex = 0; // 当前显示的字符索引
    QString currentText; // 当前显示的文本
};



#endif // PLOT_H