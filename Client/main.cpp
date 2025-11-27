#include "widget.h"
#include "logindialog.h"
#include "registerdialog.h" // 包含新创建的注册对话框头文件

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 2. 先创建登录对话框的实例
    //LoginDialog loginDialog;

    // 1. 创建所有需要的窗口实例
    Widget w;
    LoginDialog loginDialog;
    RegisterDialog registerDialog;

    while (true)
    {
        // 3. 显示登录对话框，并获取其返回结果
        int loginResult = loginDialog.exec();

        // 4. 判断登录对话框的返回结果
        if (loginResult == QDialog::Accepted) {
            // 4.1 用户点击了 "登录" 并且验证成功
            // 跳出循环，准备显示主窗口
            break;
        }
        else if (loginResult == LoginDialog::RegisterRequest) {
            // 4.2 用户点击了 "注册账号"
            // 显示注册对话框
            // 注意：这里我们不关心注册对话框的返回值 (accept/reject)
            // 因为无论注册成功还是点击返回，都应该回到登录界面
            registerDialog.exec();
            // 循环将继续，再次显示登录对话框
        }
        else {
            // 4.3 用户点击了 "取消" 或直接关闭了登录对话框
            // 程序直接退出，不显示主窗口
            return 0;
        }
    }

    // 5. 只有在成功登录后 (即循环被 break)，才显示主窗口
    w.show();

    // 6. 进入Qt的事件循环
    return a.exec();
}
