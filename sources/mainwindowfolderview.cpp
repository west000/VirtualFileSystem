#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 在文件夹显示列表中弹出菜单
 */
void MainWindow::popMenuInFv(const QPoint &pos)
{
    QTreeWidget *treeWidget = ui->treeWidget_2;
    QTreeWidgetItem *curItem = treeWidget->itemAt(pos);

    QAction newItem(QString::fromLocal8Bit("新建文件夹"), this);
    QAction newFileItem(QString::fromLocal8Bit("新建文件"), this);
    QAction openItem(QString::fromLocal8Bit("打开"), this);
    QAction copyItem(QString::fromLocal8Bit("复制"), this);
    QAction cutItem(QString::fromLocal8Bit("剪切"), this);
    QAction pasteItem(QString::fromLocal8Bit("粘贴"), this);
    QAction deleteItem(QString::fromLocal8Bit("删除"), this);
    QAction renameItem(QString::fromLocal8Bit("重命名"), this);
    QAction property(QString::fromLocal8Bit("属性"), this);

    connect(&newItem, &QAction::triggered, this, &MainWindow::newFolderInFV);
    connect(&newFileItem, &QAction::triggered, this, &MainWindow::newFileInFV);
    connect(&openItem, &QAction::triggered, this, &MainWindow::openInFV);
    connect(&deleteItem, &QAction::triggered, this, &MainWindow::deleteInFV);
    connect(&copyItem, &QAction::triggered, this, &MainWindow::copyInFV);
    connect(&cutItem, &QAction::triggered, this, &MainWindow::cutInFV);
    connect(&pasteItem, &QAction::triggered, this, &MainWindow::pasteInFV);
    connect(&renameItem, &QAction::triggered, this, &MainWindow::renameInFV);
    connect(&property, &QAction::triggered, this,&MainWindow::popPropertyInFV);

    QMenu menu(treeWidget);
    if(curItem != nullptr)
    {
        qDebug() << "current select file:" <<curItem->text(0);
        menu.addAction(&openItem);
        menu.addAction(&copyItem);
        menu.addAction(&cutItem);
        menu.addAction(&deleteItem);
//        menu.addAction(&renameItem);
        isNoItemSelect = false;
    }
    else
    {
        menu.addAction(&newItem);
        menu.addAction(&newFileItem);
        if(isCopy || isCut)
            menu.addAction(&pasteItem);
        isNoItemSelect = true;
    }
    menu.addAction(&property);
    menu.exec(QCursor::pos());
}

/*
 * 在文件夹显示列表中弹出属性框
 */
void MainWindow::popPropertyInFV()
{
    qDebug() << "property in fv";
    QTreeWidget *tree = ui->treeWidget_2;
    if(currentPath.isEmpty())
        return;

    QList<QString> path = currentPath;
    if(isNoItemSelect)
    {
        qDebug() << "no file selected";
    }
    else
    {
        QTreeWidgetItem *curItem = tree->currentItem();
        if(curItem)
        {
            QString filename = curItem->text(0);
            path.push_back(filename);
        }
    }
    QList<QString> posPath = path;
    FileTreeNode *file = findFileByPath(path);
    if(file)
    {
        showPropertyWidget(file, posPath);
    }
    else
    {
        if(isNoItemSelect == false)
            qDebug() << "can not find " << tree->currentItem()->text(0) << " when pop property";
    }
}

/*
 * 在文件夹显示列表中新建文件夹
 */
void MainWindow::newFolderInFV()
{
    qDebug() << "new folder";
    QList<QString> path = currentPath;
    FileTreeNode *file = findFileByPath(path);

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
            // 更新文件树
            path = currentPath;
            QTreeWidgetItem *dst = findItemByPathInTree(path);
            UpdateFileTreeWidget(dst, result);
            // 更新文件显示列表
            addItemToFolderView(result);
            TraverseFileTree(manager->root);
        }
    }
}

/*
 * 在文件夹显示列表中新建文件
 */
void MainWindow::newFileInFV()
{
    qDebug() << "new folder";
    QList<QString> path = currentPath;
    FileTreeNode *file = findFileByPath(path);

    if(file)
    {
        qDebug() << "create a new folder in " << file->getFileName().c_str();
        string filename("新建文件");
        int index = 1;
        while(SearchFileInCurDir(static_cast<Directory*>(file), filename))
        {
            filename = string("新建文件 (");
            filename += QString::number(++index).toStdString();
            filename += ")";
        }
        bool hasSameFile;
        FileTreeNode *result;
        if(manager->AddFile(static_cast<Directory*>(file), hasSameFile, filename, ReadWrite, "", &result))
        {
            // 更新文件树
            path = currentPath;
            QTreeWidgetItem *dst = findItemByPathInTree(path);
            UpdateFileTreeWidget(dst, result);
            // 更新文件显示列表
            addItemToFolderView(result);
            TraverseFileTree(manager->root);
        }
    }
}

/*
 * 在文件夹显示列表中打开文件或者文件夹
 */
void MainWindow::openInFV()
{
    qDebug() << "open in folder view";
    QTreeWidget *tree = ui->treeWidget_2;
    QTreeWidgetItem *curItem = tree->currentItem();
    if(curItem == nullptr)
        return;

    QString filename = curItem->text(0);
    currentPath.push_back(filename);
    QList<QString> path = currentPath;
    qDebug() << "curPath:" << Utility::pathListToQString(path);
    FileTreeNode *file = findFileByPath(path);
    if(file)
    {
        if(file->getFileType() == Dir)
        {
            qDebug() << "open folder";
            updateFolderView(static_cast<Directory*>(file));
        }
        else
        {
            qDebug() << "open file";
            currentPath.pop_back();
            openFileInFV(static_cast<File*>(file));
        }
    }
    else
    {
        qDebug() << "can not find file" << filename;
        currentPath.pop_back();
    }
}

/*
 * 弹出文本文件查看框
 */
void MainWindow::openFileInFV(File *file)
{
    isFileContentModify = false;
    isFileNameModify = false;

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(QString::fromLocal8Bit(file->getFileName().c_str()));
    dialog->setFixedHeight(400);
    dialog->setFixedWidth(350);

    QLabel *nameLabel = new QLabel(dialog);
    nameLabel->setText(QString::fromLocal8Bit("文件名"));
    QLineEdit *nameEdit = new QLineEdit(dialog);
    nameEdit->setText(QString::fromLocal8Bit(file->getFileName().c_str()));

    textEdit = new QTextEdit(dialog);
    string content;
    manager->readContentFromDisk(file->getFirstBlock(), content);
    textEdit->setText(Utility::stringToQString(content));

    QRadioButton *roButton = new QRadioButton(dialog);
    roButton->setText(QString::fromLocal8Bit("只读"));
    QRadioButton *rwButton = new QRadioButton(dialog);
    rwButton->setText(QString::fromLocal8Bit("读写"));

    if(file->getFilePermission() == ReadOnly)
    {
        isFileReadOnly = true;
        roButton->setChecked(true);
        textEdit->setReadOnly(true);
    }
    else
    {
        isFileReadOnly = false;
        rwButton->setChecked(true);
        textEdit->setReadOnly(false);
    }

    connect(roButton, &QRadioButton::clicked,
            this, &MainWindow::fileReadOnly);
    connect(rwButton, &QRadioButton::clicked,
            this, &MainWindow::fileReadWrite);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(nameLabel);
    hLayout->addWidget(nameEdit);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(rwButton);
    hLayout2->addWidget(roButton);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    vLayout->addLayout(hLayout2);
    vLayout->addWidget(textEdit);
    dialog->setLayout(vLayout);
    dialog->exec();

    qDebug() << "file close";

    if(isFileReadOnly)
    {
        if(file->getFilePermission() != ReadOnly)
            manager->ModifyFilePermission(static_cast<FileTreeNode*>(file), ReadOnly);
    }
    else
    {
        if(file->getFilePermission() != ReadWrite)
            manager->ModifyFilePermission(static_cast<FileTreeNode*>(file), ReadWrite);
    }

    string filename = file->getFileName();
    if(nameEdit->text() != QString::fromLocal8Bit(filename.c_str()))
    {
        string newName = Utility::qstringtoString(nameEdit->text());
        FileTreeNode *dir = file->getParent();
        if(dir)
        {
            if(SearchFileInCurDir(static_cast<Directory*>(dir), newName) != nullptr)
            {
                QString str = QString::fromLocal8Bit("文件名'") + nameEdit->text() + QString::fromLocal8Bit("'已存在");
                QMessageBox::warning(this, QString::fromLocal8Bit("文件重命名失败"), str, QMessageBox::Yes);
            }
            else
            {
                // 文件名改变
                isFileNameModify = true;
                filename = newName;
            }
        }
    }

    if(content != Utility::qstringtoString(textEdit->toPlainText()))
    {
        isFileContentModify = true;
        content = Utility::qstringtoString(textEdit->toPlainText());
    }

    if(isFileContentModify || isFileNameModify)
    {
        manager->ModifyFileContent(file, filename, content);
        updateFolderView();
    }
}

/*
 * 在文件夹显示列表中复制文件
 */
void MainWindow::copyInFV()
{
    qDebug() << "copy file in folder view";

    isCopy = true;
    QTreeWidget *tree = ui->treeWidget_2;
    QTreeWidgetItem *curItem = tree->currentItem();
    if(curItem == nullptr)
        return;

    QString filename = curItem->text(0);
    QList<QString> path = currentPath;
    path.push_back(filename);
    FileTreeNode *file = findFileByPath(path);

    if(file)
    {
        qDebug() << "copy file:" << QString::fromLocal8Bit(file->getFileName().c_str());
        copyFileNode = file;
    }
    else
        isCopy = false;
}

/*
 * 在文件夹显示列表中剪切文件
 */
void MainWindow::cutInFV()
{
    qDebug() << "cut file in folder view";

    isCut = true;
    QTreeWidget *tree = ui->treeWidget_2;
    QTreeWidgetItem *curItem = tree->currentItem();
    if(curItem == nullptr)
        return;

    QString filename = curItem->text(0);
    QList<QString> path = currentPath;
    path.push_back(filename);
    FileTreeNode *file = findFileByPath(path);

    if(file)
    {
        qDebug() << "cut file:" << QString::fromLocal8Bit(file->getFileName().c_str());
        cutFileNode = file;
        path = currentPath;
        path.push_back(filename);
        cutFileItemInFV = curItem;
        cutFileItem = findItemByPathInTree(path);
    }
    else
    {
        isCut = false;
        qDebug() << "cutInFV: can not find file" << filename;
    }
}

/*
 * 在文件夹显示列表中粘贴文件
 */
void MainWindow::pasteInFV()
{
    if(isCopy)
    {
        isCopy = false;
        if(copyFileNode == nullptr)
        {
            qDebug() << "copy paste failed";
            return;
        }
        QList<QString> path = currentPath;
        FileTreeNode *dir = findFileByPath(path);
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
            path = currentPath;
            QTreeWidgetItem *item = findItemByPathInTree(path);
            if(copyFileNode->getFileType() == Dir)
                UpdateFileTreeWidget(item, copyFileNode);
            addItemToFolderView(copyFileNode);
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
    else if(isCut)
    {
        isCut = false;
        if(cutFileNode == nullptr)
        {
            qDebug() << "cut paste failed";
            return ;
        }

        QList<QString> path = currentPath;
        FileTreeNode *dir = findFileByPath(path);
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
                path = currentPath;
                QTreeWidgetItem *item = findItemByPathInTree(path);
                UpdateFileTreeWidget(item, cutFileNode);
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
}

/*
 * 在文件夹显示列表中删除文件
 */
void MainWindow::deleteInFV()
{
    qDebug() << "delete in folder view";

    QTreeWidget *tree = ui->treeWidget_2;
    QTreeWidgetItem *curItem = tree->currentItem();
    if(curItem == nullptr)
        return;

    QString filename = curItem->text(0);
    QList<QString> path = currentPath;
    path.push_back(filename);
    FileTreeNode *file = findFileByPath(path);

    if(file)
    {
        path = currentPath;
        path.push_back(filename);
        if(file->getFileType() == Dir)
        {
            qDebug() << "delete folder" << QString::fromLocal8Bit(file->getFileName().c_str());

            QTreeWidgetItem *itemInTree = findItemByPathInTree(path);
            delete itemInTree;
        }
        else
        {
            qDebug() << "delete file" << QString::fromLocal8Bit(file->getFileName().c_str());
        }
        manager->DeleteFile(file);
        delete curItem;
        TraverseFileTree(manager->root);
    }
}

/*
 * 在文件夹显示列表中重命名
 */
void MainWindow::renameInFV()
{

}

/*
 * 显示属性框
 */
void MainWindow::showPropertyWidget(FileTreeNode *file, const QList<QString> &path)
{
    if(file)
    {
        QWidget *widget = new QWidget();
        widget->setParent(nullptr);
        widget->resize(330, 400);
        widget->setWindowTitle(QString::fromLocal8Bit("属性"));
        QLabel *nameLabel = new QLabel(widget);
        nameLabel->setText(QString::fromLocal8Bit("名称：") + Utility::stringToQString(file->getFileName()));
        QLabel *typeLabel = new QLabel(widget);
        if(file->getFileType() == Dir)
            typeLabel->setText(QString::fromLocal8Bit("类型：文件夹"));
        else
            typeLabel->setText(QString::fromLocal8Bit("类型：文本文件"));
        QLabel *posLabel = new QLabel(widget);
        posLabel->setText(QString::fromLocal8Bit("位置：") + Utility::pathListToQString(path));
        QLabel *sizeLabel = new QLabel(widget);
        sizeLabel->setText(QString::fromLocal8Bit("大小：") + QString::number(file->getFileSize()) + QString::fromLocal8Bit(" 字节"));
        QLabel *ctLabel = new QLabel(widget);
        ctLabel->setText(QString::fromLocal8Bit("创建时间：") + QString::fromLocal8Bit(file->getCreateTime().c_str()));
        QLabel *mtLabel = new QLabel(widget);
        mtLabel->setText(QString::fromLocal8Bit("修改时间：") + QString::fromLocal8Bit(file->getModifyTime().c_str()));

        QVBoxLayout *vLayout = new QVBoxLayout;
        vLayout->addWidget(nameLabel);
        vLayout->addWidget(typeLabel);
        vLayout->addWidget(posLabel);
        vLayout->addWidget(sizeLabel);
        vLayout->addWidget(ctLabel);
        vLayout->addWidget(mtLabel);

        widget->setLayout(vLayout);
        widget->show();
    }
}
