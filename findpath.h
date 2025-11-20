#ifndef FIND_PATH_H
#define FIND_PATH_H
#include <vector>
#include <QPoint>
#include "Data.h"
using namespace std;
vector<QPoint> *AStar(MapData* mapData,
                             int width,
                             int height,
                            int step,
                        QPoint start,
                        QPoint end);
#endif
