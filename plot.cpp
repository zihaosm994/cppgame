#include "plot.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug> // 用于调试输出

Plot::Plot(QWidget *parent) : QWidget(parent)
{
    // 设置固定大小
    setFixedSize(800, 600);

    // 初始化文本浏览器
    textBrowser = new QTextBrowser(this);
    // 这里的几何位置根据你的窗口大小调整，放在底部
    textBrowser->setGeometry(0,height()*4/5,width(),height()/5);

    // 设置为黑色半透明 (R, G, B, Alpha)
    textBrowser->setStyleSheet("background-color: rgba(0, 0, 0, 150); border: none; padding: 10px;");
    textBrowser->setAlignment(Qt::AlignLeft);
    textBrowser->setReadOnly(true);

    // 设置打印定时器
    typeTimer = new QTimer(this);
    connect(typeTimer, &QTimer::timeout, this, [this]() {
        if (currentCharIndex < currentText.length())
        {
            // 追加一个字符
            textBrowser->setText(currentText.left(currentCharIndex + 1));
            currentCharIndex++;
        }
        else
        {
            typeTimer->stop();
        }
    });

    // 设置字体
    QFont font;
    font.setPointSize(20); // 稍微调大一点字体以适应1200的宽度
    font.setBold(true);    // 通常剧情文字加粗更易读
    font.setFamily("SimSun"); // 或者 "Microsoft YaHei"
    textBrowser->setFont(font);
    textBrowser->setTextColor(Qt::white);

    setFocusPolicy(Qt::StrongFocus);
    textBrowser->setFocusPolicy(Qt::NoFocus);
}

Plot::~Plot()
{
}

void Plot::setPlotContent(std::vector<std::pair<std::string, std::string>>* plotContent)
{
    this->plotContent = plotContent;
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
    // 可以在这里设置一个默认背景，或者清空背景
    backgroundImage = QPixmap();
    update();
}

void Plot::init()
{
    if (!plotContent || plotContent->empty()) return;

    // 直接调用 showNextText 来加载第一帧（图片+文字），避免代码重复
    showNextText();

    QTimer::singleShot(100, this, [this]()
                       {
                           setFocus(); // 确保窗口获得焦点，以便接收键盘事件
                       });
}

void Plot::keyPressEvent(QKeyEvent *event)
{
    // 优先处理空格和回车
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (typeTimer->isActive())
        {
            // 如果字还没打完，用户按下按键，则直接显示全部文本
            typeTimer->stop();
            textBrowser->setText(currentText);
            currentCharIndex = currentText.length(); // 更新索引到最后
        }
        else
        {
            // 如果字已经打完，显示下一句
            showNextText();
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void Plot::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void Plot::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // 只有图片加载成功才绘制
    if (!backgroundImage.isNull()) {
        painter.drawPixmap(0, 0, backgroundImage);
    } else {
        // 如果没有图片，可以填充黑色背景
        painter.fillRect(rect(), Qt::black);
    }
}

void Plot::showNextText()
{
    // 安全检查
    if (!plotContent) return;

    if (currentTextIndex < plotContent->size())
    {
        // 1. 获取当前的数据对 (图片路径, 文本内容)
        std::pair<std::string, std::string> currentData = (*plotContent)[currentTextIndex];
        std::string imgPath = currentData.first;
        std::string txtContent = currentData.second;

        // 2. 加载并处理图片
        bool loadSuccess = backgroundImage.load(QString::fromStdString(imgPath));
        if (loadSuccess) {
            // 缩放到窗口大小 (1200x800)，保持比例还是拉伸看你需求
            // 这里使用 IgnoreAspectRatio 拉伸铺满，或者 KeepAspectRatioByExpanding 裁剪铺满
            backgroundImage = backgroundImage.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            qDebug() << "Failed to load image:" << QString::fromStdString(imgPath);
        }

        // 触发重绘，更新背景
        update();

        // 3. 处理文本打字机效果
        currentText = QString::fromStdString(txtContent);
        currentCharIndex = 0;
        textBrowser->clear();

        // 启动打字机
        typeTimer->start(50);

        // 4. 索引指向下一条
        currentTextIndex++;
    }
    else
    {
        // 剧情结束
        emit plotFinished();
    }
}

void Plot::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setFocus();
}
