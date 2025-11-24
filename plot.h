#ifndef PLOT_H
#define PLOT_H

#include <QObject>
#include <QWidget>
#include <QTextBrowser>
#include <QPixmap>
#include <vector>
#include <string>
#include <utility> // for std::pair
#include <QTimer>
#include <QString>

class Plot : public QWidget
{
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();

    // 设置剧情内容
    void setPlotContent(std::vector<std::pair<std::string, std::string>>* plotContent);
    // 重置剧情状态
    void reset();

signals:
    void plotFinished(); // 剧情播放完毕信号

private:
    QTextBrowser *textBrowser;         // 用于显示文本的 QTextBrowser 控件

    // 核心数据指针
    std::vector<std::pair<std::string, std::string>>* plotContent = nullptr;

    int currentTextIndex = 0;          // 当前显示的剧情节点索引
    QPixmap backgroundImage;           // **新增**：当前显示的背景图片

    void init();

    // 重写键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    // 显示下一个剧情（图片+文本）的函数
    void showNextText();

    // 重写paintEvent事件
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

    QTimer *typeTimer;
    int currentCharIndex = 0; // 当前显示的字符索引
    QString currentText;      // 当前显示的文本
    void resizeEvent(QResizeEvent * event)override;
};

#endif // PLOT_H
