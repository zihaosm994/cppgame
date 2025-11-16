#ifndef STORE_H
#define STORE_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include "Data.h"

class Store : public QDialog
{
    Q_OBJECT

public:
    explicit Store(QWidget *parent = nullptr);
    ~Store();

    // 设置商店数据
    void setStoreData(int *magicCrystal, WeaponData *weapon1, WeaponData *weapon2);

    // 显示商店对话框
    int exec() override;

private:
    // 数据指针（引用外部数据）
    int *_magicCrystal;
    WeaponData *_weaponData_1;
    WeaponData *_weaponData_2;

    // UI组件
    QLabel *imageLabel;
    QLabel *crystalLabel;
    QPushButton *buyWeapon1Btn;
    QPushButton *buyWeapon2Btn;
    QPushButton *closeBtn;

    // 初始化UI
    void initUI();
    void updateUI();

    // 购买处理
    void onBuyWeapon1();
    void onBuyWeapon2();
};

#endif // STORE_H
