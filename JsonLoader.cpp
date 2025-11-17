//
// Created by 14zhieow on 2025/11/16.
//

#include "JsonLoader.h"
#include <iostream>
#include <stdexcept>

std::shared_ptr<GameData> JsonGameLoader::loadGame(const std::string &filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Can't open file" + filename);

        json json_data;
        file >> json_data;

        auto gameData = std::make_shared<GameData>();

        if (json_data.contains("player") && json_data["player"].is_object()) {
            gameData->playerData = parsePlayerData(json_data["player"]);
        } else std::cerr << "ERROR: Missing Player Data!" << std::endl;

        if (json_data.contains("enemy") && json_data["enemy"].is_array()) {
            gameData->enemySpawnConfigs = parseEnemyConfig(json_data["enemy"]);
        } else std::cerr << "ERROR: Missing Enemy Data!" << std::endl;

        if (json_data.contains("map") && json_data["map"].is_object()) {
            gameData->mapData = parseMapData(json_data["map"]);
        } else std::cerr << "ERROR: Missing Map Data!" << std::endl;

        if (json_data.contains("bullet") && json_data["bullet"].is_array()) {
            gameData->bulletDataList = parseBulletData(json_data["bullet"]);
        } else std::cerr << "ERROR: Missing Bullet Data!" << std::endl;

        std::cout << "Completely load game data" << std::endl;
        std::cout << "- Bullet type number: " << gameData->bulletDataList.size() << std::endl;
        std::cout << "- Enemy spawn config number: " << gameData->enemySpawnConfigs.size() << std::endl;

        return gameData;
    } catch (const std::exception& e) {
        std::cerr << "Failed to Load Game Data: " << e.what() << std::endl;
        return std::make_shared<GameData>();
    }
}

PlayerData JsonGameLoader::parsePlayerData(const json &player_json) {
    PlayerData playerData;
    playerData.playerId = player_json.value("id", 0);
    playerData.hp = player_json.value("hp", 3000);
    playerData.width = player_json.value("width", 60);
    playerData.height = player_json.value("height", 90);
    playerData.moveStep = player_json.value("move_step", 3);

    if (player_json.contains("generate_pos") && player_json["generate_pos"].is_array() && player_json["generate_pos"].size() >= 2) {
        playerData.generatePos = QPoint(player_json["generate_pos"][0], player_json["generate_pos"][1]);
    } else playerData.generatePos = QPoint(0, 300);

    playerData.frameCnt = player_json.value("frame_cnt", 8);
    playerData.damage = player_json.value("damage", 20);
    playerData.attackCD = player_json.value("attack_cd", 200);
    playerData.attackRange = player_json.value("attack_range", 600);
    playerData.bulletId = player_json.value("bullet_id", 0);
    playerData.bulletSpeed = player_json.value("bullet_speed", 10.0);

    if (player_json.contains("right_walk_paths") && player_json["right_walk_paths"].is_array()) {
        for (const auto& path : player_json["right_walk_paths"])
            if (path.is_string())
                playerData.rightWalkPaths.push_back(path);
    }

    if (player_json.contains("left_walk_paths") && player_json["left_walk_paths"].is_array()) {
        for (const auto& path : player_json["left_walk_paths"])
            if (path.is_string())
                playerData.leftWalkPaths.push_back(path);
    }

    if (player_json.contains("up_walk_paths") && player_json["up_walk_paths"].is_array()) {
        for (const auto& path : player_json["up_walk_paths"])
            if (path.is_string())
                playerData.upWalkPaths.push_back(path);
    }

    if (player_json.contains("down_walk_paths") && player_json["down_walk_paths"].is_array()) {
        for (const auto& path : player_json["down_walk_paths"])
            if (path.is_string())
                playerData.downWalkPaths.push_back(path);
    }

    return playerData;
}

std::vector<EnemySpawnConfig> JsonGameLoader::parseEnemyConfig(const json &enemy_json) {
    std::vector<EnemySpawnConfig> enemySpawnConfigs;
    for (const auto& enemy_config_json : enemy_json) {
        EnemyData enemyData;
        enemyData.enemyType = enemy_config_json.value("type", 0);
        enemyData.hp = enemy_config_json.value("hp", 100);
        enemyData.damage = enemy_config_json.value("damage", 10);
        enemyData.width = enemy_config_json.value("width", 80);
        enemyData.height = enemy_config_json.value("height", 80);
        enemyData.frameCnt = enemy_config_json.value("frame_cnt", 8);
        enemyData.canMove = enemy_config_json.value("can_move", true);
        enemyData.moveStep = enemy_config_json.value("move_step", 8);
        enemyData.speedF = std::make_pair(enemy_config_json.value("speed_x", 30), enemy_config_json.value("speed_y", 60));
        enemyData.pathUpdateFreq = enemy_config_json.value("path_update_freq", 500);
        enemyData.hasMeleeAttack = enemy_config_json.value("has_melee_attack", true);
        enemyData.hasRangedAttack = enemy_config_json.value("has_ranged_attack", false);
        enemyData.attackCD = enemy_config_json.value("attack_cd", 1000);
        enemyData.bulletId = enemy_config_json.value("bullet_id", 0);
        enemyData.bulletSpeed = enemy_config_json.value("bullet_speed", 16.0);
        if (enemy_config_json.contains("image_paths") && enemy_config_json["image_paths"].is_array()) {
            for (const auto& path : enemy_config_json["image_paths"])
                if (path.is_string())
                    enemyData.imagePaths.push_back(path);
        }
        EnemySpawnConfig enemySpawnConfig;
        enemySpawnConfig.enemyData = enemyData;
        enemySpawnConfig.spawnFreq = enemy_config_json.value("spawn_freq", 1000);
        enemySpawnConfig.totalNum = enemy_config_json.value("total_num", 0);
        if (enemy_config_json.contains("spawn_positions") && enemy_config_json["spawn_positions"].is_array()) {
            for (const auto& pos : enemy_config_json["spawn_positions"])
                if (pos.is_array() && pos.size() >= 2)
                    enemySpawnConfig.spawnPositions.push_back(QPoint(pos[0], pos[1]));
        }
        enemySpawnConfigs.push_back(enemySpawnConfig);
    }
    return enemySpawnConfigs;
}

MapData JsonGameLoader::parseMapData(const json &map_json) {
    MapData mapData;
    mapData.backgroundImgPath = map_json.value("background_img_path", "");
    if (map_json.contains("obstacles") && map_json["obstacles"].is_array()) {
        for (const auto& obstacle_json : map_json["obstacles"]) {
            ObstacleData obstacleData;
            if (obstacle_json.contains("pos") && obstacle_json["pos"].is_array() && obstacle_json["pos"].size() >= 2)
                obstacleData.pos = QPoint(obstacle_json["pos"][0], obstacle_json["pos"][1]);
            obstacleData.width = obstacle_json.value("width", 0);
            obstacleData.height = obstacle_json.value("height", 0);
            obstacleData.imagePath = obstacle_json.value("image_path", "");
            mapData.obstacles.push_back(obstacleData);
        }
    }
    return mapData;
}

std::vector<BulletData> JsonGameLoader::parseBulletData(const json &bullet_json) {
    std::vector<BulletData> bulletDataList;
    for (const auto& bullet_data_json : bullet_json) {
        BulletData bulletData;
        bulletData.bulletId = bullet_data_json.value("bullet_id", 0);
        bulletData.width = bullet_data_json.value("width", 20);
        bulletData.height = bullet_data_json.value("height", 20);
        bulletData.moveSpeed = bullet_data_json.value("move_speed", 10);
        bulletData.damageMultiplier = bullet_data_json.value("damage_multiplier", 1);
        bulletData.frameCnt = bullet_data_json.value("frame_cnt", 8);
        bulletData.targetHeight = bullet_data_json.value("target_height", 90);
        bulletData.targetWidth = bullet_data_json.value("target_width", 60);
        if (bullet_data_json.contains("image_paths") && bullet_data_json["image_paths"].is_array()) {
            for (const auto& path : bullet_data_json["image_paths"])
                if (path.is_string())
                    bulletData.imagePaths.push_back(path);
        }
        bulletDataList.push_back(bulletData);
    }
    return bulletDataList;
}