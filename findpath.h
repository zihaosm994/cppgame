#ifndef FIND_PATH_H
#define FIND_PATH_H
#include <vector>
#include <utility>
using namespace std;
vector<pair<int,int>> AStar(const vector<vector<int>>& grid, 
                            pair<int,int> start, 
                            pair<int,int> end);
#endif