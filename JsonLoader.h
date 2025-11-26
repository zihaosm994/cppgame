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
    // Load game data from JSON file
    static GameData* loadGame(const std::string& filename);

private:
    // Parse individual components from JSON
    static PlayerData parsePlayerData(const json& player_json);
    static std::vector<EnemySpawnConfig> parseEnemyConfig(const json& enemy_json);
    static MapData parseMapData(const json& map_json);
    static std::vector<BulletData> parseBulletData(const json& bullet_json);
    static BulletData parseSingleBulletData(const json& bullet_json);
    static std::vector<npcData> parseNpcData(const json& npc_json);
    static std::vector<TaskData> parseTaskData(const json &task_json);
};

// Add this to JsonLoader.h inside the JsonPlotLoader class
class JsonPlotLoader {
public:
    // Load plot data from JSON file
    static std::vector<PlotData> * loadPlot(const std::string& filename);
};



#endif //GAME_JSONLOADER_H
