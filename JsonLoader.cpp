// JsonLoader.cpp
#include "JsonLoader.h"
#include <iostream>
#include <stdexcept>
#include <QFile>
#include <QByteArray>

GameData* JsonGameLoader::loadGame(const std::string &filename) {
    try {
        // Use QFile instead of std::ifstream for Qt resources
        QFile file(QString::fromStdString(filename));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        // Read file content into QByteArray
        QByteArray fileData = file.readAll();
        file.close();

        // Parse JSON from QByteArray
        json jsonData = json::parse(fileData.constData());

        auto gameData = new GameData();

        // Parse player data
        if (jsonData.contains("player") && jsonData["player"].is_object()) {
            gameData->playerData = parsePlayerData(jsonData["player"]);
        }

        // Parse enemy spawn configurations
        if (jsonData.contains("enemies") && jsonData["enemies"].is_array()) {
            gameData->enemySpawnConfigs = parseEnemyConfig(jsonData["enemies"]);
        }

        // Parse map data
        if (jsonData.contains("map") && jsonData["map"].is_object()) {
            gameData->mapData = parseMapData(jsonData["map"]);
        }

        // Parse bullet data
        if (jsonData.contains("bullets") && jsonData["bullets"].is_array()) {
            gameData->bulletDataList = parseBulletData(jsonData["bullets"]);
        }

        // Parse NPC data
        if (jsonData.contains("npcs") && jsonData["npcs"].is_array()) {
            gameData->npcDatas = parseNpcData(jsonData["npcs"]);
        }

        gameData->bossNum = 1;

        // Parse Task List
        if (jsonData.contains("tasks") && jsonData["tasks"].is_array()) {
            gameData->taskList = parseTaskData(jsonData["tasks"]);
        }

        return gameData;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game data: " << e.what() << std::endl;
        return nullptr;
    }
}

PlayerData JsonGameLoader::parsePlayerData(const json &player_json) {
    PlayerData playerData{};

    playerData.hp = player_json.value("hp", 3000);
    playerData.width = player_json.value("width", 60);
    playerData.height = player_json.value("height", 90);
    playerData.moveStep = player_json.value("moveStep", 3);
    playerData.damage = player_json.value("damage", 20);
    playerData.attackCD = player_json.value("attackCD", 200);
    playerData.attackRange = player_json.value("attackRange", 600);

    // Parse generate position
    if (player_json.contains("generatePos") &&
        player_json["generatePos"].is_array() &&
        player_json["generatePos"].size() == 2) {
        playerData.generatePos = QPoint(
            player_json["generatePos"][0],
            player_json["generatePos"][1]
        );
    }

    // Parse animation paths
    if (player_json.contains("rightWalkPaths") && player_json["rightWalkPaths"].is_array()) {
        for (const auto& path : player_json["rightWalkPaths"]) {
            if (path.is_string()) {
                playerData.rightWalkPaths.push_back(path);
            }
        }
    }

    if (player_json.contains("leftWalkPaths") && player_json["leftWalkPaths"].is_array()) {
        for (const auto& path : player_json["leftWalkPaths"]) {
            if (path.is_string()) {
                playerData.leftWalkPaths.push_back(path);
            }
        }
    }

    if (player_json.contains("upWalkPaths") && player_json["upWalkPaths"].is_array()) {
        for (const auto& path : player_json["upWalkPaths"]) {
            if (path.is_string()) {
                playerData.upWalkPaths.push_back(path);
            }
        }
    }

    if (player_json.contains("downWalkPaths") && player_json["downWalkPaths"].is_array()) {
        for (const auto& path : player_json["downWalkPaths"]) {
            if (path.is_string()) {
                playerData.downWalkPaths.push_back(path);
            }
        }
    }

    // Parse bullet data for player
    if (player_json.contains("bulletData") && player_json["bulletData"].is_array()) {
        playerData.bulletData = parseBulletData(player_json["bulletData"]);
    }

    return playerData;
}

std::vector<EnemySpawnConfig> JsonGameLoader::parseEnemyConfig(const json &enemy_json) {
    std::vector<EnemySpawnConfig> enemyConfigs;

    for (const auto& enemyItem : enemy_json) {
        EnemySpawnConfig config{};

        // Parse enemy data
        EnemyData enemyData{};
        enemyData.enemyID = enemyItem.value("enemyID", 0);
        enemyData.hp = enemyItem.value("hp", 100);
        enemyData.damage = enemyItem.value("damage", 10);
        enemyData.width = enemyItem.value("width", 80);
        enemyData.height = enemyItem.value("height", 80);
        enemyData.canMove = enemyItem.value("canMove", true);
        enemyData.moveStep = enemyItem.value("moveStep", 2);
        enemyData.pathUpdateFreq = enemyItem.value("pathUpdateFreq", 0);
        enemyData.hasMeleeAttack = enemyItem.value("hasMeleeAttack", true);
        enemyData.hasRangedAttack = enemyItem.value("hasRangedAttack", false);
        enemyData.bulletWithoutObstacles = enemyItem.value("bulletWithoutObstacles", false);
        enemyData.attackCD = enemyItem.value("attackCD", 1000);

        // Parse speed range
        if (enemyItem.contains("speedF") && enemyItem["speedF"].is_array() && enemyItem["speedF"].size() == 2) {
            enemyData.speedF = std::make_pair(
                enemyItem["speedF"][0].get<int>(),
                enemyItem["speedF"][1].get<int>()
            );
        }

        // Parse walk paths
        if (enemyItem.contains("rightWalkPaths") && enemyItem["rightWalkPaths"].is_array()) {
            for (const auto& path : enemyItem["rightWalkPaths"]) {
                if (path.is_string()) {
                    enemyData.rightWalkPaths.push_back(path);
                }
            }
        }

        if (enemyItem.contains("leftWalkPaths") && enemyItem["leftWalkPaths"].is_array()) {
            for (const auto& path : enemyItem["leftWalkPaths"]) {
                if (path.is_string()) {
                    enemyData.leftWalkPaths.push_back(path);
                }
            }
        }

        // Parse bullet data reference
        // Note: In a real implementation, you would need to link this to actual bullet data
        json bulletJson = enemyItem["bulletData"];
        BulletData *bulletData = new BulletData;
        bulletData->bulletID = bulletJson.value("bulletID", 0);
        bulletData->moveSpeed = bulletJson.value("moveSpeed", 10);
        bulletData->damageMultiplier = bulletJson.value("damageMultiplier", 1.0);
        bulletData->dmoveDis = bulletJson.value("dmoveDis", 10);
        bulletData->width = bulletJson.value("width", 20);
        bulletData->height = bulletJson.value("height", 20);
        if (bulletJson.contains("imagePaths") && bulletJson["imagePaths"].is_array()) {
            for (const auto& path : bulletJson["imagePaths"]) {
                if (path.is_string()) {
                    bulletData->imagePaths.push_back(path);
                }
            }
        }
        enemyData.bulletData = bulletData;
        config.enemyData = enemyData;

        // Parse spawn configuration
        config.totalNum = enemyItem.value("totalNum", 20);
        config.spawnFreq = enemyItem.value("spawnFreq", 1000);

        if (enemyItem.contains("spawnPositions") && enemyItem["spawnPositions"].is_array()) {
            for (const auto& pos : enemyItem["spawnPositions"]) {
                if (pos.is_array() && pos.size() == 2) {
                    config.spawnPositions.push_back(QPoint(pos[0], pos[1]));
                }
            }
        }

        enemyConfigs.push_back(config);
    }

    return enemyConfigs;
}

MapData JsonGameLoader::parseMapData(const json &map_json) {
    MapData mapData{};

    mapData.backgroundImgPath = map_json.value("backgroundImgPath", "");
    mapData.mapWidth = map_json.value("mapWidth", 2048);
    mapData.mapHeight = map_json.value("mapHeight", 1042);

    if (map_json.contains("obstacles") && map_json["obstacles"].is_array()) {
        for (const auto& obstacle : map_json["obstacles"]) {
            ObstacleData obstacleData{};

            obstacleData.width = obstacle.value("width", 0);
            obstacleData.height = obstacle.value("height", 0);
            obstacleData.imagePath = obstacle.value("imagePath", "");

            if (obstacle.contains("pos") && obstacle["pos"].is_array() && obstacle["pos"].size() == 2) {
                obstacleData.pos = QPoint(obstacle["pos"][0], obstacle["pos"][1]);
            }

            mapData.obstacles.push_back(obstacleData);
        }
    }

    return mapData;
}

BulletData JsonGameLoader::parseSingleBulletData(const json &bulletItem) {
    BulletData bulletData{};

    bulletData.bulletID = bulletItem.value("bulletID", 0);
    bulletData.width = bulletItem.value("width", 20);
    bulletData.height = bulletItem.value("height", 20);
    bulletData.damageMultiplier = bulletItem.value("damageMultiplier", 1.0);
    bulletData.moveSpeed = bulletItem.value("moveSpeed", 10);
    bulletData.dmoveDis = bulletItem.value("dmoveDis", 80);

    if (bulletItem.contains("imagePaths") && bulletItem["imagePaths"].is_array()) {
        for (const auto& path : bulletItem["imagePaths"]) {
            if (path.is_string()) {
                bulletData.imagePaths.push_back(path);
            }
        }
    }
    return bulletData;
}
std::vector<BulletData> JsonGameLoader::parseBulletData(const json &bullet_json) {
    std::vector<BulletData> bullets;

    for (const auto& bulletItem : bullet_json) {
        BulletData bulletData{};

        bulletData.bulletID = bulletItem.value("bulletID", 0);
        bulletData.width = bulletItem.value("width", 20);
        bulletData.height = bulletItem.value("height", 20);
        bulletData.damageMultiplier = bulletItem.value("damageMultiplier", 1.0);
        bulletData.moveSpeed = bulletItem.value("moveSpeed", 10);
        bulletData.dmoveDis = bulletItem.value("dmoveDis", 80);

        if (bulletItem.contains("imagePaths") && bulletItem["imagePaths"].is_array()) {
            for (const auto& path : bulletItem["imagePaths"]) {
                if (path.is_string()) {
                    bulletData.imagePaths.push_back(path);
                }
            }
        }
        bullets.push_back(bulletData);
    }

    return bullets;
}

std::vector<npcData> JsonGameLoader::parseNpcData(const json &npc_json) {
    std::vector<npcData> npcs;

    for (const auto& npcItem : npc_json) {
        npcData npc{};

        npc.npcID = npcItem.value("npcID", 0);
        npc.imagePath = npcItem.value("imagePath", "");
        npc.width1 = npcItem.value("width1", 0);
        npc.height1 = npcItem.value("height1", 0);
        npc.greetingImagePath = npcItem.value("greetingImagePath", "");
        npc.width2 = npcItem.value("width2", 0);
        npc.height2 = npcItem.value("height2", 0);
        npc.dhp = npcItem.value("dhp", 0);
        npc.ddamage = npcItem.value("ddamage", 0);
        npc.dattackRange = npcItem.value("dattackRange", 0);
        npc.dattackCD = npcItem.value("dattackCD", 0);
        npc.dmoveStep = npcItem.value("dmoveStep", 0);

        if (npcItem.contains("pos") && npcItem["pos"].is_array() && npcItem["pos"].size() == 2) {
            npc.pos = QPoint(npcItem["pos"][0], npcItem["pos"][1]);
        }

        // Note: In a real implementation, you would need to link this to actual bullet data
        npc.bulletData = nullptr;

        npcs.push_back(npc);
    }

    return npcs;
}

std::vector<PlotData> * JsonPlotLoader::loadPlot(const std::string& filename) {
    try {
        // Use QFile instead of std::ifstream for Qt resources
        QFile file(QString::fromStdString(filename));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open plot file: " + filename);
        }

        // Read file content into QByteArray
        QByteArray fileData = file.readAll();
        file.close();

        // Parse JSON from QByteArray
        json jsonData = json::parse(fileData.constData());

        std::vector<PlotData> * plotDatas = new std::vector<PlotData>;

        // Check if the root object contains a "plot" key
        if (jsonData.contains("plot") && jsonData["plot"].is_array()) {
            const auto& plotArray = jsonData["plot"];

            for (const auto& dialogueSequence : plotArray) {
                // Each dialogueSequence should be an array of dialogue elements
                if (dialogueSequence.is_array()) {
                    PlotData sequence;

                    for (const auto& dialogueElement : dialogueSequence) {
                        // Each dialogueElement should have image_path and text
                        if (dialogueElement.is_object() &&
                            dialogueElement.contains("image_path") &&
                            dialogueElement["image_path"].is_string() &&
                            dialogueElement.contains("text") &&
                            dialogueElement["text"].is_string()) {

                            std::string imagePath = dialogueElement["image_path"];
                            std::string text = dialogueElement["text"];
                            sequence.imagePath_texts.emplace_back(imagePath, text);
                        }
                    }

                    // Add completed dialogue sequence to plotData
                    plotDatas->push_back(sequence);
                }
            }
        }

        return plotDatas;
    } catch (const std::exception& e) {
        std::cerr << "Error loading plot data: " << e.what() << std::endl;
        return {};
    }
}

std::vector<TaskData> JsonGameLoader::parseTaskData(const json &task_json) {
    std::vector<TaskData> tasks;

    for (const auto& item : task_json) {
        TaskData task;

        // 1. 解析基础文本信息
        // 注意：保留了 Data.h 中的拼写 "taskDiscrubtion"
        task.taskName = item.value("taskName", "Unknown Task");
        task.taskDiscrubtion = item.value("taskDiscrubtion", "");

        // 2. 解析任务目标条件 (注意 Data.h 中要求的默认值 -1)
        task.bossID_request = item.value("bossID_request", -1);
        task.bossID_target = item.value("bossID_target", -1);

        task.enemyCnt_request = item.value("enemyCnt_request", -1);
        task.enemyCnt_target = item.value("enemyCnt_target", 0); // 数量默认可以为0

        task.npcID_request = item.value("npcID_request", -1);
        task.npcID_target = item.value("npcID_target", -1);

        task.isValid = item.value("isValid", false);
        task.isComplete = item.value("isComplete", false);

        // 3. 解析 Buffs (数组，最多3个)
        if (item.contains("buffs") && item["buffs"].is_array()) {
            int buffIndex = 0;
            for (const auto& buffItem : item["buffs"]) {
                if (buffIndex >= 3) break; // 防止越界

                // 解析属性加成
                task.buffs[buffIndex].dhp = buffItem.value("dhp", 0);
                task.buffs[buffIndex].ddamage = buffItem.value("ddamage", 0);
                task.buffs[buffIndex].dattackRange = buffItem.value("dattackRange", 0);
                task.buffs[buffIndex].dattackCD = buffItem.value("dattackCD", 0);
                task.buffs[buffIndex].dmoveStep = buffItem.value("dmoveStep", 0);

                // 解析 Buff 附带的子弹
                // 如果 JSON 中包含 bulletData 对象
                if (buffItem.contains("bulletData") && buffItem["bulletData"].is_object()) {
                    // 解析子弹数据并创建新对象
                    BulletData bd = parseSingleBulletData(buffItem["bulletData"]);
                    task.buffs[buffIndex].bulletData = new BulletData(bd);
                } else {
                    task.buffs[buffIndex].bulletData = nullptr;
                }

                buffIndex++;
            }
        }

        tasks.push_back(task);
    }

    return tasks;
}


