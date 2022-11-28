/********************************************************************************
** Form generated from reading UI file 'color_quick_GUI.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLOR_QUICK_GUI_H
#define UI_COLOR_QUICK_GUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_color_quick_GUIClass
{
public:
    QWidget *centralWidget;
    QLabel *label;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *color_quick_GUIClass)
    {
        if (color_quick_GUIClass->objectName().isEmpty())
            color_quick_GUIClass->setObjectName("color_quick_GUIClass");
        color_quick_GUIClass->resize(600, 400);
        centralWidget = new QWidget(color_quick_GUIClass);
        centralWidget->setObjectName("centralWidget");
        label = new QLabel(centralWidget);
        label->setObjectName("label");
        label->setGeometry(QRect(140, 140, 171, 121));
        color_quick_GUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(color_quick_GUIClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 600, 22));
        color_quick_GUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(color_quick_GUIClass);
        mainToolBar->setObjectName("mainToolBar");
        color_quick_GUIClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(color_quick_GUIClass);
        statusBar->setObjectName("statusBar");
        color_quick_GUIClass->setStatusBar(statusBar);

        retranslateUi(color_quick_GUIClass);

        QMetaObject::connectSlotsByName(color_quick_GUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *color_quick_GUIClass)
    {
        color_quick_GUIClass->setWindowTitle(QCoreApplication::translate("color_quick_GUIClass", "color_quick_GUI", nullptr));
        label->setText(QCoreApplication::translate("color_quick_GUIClass", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class color_quick_GUIClass: public Ui_color_quick_GUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLOR_QUICK_GUI_H
