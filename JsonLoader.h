//
// Created by 14zhieow on 2025/11/16.
//

#ifndef GAME_JSONLOADER_H
#define GAME_JSONLOADER_H

#include "Data.h"
#include <string>
#include <fstream>
#include <memory>
#include <third_party/json.hpp>

using json = nlohmann::json;

class JsonGameLoader {
public:
    // 加载关卡数据
    static std::shared_ptr<GameData> loadGame(const std::string& filename);

private:
    // 从json文件中加载关卡数据
    static PlayerData parsePlayerData(const json& player_json);
    static EnemyData parseEnemyData(const json& enemy_json);
    static MapData parseMapData(const json& map_json);
};

class JsonPlotLoader {
public:
    // 加载对话数据
    static std::shared_ptr<PlotData> loadPlot(const std::string& filename);

private:

};


#endif //GAME_JSONLOADER_H