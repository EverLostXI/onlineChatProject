#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = nullptr);
    ~AddFriendDialog();

    // [修改] 这个函数现在用来获取成功添加的好友ID
    int getAddedFriendId() const;

private slots:
    // 这个槽函数会在点击"OK"按钮时被调用
    void onOkButtonClicked();

    // [新增] 这个槽函数用来接收来自NetworkManager的反馈
    void onAddFriendResponse(bool success, uint8_t friendId);

private:
    Ui::AddFriendDialog *ui;
    int m_addedFriendId; // [新增] 用于存储成功添加的好友ID
};

#endif // ADDFRIENDDIALOG_H
