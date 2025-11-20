#ifndef CHARACTER_H
#define CHARACTER_H
#include <qobject.h>

class Character :public QObject
{
    Q_OBJECT;
public:
    Character(QObject * parent):QObject(parent){}
    virtual int getX()const =0;
    virtual int getY()const=0;
    virtual int getHp()const=0;
    virtual void setHP(int hp)=0;
    virtual int getWidth()const=0;
    virtual int getHeight()const=0;
};
// 方向枚举
enum Direction
{
    Up,
    Down,
    Left,
    Right
};
struct PictureIndex
{
    int curIndex;
    int maxCnt;
    PictureIndex(int curIndex=0,int maxCnt=0):curIndex(curIndex),maxCnt(maxCnt)
    {}
};
#endif // CHARACTER_H

