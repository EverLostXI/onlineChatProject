/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *plainTextEdit;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *le_text;
    QPushButton *btn_send;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *le_name;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *le_address;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLineEdit *le_port;
    QPushButton *btn_start;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(350, 479);
        verticalLayout = new QVBoxLayout(Widget);
        verticalLayout->setObjectName("verticalLayout");
        plainTextEdit = new QPlainTextEdit(Widget);
        plainTextEdit->setObjectName("plainTextEdit");

        verticalLayout->addWidget(plainTextEdit);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        le_text = new QLineEdit(Widget);
        le_text->setObjectName("le_text");

        horizontalLayout_4->addWidget(le_text);

        btn_send = new QPushButton(Widget);
        btn_send->setObjectName("btn_send");

        horizontalLayout_4->addWidget(btn_send);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label_2 = new QLabel(Widget);
        label_2->setObjectName("label_2");

        horizontalLayout->addWidget(label_2);

        le_name = new QLineEdit(Widget);
        le_name->setObjectName("le_name");

        horizontalLayout->addWidget(le_name);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_3 = new QLabel(Widget);
        label_3->setObjectName("label_3");

        horizontalLayout_2->addWidget(label_3);

        le_address = new QLineEdit(Widget);
        le_address->setObjectName("le_address");

        horizontalLayout_2->addWidget(le_address);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label = new QLabel(Widget);
        label->setObjectName("label");

        horizontalLayout_3->addWidget(label);

        le_port = new QLineEdit(Widget);
        le_port->setObjectName("le_port");

        horizontalLayout_3->addWidget(le_port);


        verticalLayout->addLayout(horizontalLayout_3);

        btn_start = new QPushButton(Widget);
        btn_start->setObjectName("btn_start");

        verticalLayout->addWidget(btn_start);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        btn_send->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\346\234\254\346\234\272\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "\346\234\215\345\212\241\345\231\250\345\234\260\345\235\200\357\274\232", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243\357\274\232", nullptr));
        btn_start->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213\350\277\236\346\216\245", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
