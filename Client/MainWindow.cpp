// MainWindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AddFriendDialog.h"
#include <QListWidgetItem> // 如果槽函数参数用到了，需要包含头文件
#include "networkmanager.h"

// ... 其他代码 ...

// 构造函数可能已经有了
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ...
    connect(&NetworkManager::instance(), &NetworkManager::autoAcceptFriendRequest, this, &MainWindow::onAutoAcceptFriendRequest);
    // --- 添加初始的假数据 ---
    m_friends[1001] = "张三";
    m_friends[1002] = "李四";
    m_groups[2001] = "家庭群";

    // 启动时刷新一次列表
    updateConversationList();
}

// ！！！确保析构函数的实现存在 ！！！
MainWindow::~MainWindow()
{
    delete ui;
}

// ！！！为所有槽函数添加实现 ！！！

void MainWindow::on_sendButton_clicked()
{
    // 这里暂时可以为空，但函数体必须存在
}

void MainWindow::on_conversationListWidget_itemClicked(QListWidgetItem *item)
{
    (void)item;
}

void MainWindow::on_addFriendButton_clicked() // 假设你的按钮槽函数是这个名字
{
    AddFriendDialog dialog(this);

    // dialog.exec() 会显示对话框并等待它关闭
    // 如果我们在对话框内部调用了 accept(), exec() 会返回 QDialog::Accepted
    if (dialog.exec() == QDialog::Accepted) {
        // 1. 如果添加成功，就从对话框获取新好友的ID
        int newFriendId = dialog.getAddedFriendId();

        // 2. 检查ID是否有效，并更新本地好友列表
        if (newFriendId != -1 && !m_friends.contains(newFriendId)) {
            // 为了显示，我们先给一个默认名字
            QString newFriendName = QString("好友 %1").arg(newFriendId);
            m_friends[newFriendId] = newFriendName;

            // 3. 刷新界面上的好友列表
            updateConversationList();

        }
    }
    // 如果用户点击了 "Cancel" 或者添加失败后关闭了窗口，exec() 会返回 Rejected，我们什么都不做
}

void MainWindow::on_createGroupButton_clicked()
{
    CreateGroupDialog dialog(this);

    // 1. 把当前的好友列表传给对话框
    dialog.setFriendsList(m_friends);

    // 2. 显示对话框并等待用户操作
    if (dialog.exec() == QDialog::Accepted) {
        // 3. 从对话框获取结果
        QString groupName = dialog.getGroupName();
        QVector<uint8_t> memberIds = dialog.getSelectedMemberIDs(); // 假设返回的是ID

        // 4. 为了测试，我们创建一个新的群ID
        // 简单的逻辑：取当前最大群ID+1，或者一个随机数
        int newGroupId = 2001;
        if (!m_groups.isEmpty()) {
            newGroupId = m_groups.lastKey() + 1;
        }

        // 5. 更新我们的“本地数据库”
        m_groups[newGroupId] = groupName;

        // 6. 刷新主界面
        updateConversationList();

        // (可选) 在控制台打印出群成员，确认选择正确
        qDebug() << "创建了新群聊:" << groupName << "ID:" << newGroupId;
        qDebug() << "群成员ID:";
        for(uint8_t id : memberIds) {
            qDebug() << id;
        }
    }
}

void MainWindow::updateConversationList()
{
    // 1. 先清空当前的列表
    ui->conversationListWidget->clear();

    // 2. 添加所有好友到列表
    for(auto it = m_friends.constBegin(); it != m_friends.constEnd(); ++it) {
        QString itemText = QString("好友: %1 (%2)").arg(it.value()).arg(it.key());
        ui->conversationListWidget->addItem(itemText);
    }

    // 3. 添加所有群聊到列表
    for(auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
        QString itemText = QString("群聊: %1 (%2)").arg(it.value()).arg(it.key());
        ui->conversationListWidget->addItem(itemText);
    }
}

void MainWindow::onAutoAcceptFriendRequest(uint8_t requesterId)
{
    qDebug() << "[MainWindow] Auto-accepting and adding friend:" << requesterId;

    // 1. 检查是否已经是好友了，防止重复添加
    if (m_friends.contains(requesterId)) {
        qDebug() << "[MainWindow] Friend" << requesterId << "already exists.";
        // 即使已经是好友，也应该回复一个成功的消息，让对方能完成添加流程
    } else {
        // 2. 添加到你的好友数据中 (m_friends)
        // 我们暂时不知道新好友的名字，所以先用ID作为临时名字
        QString temporaryName = QString("用户 %1").arg(requesterId);
        m_friends.insert(requesterId, temporaryName);

        // 3. 调用你已有的函数刷新UI
        updateConversationList();
    }

    // 4. [关键] 无论对方是否已经是好友，都回复服务器，告诉它你已经“同意”了
    // 这样可以确保发起请求的A端能够收到成功的响应
    uint8_t selfId = NetworkManager::selfId(); // [注意] 这里需要获取当前用户的真实ID
    NetworkManager::instance().sendAddFriendResponse(requesterId, selfId, true);
}
