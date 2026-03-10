#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sprite.hpp"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::MainWindow *ui;
    void addVirtualSpritesheet(std::shared_ptr<VirtualSpritesheet> spritesheet);
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionMenubarOpenImageFolder_triggered();

    void on_actionMenubarOpenSpritesheet_triggered();

    //void on_treeView_customContextMenuRequested(const QPoint &pos);

    void on_modeComboBox_currentIndexChanged(int index);

    void on_homeButton_pressed();

    void on_rasterizeButton_pressed();

    void on_toolButton_pressed();

    void on_rasterizeSpritesButton_pressed();

    void on_trimSpritesButton_pressed();

private:
};
#endif // MAINWINDOW_H
