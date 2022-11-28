#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_color_quick_GUI.h"

class color_quick_GUI : public QMainWindow
{
    Q_OBJECT

public:
    color_quick_GUI(QWidget *parent = nullptr);
    ~color_quick_GUI();

private:
    Ui::color_quick_GUIClass ui;
};
