#include "store.h"
#include <QMessageBox>

Store::Store(QWidget *parent)
    : QDialog(parent),
    _magicCrystal(nullptr),
    _weaponData_1(nullptr),
    _weaponData_2(nullptr)
{
    setWindowTitle("魔法商店");
    setFixedSize(600, 400);
    initUI();
}

Store::~Store()
{
}

void Store::setStoreData(int *magicCrystal, WeaponData *weapon1, WeaponData *weapon2)
{
    _magicCrystal = magicCrystal;
    _weaponData_1 = weapon1;
    _weaponData_2 = weapon2;
    updateUI();
}

void Store::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // 左侧：洛琪希图片
    imageLabel = new QLabel(this);
    QPixmap roxyImage(":/images/RoxyWhole.png");
    imageLabel->setPixmap(roxyImage.scaled(200, 400, Qt::KeepAspectRatio));
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setFixedSize(200, 400);

    // 右侧：商店内容
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // 显示当前水晶数量
    crystalLabel = new QLabel("当前魔法水晶: 0", this);
    crystalLabel->setStyleSheet("font-size: 16px; font-weight: bold;");

    // 武器1升级按钮
    buyWeapon1Btn = new QPushButton("升级洛琪希的法杖(冰霜击)\n消耗: 100魔法水晶", this);
    buyWeapon1Btn->setStyleSheet("font-size: 14px; padding: 10px;");
    connect(buyWeapon1Btn, &QPushButton::clicked, this, &Store::onBuyWeapon1);

    // 武器2升级按钮
    buyWeapon2Btn = new QPushButton("升级傲慢水龙王(岩炮弹)\n消耗: 100魔法水晶", this);
    buyWeapon2Btn->setStyleSheet("font-size: 14px; padding: 10px;");
    connect(buyWeapon2Btn, &QPushButton::clicked, this, &Store::onBuyWeapon2);

    // 关闭按钮
    closeBtn = new QPushButton("离开商店", this);
    closeBtn->setStyleSheet("font-size: 14px; padding: 10px;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // 添加到右侧布局
    rightLayout->addWidget(crystalLabel);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(buyWeapon1Btn);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(buyWeapon2Btn);
    rightLayout->addStretch();
    rightLayout->addWidget(closeBtn);

    // 添加到主布局
    mainLayout->addWidget(imageLabel);
    mainLayout->addLayout(rightLayout);
}

void Store::updateUI()
{
    if (_magicCrystal)
    {
        crystalLabel->setText(QString("当前魔法水晶: %1").arg(*_magicCrystal));
    }
}

int Store::exec()
{
    updateUI();
    return QDialog::exec();
}

void Store::onBuyWeapon1()
{

}

void Store::onBuyWeapon2()
{

}
