#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 清空文件夹显示列表
 */
void MainWindow::clearFolderView()
{
    QTreeWidget *tree = ui->treeWidget_2;
    while(tree->topLevelItemCount())
    {
        delete tree->topLevelItem(0);
    }
}

/*
 * 在左侧点击右键，弹出菜单
 */
void MainWindow::popMenu(const QPoint &pos)
{
    QTreeWidget *treeWidget = ui->treeWidget;
    QTreeWidgetItem *curItem = treeWidget->itemAt(pos);

    if(isRename)
        nameChanged();

    if(curItem == nullptr)
        return ;

    QString text = curItem->text(0);
    qDebug() << text;

    QAction newItem(QString::fromLocal8Bit("新建文件夹"), this);
    QAction copyItem(QString::fromLocal8Bit("复制"), this);
    QAction cutItem(QString::fromLocal8Bit("剪切"), this);
    QAction pasteItem(QString::fromLocal8Bit("粘贴"), this);
    QAction deleteItem(QString::fromLocal8Bit("删除"), this);
    QAction renameItem(QString::fromLocal8Bit("重命名"), this);
    QAction property(QString::fromLocal8Bit("属性"), this);

    connect(&newItem, &QAction::triggered, this, &MainWindow::newItem);
    connect(&deleteItem, &QAction::triggered, this, &MainWindow::deleteItem);
    connect(&copyItem, &QAction::triggered, this, &MainWindow::copyItem);
    connect(&cutItem, &QAction::triggered, this, &MainWindow::cutItem);
    connect(&pasteItem, &QAction::triggered, this, &MainWindow::pasteItem);
    connect(&renameItem, &QAction::triggered, this, &MainWindow::renameItem);
    connect(&property, &QAction::triggered, this,&MainWindow::popProperty);

    QMenu menu(treeWidget);
    menu.addAction(&newItem);
    menu.addAction(&copyItem);
    menu.addAction(&cutItem);
    if(isCopy || isCut)
        menu.addAction(&pasteItem);
    menu.addAction(&deleteItem);
    menu.addAction(&renameItem);
    menu.addAction(&property);
    menu.exec(QCursor::pos());
}

/*
 * 属性框
 */
void MainWindow::popProperty()
{
    qDebug() << "property";

    QTreeWidget *treeWidget = ui->treeWidget;
    QTreeWidgetItem *curItem = treeWidget->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    QList<QString> path;
    getCurPath(curItem, path);
    // 更新当前路径
    updateCurrentPath(path);
    if(file)
    {
        showPropertyWidget(file, path);
    }
    else
    {
        qDebug() << "can not find file";
    }
}

/*
 * 新建文件夹
 */
void MainWindow::newItem()
{
    qDebug() << "newItem";
    QTreeWidget *tree = ui->treeWidget;
    QTreeWidgetItem *curItem = tree->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    if(file)
    {
        qDebug() << "create a new folder in " << file->getFileName().c_str();
        string filename("新建文件夹");
        int index = 1;
        while(SearchFileInCurDir(static_cast<Directory*>(file), filename))
        {
            filename = string("新建文件夹 (");
            filename += QString::number(++index).toStdString();
            filename += ")";
        }
        bool hasSameFolder;
        Directory *result;
        if(manager->AddFolder(static_cast<Directory*>(file), hasSameFolder, filename, ReadWrite, &result))
        {
            UpdateFileTreeWidget(curItem, result);
            TraverseFileTree(manager->root);
        }
    }
}

/*
 * 复制文件
 */
void MainWindow::copyItem()
{
    qDebug() << "copytItem";
    isCopy = true;
    QTreeWidget *tree = ui->treeWidget;
    QTreeWidgetItem *curItem = tree->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    if(file)
    {
        qDebug() << "copy file:" << file->getFileName().c_str();
        copyFileNode = file;
    }
    else
        isCopy = false;
}

/*
 * 剪切文件
 */
void MainWindow::cutItem()
{
    qDebug() << "cutItem";
    isCut = true;
    QTreeWidget *tree = ui->treeWidget;
    QTreeWidgetItem *curItem = tree->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    if(file)
    {
        qDebug() << "cut file:" << file->getFileName().c_str();
        cutFileNode = file;
        cutFileItem = curItem;
    }
    else
        isCut = false;
}

/*
 * 粘贴文件
 */
void MainWindow::pasteItem()
{
    if(isCopy)
    {
        isCopy = false;
        if(copyFileNode != nullptr)
        {
            QTreeWidget *tree = ui->treeWidget;
            QTreeWidgetItem *curItem = tree->currentItem();
            FileTreeNode *dir = getFileByItem(curItem);
            qDebug() << "paste file:" << copyFileNode->getFileName().c_str() << " to " << dir->getFileName().c_str();

            if(dir == copyFileNode)
            {
                QString str = QString::fromLocal8Bit("目标文件夹是源文件夹的子文件夹");
                QMessageBox::critical(this, QString::fromLocal8Bit("粘贴失败"),
                                        str, QMessageBox::Yes);
                return ;
            }

            bool hasCrash;
            if(manager->CopyAndPaste(static_cast<Directory*>(dir), copyFileNode, hasCrash))
            {
                qDebug() << "copy and paste success";
                TraverseFileTree(manager->root);
                if(copyFileNode->getFileType() == Dir)
                    UpdateFileTreeWidget(curItem, copyFileNode);
                updateFolderView();
            }
            else if(hasCrash)
            {
                QString str = QString::fromLocal8Bit("此目标已经包含名为'");
                str += QString::fromLocal8Bit(copyFileNode->getFileName().c_str()) + QString::fromLocal8Bit("'的文件");
                QMessageBox::critical(this, QString::fromLocal8Bit("粘贴失败"),
                                        str, QMessageBox::Yes);
            }
            else
            {
                QMessageBox::critical(this, QString::fromLocal8Bit("粘贴失败"),
                                        QString::fromLocal8Bit("磁盘空间不足"), QMessageBox::Yes);
            }
        }
        else
        {
            qDebug() << "copy paste failed";
        }
    }
    else if(isCut)
    {
        isCut = false;
        if(cutFileNode != nullptr)
        {
            QTreeWidget *tree = ui->treeWidget;
            QTreeWidgetItem *curItem = tree->currentItem();
            FileTreeNode *dir = getFileByItem(curItem);
            qDebug() << "paste file:" << cutFileNode->getFileName().c_str() << " to " << dir->getFileName().c_str();

            if(dir == cutFileNode)
            {
                QString str = QString::fromLocal8Bit("目标文件夹是源文件夹的子文件夹");
                QMessageBox::critical(this, QString::fromLocal8Bit("粘贴失败"),
                                        str, QMessageBox::Yes);
                return ;
            }

            bool isSuccess = manager->CutAndPaste(static_cast<Directory*>(dir), cutFileNode);
            if(isSuccess)
            {
                qDebug() << "cut and paste success";
                TraverseFileTree(manager->root);
                if(cutFileNode->getFileType() == Dir)
                {
                    UpdateFileTreeWidget(curItem, cutFileNode);
                }
                updateFolderView();
                qDebug() << "update file tree success";
                // 删除源节点
                if(cutFileItem)
                    delete cutFileItem;
                if(cutFileItemInFV)
                    delete cutFileItemInFV;
            }
            else
            {
                QString str = QString::fromLocal8Bit("此目标已经包含名为'");
                str += QString::fromLocal8Bit(cutFileNode->getFileName().c_str()) + QString::fromLocal8Bit("'的文件");
                QMessageBox::critical(this, QString::fromLocal8Bit("粘贴失败"),
                                        str, QMessageBox::Yes);
            }
        }
        else
        {
            qDebug() << "cut paste failed";
        }
    }
}

/*
 * 删除文件
 */
void MainWindow::deleteItem()
{
    qDebug() << "deleteItem";

    QTreeWidget *tree = ui->treeWidget;
    QTreeWidgetItem *curItem = tree->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    if(file)
    {
        qDebug() << "delete file:" << file->getFileName().c_str();
        if(file == manager->root)
        {
            QMessageBox::critical(this, QString::fromLocal8Bit("删除失败"),
                                  QString::fromLocal8Bit("不能删除根目录！"), QMessageBox::Yes);
            return ;
        }
        manager->DeleteFile(file);
        TraverseFileTree(manager->root);
        delete curItem;

        updateFolderView();
    }
}

/*
 * 重命名
 */
void MainWindow::renameItem()
{
    qDebug() << "renameItem" ;
    isRename = true;

    QTreeWidget *tree = ui->treeWidget;
    QTreeWidgetItem *curItem = tree->currentItem();
    FileTreeNode *file = getFileByItem(curItem);
    if(file)
    {
        qDebug() << "rename file:" << QString::fromLocal8Bit(file->getFileName().c_str());
        renameFileNode = file;
        renameFileItem = curItem;
        curItem->setFlags(curItem->flags() | Qt::ItemIsEditable);
        tree->editItem(curItem, 0);
        connect(tree,&QTreeWidget::itemSelectionChanged,
                this, &MainWindow::nameChanged);
    }
    else
    {
        qDebug() << "can not find " << curItem->text(0);
    }
}

void MainWindow::nameChanged()      // QTreeWidgetItem *item, int index
{
    if(isRename)
    {
        isRename = false;
        QTreeWidgetItem *item = renameFileItem;

        if(renameFileNode)
        {
            QString oldName = QString::fromLocal8Bit(renameFileNode->getFileName().c_str());
            //if(oldName == QString::fromLocal8Bit(item->text(0).toStdString().c_str()))
            if(renameFileNode->getFileName() == Utility::qstringtoString(item->text(0)))
            {
                qDebug() << "the file name do not change";
                return;
            }
            qDebug() << "the file name change";
            QString name = item->text(0);
            string newName = name.toStdString();
            FileTreeNode *parent = renameFileNode->getParent();
            if(parent != nullptr)
            {
                if(SearchFileInCurDir(static_cast<Directory*>(parent), newName))
                {
                    item->setText(0, oldName);
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                    renameFileNode = nullptr;

                    QString str = QString::fromLocal8Bit("此目标已经包含名为'");
                    str += QString::fromLocal8Bit(newName.c_str()) + QString::fromLocal8Bit("'的文件");
                    QMessageBox::critical(this, QString::fromLocal8Bit("文件名已存在"),
                                            str, QMessageBox::Yes);
                    return;
                }
            }

            // 重命名成功，修改文件目录
            qDebug() << "rename success";
            qDebug() << "modify " << QString::fromLocal8Bit(renameFileNode->getFileName().c_str());
            manager->RenameFile(*renameFileNode, Utility::qstringtoString(name));
            // 设置为不可编辑
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            updateFolderView();
            renameFileNode = nullptr;
        }
    }
}
