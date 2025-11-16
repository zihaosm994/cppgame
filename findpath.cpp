#include <iostream>
#include "findpath.h"
#include <queue>
#include <cmath>
#include <algorithm>

using namespace std;
//叫做AStar只是我懒得改了, 这只是一个过分简单的路径生成函数
vector<pair<int, int>> AStar(const vector<vector<int>>& grid, 
                            pair<int, int> start, 
                            pair<int, int> end) {

//直接返回直角曼哈顿路径
    vector<pair<int, int>> path;
    for(int i = start.first; i != end.first; i += (end.first - start.first > 0 ? 1 : -1)){
        path.push_back({i, start.second}); 
    }
    for(int i = start.second; i!= end.second; i += (end.second - start.second > 0? 1 : -1)){
        path.push_back({end.first, i}); 
    }
    return path;
}
