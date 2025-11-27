#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("客户端");
}

Widget::~Widget()
{
    delete ui;
}

// 发送信息
void Widget::on_btn_send_clicked()
{
    if(m_client == nullptr)
    {
        QMessageBox::critical(this, "警告", "先连接服务器再发送消息");
        return;
    }

    QString name = ui->le_name->text().trimmed();
    QString text = ui->le_text->text().trimmed();
    if(name.isEmpty() || text.isEmpty())
    {
        QMessageBox::critical(this, "警告", "用户名和消息均不能为空");
        return;
    }

    // 发送
    m_client->write(QString(name + "\r\r" + text).toUtf8());
    ui->plainTextEdit->appendPlainText(name + ":" + text);
}

// 连接/取消
void Widget::on_btn_start_clicked()
{
    if(ui->btn_start->text() == "开始连接")
    {
        // 连接检查
        QString address = ui->le_address->text().trimmed();
        QString port = ui->le_port->text().trimmed();
        if(address.isEmpty() || port.isEmpty())
        {
            QMessageBox::critical(this, "警告", "服务器地址和端口不能为空");
            return;
        }
        bool ok = false;
        quint16 iport = port.toUInt(&ok);
        if(!ok)
        {
            QMessageBox::critical(this, "警告", "端口号必须是整数");
            return;
        }

        // 连接服务器
        m_client = new QTcpSocket(this);
        m_client->connectToHost(address, iport);
        connect(m_client, &QTcpSocket::readyRead, this, [=]()
        {
            QString msg = QString(m_client->readAll());
            msg = msg.replace("\r\r", ":");
            ui->plainTextEdit->appendPlainText(msg);
        });

        ui->plainTextEdit->appendPlainText("连接服务器成功!");
        ui->btn_start->setText("取消连接");
    }
    else
    {
        m_client->close();
        m_client->deleteLater();
        m_client = nullptr;
        ui->plainTextEdit->appendPlainText("断开连接!");
        ui->btn_start->setText("开始连接");
    }
}
