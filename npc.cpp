#include "npc.h"
#include "Character.h"
#include <cmath>

// Quest构造函数
Quest::Quest(int id, QString name, QString desc, QuestType t, int target, int reward)
    : questId(id), questName(name), description(desc), type(t),
    targetCount(target), currentCount(0), rewardCrystals(reward),
    status(NotStarted)
{
}

// 更新任务进度
void Quest::updateProgress(int amount)
{
    if (status == InProgress) {
        currentCount += amount;
        if (currentCount >= targetCount) {
            currentCount = targetCount;
        }
    }
}

// 检查任务是否完成
bool Quest::isComplete() const
{
    return currentCount >= targetCount;
}

// 获取任务进度文本
QString Quest::getProgressText() const
{
    return QString("%1/%2").arg(currentCount).arg(targetCount);
}

// NPC构造函数
NPC::NPC(QPoint position, QString name, QObject *parent)
    : QObject(parent), x(position.x()), y(position.y()), npcName(name),
    greeting("你好，冒险者！"), interactionRange(150),
    currentImage(0), animationFrame(0)
{
}

// 获取X坐标
int NPC::getX() const
{
    return x;
}

// 获取Y坐标
int NPC::getY() const
{
    return y;
}

// 设置位置
void NPC::setPosition(int newX, int newY)
{
    x = newX;
    y = newY;
}

// 获取NPC名称
QString NPC::getName() const
{
    return npcName;
}

// 设置问候语
void NPC::setGreeting(QString greet)
{
    greeting = greet;
}

// 设置交互范围
void NPC::setInteractionRange(int range)
{
    interactionRange = range;
}

// 获取当前图片索引
int NPC::getCurrentImage() const
{
    return currentImage;
}

// 检查玩家是否在交互范围内
bool NPC::isPlayerInRange(Player* player) const
{
    if (!player) return false;

    // 计算玩家中心点和NPC中心点的距离
    int dx = player->getX() + player->width / 2 - (x + width / 2);
    int dy = player->getY() + player->height / 2 - (y + height / 2);
    double distance = std::sqrt(dx * dx + dy * dy);

    return distance <= interactionRange;
}

// 添加任务
void NPC::addQuest(const Quest& quest)
{
    quests.push_back(quest);
}

// 获取可接取的任务
std::vector<Quest*> NPC::getAvailableQuests()
{
    std::vector<Quest*> available;
    for (auto& quest : quests) {
        if (quest.status == NotStarted) {
            available.push_back(&quest);
        }
    }
    return available;
}

// 获取进行中的任务
std::vector<Quest*> NPC::getActiveQuests()
{
    std::vector<Quest*> active;
    for (auto& quest : quests) {
        if (quest.status == InProgress) {
            active.push_back(&quest);
        }
    }
    return active;
}

// 获取已完成的任务
std::vector<Quest*> NPC::getCompletedQuests()
{
    std::vector<Quest*> completed;
    for (auto& quest : quests) {
        if (quest.status == Completed) {
            completed.push_back(&quest);
        }
    }
    return completed;
}

// 接受任务
bool NPC::acceptQuest(int questId)
{
    for (auto& quest : quests) {
        if (quest.questId == questId && quest.status == NotStarted) {
            quest.status = InProgress;
            emit questAccepted(questId);
            return true;
        }
    }
    return false;
}

// 完成任务
bool NPC::completeQuest(int questId)
{
    for (auto& quest : quests) {
        if (quest.questId == questId && quest.status == InProgress && quest.isComplete()) {
            quest.status = Completed;
            emit questCompleted(questId);
            return true;
        }
    }
    return false;
}

// 领取奖励
bool NPC::claimReward(int questId, int& outReward)
{
    for (auto& quest : quests) {
        if (quest.questId == questId && quest.status == Completed) {
            outReward = quest.rewardCrystals;
            quest.status = Rewarded;
            emit rewardClaimed(questId, outReward);
            return true;
        }
    }
    return false;
}

// 获取对话内容
QString NPC::getDialogue()
{
    QString dialogue = greeting + "\n\n";

    // 优先显示已完成的任务
    auto completed = getCompletedQuests();
    if (!completed.empty()) {
        dialogue += "太好了！你完成了任务！\n";
        for (auto* quest : completed) {
            dialogue += QString("【%1】已完成！点击领取奖励：%2 魔法水晶\n")
                            .arg(quest->questName).arg(quest->rewardCrystals);
        }
        return dialogue;
    }

    // 显示进行中的任务
    auto active = getActiveQuests();
    if (!active.empty()) {
        dialogue += "你当前的任务进度：\n";
        for (auto* quest : active) {
            dialogue += QString("【%1】%2\n")
                            .arg(quest->questName).arg(quest->getProgressText());
        }
        return dialogue;
    }

    // 显示可接取的任务
    auto available = getAvailableQuests();
    if (!available.empty()) {
        dialogue += "我这里有些任务需要你的帮助：\n\n";
        for (auto* quest : available) {
            dialogue += QString("【%1】\n%2\n目标：%3\n奖励：%4 魔法水晶\n\n")
                            .arg(quest->questName)
                            .arg(quest->description)
                            .arg(quest->targetCount)
                            .arg(quest->rewardCrystals);
        }
        return dialogue;
    }

    // 没有任务
    dialogue += "感谢你的帮助，冒险者！";
    return dialogue;
}

// 更新动画
void NPC::updateAnimation()
{
    animationFrame++;
    if (animationFrame >= ANIMATION_SPEED) {
        animationFrame = 0;
        currentImage = (currentImage + 1) % 4; // 4帧循环
    }
}

// 检查是否有交互内容
bool NPC::hasInteraction() const
{
    return !quests.empty();
}
