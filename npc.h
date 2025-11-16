#ifndef NPC_H
#define NPC_H

#include <QObject>
#include <QString>
#include <QPoint>
#include <vector>

// 前向声明
class Player;

// 任务类型枚举
enum QuestType {
    KillEnemies,    // 击杀敌人
    CollectItems,   // 收集物品
    ReachLocation   // 到达指定位置
};

// 任务状态枚举
enum QuestStatus {
    NotStarted,     // 未开始
    InProgress,     // 进行中
    Completed,      // 已完成
    Rewarded        // 已领取奖励
};

// 任务结构
struct Quest {
    int questId;                // 任务ID
    QString questName;          // 任务名称
    QString description;        // 任务描述
    QuestType type;             // 任务类型
    int targetCount;            // 目标数量
    int currentCount;           // 当前进度
    int rewardCrystals;         // 奖励魔法水晶数量
    QuestStatus status;         // 任务状态

    // 构造函数
    Quest(int id = 0, QString name = "", QString desc = "",
          QuestType t = KillEnemies, int target = 0, int reward = 0);

    // 更新任务进度
    void updateProgress(int amount = 1);

    // 检查任务是否完成
    bool isComplete() const;

    // 获取任务进度文本
    QString getProgressText() const;
};

// NPC类
class NPC : public QObject
{
    Q_OBJECT

public:
    // 构造函数
    NPC(QPoint position, QString name, QObject *parent = nullptr);

    // NPC尺寸（与敌人相同）
    const int width = 80;
    const int height = 80;

    // 获取位置
    int getX() const;
    int getY() const;
    void setPosition(int newX, int newY);

    // 获取NPC名称
    QString getName() const;

    // 设置问候语
    void setGreeting(QString greet);

    // 设置交互范围
    void setInteractionRange(int range);

    // 获取当前图片索引
    int getCurrentImage() const;

    // 检查玩家是否在交互范围内
    bool isPlayerInRange(Player* player) const;

    // 任务管理
    void addQuest(const Quest& quest);
    std::vector<Quest*> getAvailableQuests();  // 获取可接取的任务
    std::vector<Quest*> getActiveQuests();     // 获取进行中的任务
    std::vector<Quest*> getCompletedQuests();  // 获取已完成的任务

    // 任务操作
    bool acceptQuest(int questId);             // 接受任务
    bool completeQuest(int questId);           // 完成任务
    bool claimReward(int questId, int& outReward); // 领取奖励

    // 获取对话内容
    QString getDialogue();

    // 更新动画
    void updateAnimation();

    // 检查是否有交互内容
    bool hasInteraction() const;

signals:
    // 任务相关信号
    void questAccepted(int questId);
    void questCompleted(int questId);
    void rewardClaimed(int questId, int crystals);
    void dialogueTriggered(QString dialogue);

private:
    int x, y;                           // NPC位置
    QString npcName;                    // NPC名称
    QString greeting;                   // 问候语
    int interactionRange;               // 交互范围（像素）
    std::vector<Quest> quests;          // 任务列表

    // 动画相关
    int currentImage;                   // 当前图片索引
    int animationFrame;                 // 动画帧计数
    const int ANIMATION_SPEED = 30;     // 动画速度（帧数）
};

#endif // NPC_H
