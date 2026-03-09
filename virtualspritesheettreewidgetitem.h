#ifndef VIRTUALSPRITESHEETTREEWIDGETITEM_H
#define VIRTUALSPRITESHEETTREEWIDGETITEM_H

#include "sprite.h"
#include <QWidget>
#include <qtreewidget.h>

class VirtualSpritesheetTreeWidgetItem : public QTreeWidgetItem
{
public:
    using QTreeWidgetItem::QTreeWidgetItem;

    std::shared_ptr<VirtualSpritesheet> virtualSpritesheet;

    VirtualSpritesheetTreeWidgetItem(QTreeWidget *parent = nullptr);
};

#endif // VIRTUALSPRITESHEETTREEWIDGETITEM_H
