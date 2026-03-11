#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ImageData.hpp"
#include <QFileDialog>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>
#include "stringcreator.hpp"
#include <chrono>
#include "spriteopera.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow) {
    SpriteOpera::inst().window = this;
    ui->setupUi(this);

    ui->modeComboBox->addItem("Spritesheet");
    ui->modeComboBox->addItem("Sprite Editor");
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_actionMenubarOpenImageFolder_triggered() { // Open>Folder of images
    // open file dialog
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Choose folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
        );
    if (folderPath.isEmpty()) return;
    QDir folderDir(folderPath);
    // get files from folder
    QFileInfoList fileInfoList = folderDir.entryInfoList(
        QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff" << "*.tif",
        QDir::Files
        );
    QStringList filePaths;
    for (const QFileInfo &fileInfo : std::as_const(fileInfoList)) {
        filePaths << fileInfo.absoluteFilePath();
    }

    // add items to list
    //QStringListModel* stringListModel = new QStringListModel(this);
    //stringListModel->setStringList(filePaths);
    //ui->openImagesListView->setModel(stringListModel);
    // get ImageData from files
    std::vector<std::shared_ptr<IImageData>> imageDatas;
    for (size_t i = 0; i < filePaths.size(); i++) {
        // get file path
        QByteArray utf8ByteArray = filePaths[i].toUtf8();
        const char* fileNameString = utf8ByteArray.constData();

        // print progress
        if (i % 10 == 0 || i == filePaths.size() - 1) {
            float percent = static_cast<float>(i) / static_cast<float>(filePaths.size());
            std::cout << "[INFO] " << generateProgressBar(percent, 50) << "[" << (i + 1) << " / " << filePaths.size() << "] Adding image: " << fileNameString;
            if (i != filePaths.size() - 1) std::cout << "\r";
            std::cout.flush();
        }

        // load image data
        std::shared_ptr<ImageData<unsigned char, 255>> data = std::make_shared<ImageData<unsigned char, 255>>(fileNameString);
        if (data->width == 0) continue; // dont add if image failed to load
        imageDatas.push_back(std::static_pointer_cast<IImageData>(data));
    }
    std::cout << std::endl;

    // rect pack
    std::vector<Rect> freeRects;
    std::vector<Sprite> sprites;
    const double alphaThreshold = 0.1;
    int i = 0;
    for (std::shared_ptr<IImageData>& data : imageDatas) {
        // create region
        std::shared_ptr<BinaryRasterMask> region = std::make_shared<BinaryRasterMask>(Vec2(0, 0), Vec2(data->getWidth(), data->getHeight()));
        for (size_t y = 0; y < data->getHeight(); y++) {
            for (size_t x = 0; x < data->getWidth(); x++) {
                region->set(x, y, data->sampleNormalAlpha(x, y) > alphaThreshold);
            }
        }
        std::cout << "Adding " << (++i) << "/" << imageDatas.size() << std::endl;
        sprites.push_back({data, region});
    }
    std::shared_ptr<VirtualSpritesheet> spritesheet = std::make_shared<VirtualSpritesheet>();
    spritesheet->rectPack(sprites, &freeRects, 10);

    addVirtualSpritesheet(spritesheet);
    for (size_t i = 0; i < freeRects.size(); i++) ui->openGLWidget->addFreeRect(freeRects[i]);
}
void MainWindow::addVirtualSpritesheet(std::shared_ptr<VirtualSpritesheet> spritesheet) {
    // add sprites to openGLWidget sprite list and add item to list
    ui->treeView->setModel(spritesheet.get());

    for (size_t i = 0; i < spritesheet->sprites.size(); i++) {
        // load opengl texture
        spritesheet->sprites[i].sprite.image->loadTexture();
        spritesheet->sprites[i].sprite.normalMap->loadTexture();
    }
    SpriteOpera::inst().spritesheets.push_back(spritesheet);
    SpriteOpera::inst().currentSpritesheetIndex = SpriteOpera::inst().spritesheets.size() - 1;
    ui->openGLWidget->resetZoom();
}


void MainWindow::on_actionMenubarOpenSpritesheet_triggered() {// Open>Spritesheet    // open file dialog
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Choose file"),
        QDir::homePath(),
        tr("Image Files (*.png *.jpg *.bmp);;All Files (*.*)"));

    if (filePath.isEmpty()) return;
    QStringList filePaths;
    filePaths << filePath;

    // get ImageData from files
    QByteArray utf8ByteArray = filePath.toUtf8();
    const char* fileNameString = utf8ByteArray.constData();
    std::shared_ptr<IImageData> imageData = static_pointer_cast<IImageData>(std::make_shared<ImageData<unsigned char, 255>>(fileNameString));
    if (imageData->getWidth() == 0) return;

    // separate spritesheet
    auto start = std::chrono::high_resolution_clock::now();
    std::shared_ptr<VirtualSpritesheet> spritesheet = std::make_shared<VirtualSpritesheet>();
    spritesheet->separate(imageData);

    auto end = std::chrono::high_resolution_clock::now();
    long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "separate time: " << milliseconds << "ms" << std::endl;

    // add sprites to openGLWidget sprite list
    addVirtualSpritesheet(spritesheet);
}




/*void MainWindow::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *nd = ui->treeView->itemAt(pos);
    if (nd == nullptr) return;


    if (auto widgetItem = dynamic_cast<VirtualSpritesheetTreeWidgetItem*>(nd)) {
        QMenu menu(this);
        // actions
        QAction *repackAction = new QAction(QIcon(":/Resource/warning32.ico"), tr("&Repack Spritesheet"), &menu);
        repackAction->setStatusTip(tr("Repack spritesheet"));
        connect(repackAction, &QAction::triggered, widgetItem->virtualSpritesheet.get(), &VirtualSpritesheet::repack);

        QAction *saveToImageAction = new QAction(QIcon(":/Resource/warning32.ico"), tr("&Save to Image"), &menu);
        saveToImageAction->setStatusTip(tr("Rasterize the spritesheet and save to single image file"));
        connect(saveToImageAction, &QAction::triggered, widgetItem->virtualSpritesheet.get(), &VirtualSpritesheet::rasterizeToFile);

        menu.addAction(repackAction);
        menu.addAction(saveToImageAction);

        QPoint pt(pos);
        menu.exec( ui->treeView->mapToGlobal(pos) );
    }
}*/


void MainWindow::on_modeComboBox_currentIndexChanged(int index) {
    switch (index) {
    case 0:
        ui->openGLWidget->changeMode(SPRITESHEET);
        break;
    case 1:
        ui->openGLWidget->changeMode(SPRITEEDITOR);
        break;
    }
}


void MainWindow::on_homeButton_pressed() {
    ui->openGLWidget->resetZoom();
}


void MainWindow::on_rasterizeButton_pressed() {
    if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
        SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex)->rasterizeToFile();
    }
}


void MainWindow::on_toolButton_pressed() {
    if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
        SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex)->repack();
        ui->openGLWidget->update();
    }
}


void MainWindow::on_rasterizeSpritesButton_pressed() {
    if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
        SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex)->rasterizeSpritesToFolder();
    }
}


void MainWindow::on_trimSpritesButton_pressed() {
    if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
        SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex)->trimAllSprites();
        ui->openGLWidget->update();
    }
}

