#include "plot.h"
#include <QMouseEvent>
#include <QPainter>

Plot::Plot(QWidget *parent) : QWidget(parent)
{
    // 设置固定大小（放大到1200x800）
    setFixedSize(1200, 800);

    // 初始化文本浏览器
    textBrowser = new QTextBrowser(this);       // 创建 QTextBrowser 控件
    textBrowser->setGeometry(0, 650, 1200, 150); // 设置文本浏览器的位置和大小（放大）
    // 设置为黑色半透明
    textBrowser->setStyleSheet("background-color: rgba(0, 0, 0, 128);");
    // 设置文本靠左对齐
    textBrowser->setAlignment(Qt::AlignLeft);
    textBrowser->setReadOnly(true); // 设置文本浏览器为只读模式，不允许编辑

    // 设置打印定时器
    typeTimer = new QTimer(this);
    connect(typeTimer, &QTimer::timeout, this, [this]() { // 连接定时器的超时信号到一个 lambda 函数，用于逐字显示文本
        if (currentCharIndex < currentText.length())
        {                                                                 // 如果还有字符要显示
            textBrowser->setText(currentText.left(currentCharIndex + 1)); // 在文本浏览器中追加当前字符
            currentCharIndex++;                                           // 增加当前字符索引
        }
        else
        {                      // 如果没有字符要显示了
            typeTimer->stop(); // 停止定时器
        }
    });

    // 设置字体
    QFont font;            // 创建 QFont 对象
    font.setPointSize(16); // 设置字体大小为 16 像素
    font.setBold(false);   // 设置字体为非粗体
    // 设置字体样式为
    font.setFamily("SimSun");
    textBrowser->setFont(font); // 设置文本浏览器的字体为 font
    // 设置文本颜色
    textBrowser->setTextColor(Qt::white); // 设置文本颜色为白色

    setFocusPolicy(Qt::StrongFocus);          // 设置窗口的焦点策略为强焦点
    textBrowser->setFocusPolicy(Qt::NoFocus); // 防止抢夺窗口焦点
}

Plot::~Plot()
{
    // 不需要手动删除 textBrowser 和 typeTimer，Qt 的父子关系会自动管理
}

void Plot::setPlotContent(std::vector<std::string> *textList, std::string bgPath)
{
    this->textList = *textList;
    this->bgPath = bgPath;
    init();
}

void Plot::reset()
{
    currentTextIndex = 0;
    currentCharIndex = 0;
    currentText.clear();
    textBrowser->clear();
    if (typeTimer->isActive())
    {
        typeTimer->stop();
    }
}
void Plot::init()
{
    // 设置背景图片
    backgroundImage.load(bgPath.c_str());                                                              // 加载背景图片
    backgroundImage = backgroundImage.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放背景图片以适应窗口大小

    showNextText(); // 显示第一个文本
    QTimer::singleShot(100, this, [this]()
                       {
                           setFocus(); // 设置窗口焦点
                       });
}
void Plot::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    { // 如果按下的是空格键或回车键
        if (typeTimer->isActive())
        {                                      // 如果定时器正在运行
            typeTimer->stop();                 // 停止定时器
            textBrowser->setText(currentText); // 显示当前文本
        }
        else
        {
            showNextText(); // 显示下一个文本
        }
    }
}
void Plot::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void Plot::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);                    // 创建 QPainter 对象，用于绘制窗口内容
    painter.drawPixmap(0, 0, backgroundImage); // 绘制背景图片
}

void Plot::showNextText()
{
    if (currentTextIndex < textList.size())
    { // 如果还有文本要显示
        currentText = QString::fromStdString(textList.at(currentTextIndex));
        currentCharIndex = 0;
        textBrowser->clear();
        typeTimer->start(50); // 每50ms显示一个字（可根据需要调整）
        currentTextIndex++;
    }
    else
    {                        // 如果没有文本要显示了
        emit plotFinished(); // 发送剧情结束信号
    }
}
void Plot::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setFocus(); // 设置窗口焦点
}
