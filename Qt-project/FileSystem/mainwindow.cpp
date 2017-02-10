#include "mainwindow.h"
#include "ui_mainwindow.h"


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::MainWindow(FSManager *m, QWidget *parent) :
    manager(m),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->curPathEdit->setText("/");
    ui->curPathEdit->setReadOnly(true);

    if(true)
    {
        QTreeWidget *treeWidget = ui->treeWidget;
        treeWidget->setColumnCount(1);
        treeWidget->setHeaderLabel(QString::fromLocal8Bit("文件目录"));
        treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);        // 必设置为CustomContextMenu模式，否则点击右键无反应！
        QTreeWidgetItem *root = UpdateFileTreeWidget(treeWidget, manager->root);
        if(root != nullptr)
        {
            QList<QTreeWidgetItem *> rootList;
            rootList << root;
            treeWidget->insertTopLevelItems(0, rootList);
        }
        connect(treeWidget, &QTreeWidget::itemClicked,
                this, &MainWindow::clickedInTree);
        connect(treeWidget, &QTreeWidget::customContextMenuRequested,
                this, &MainWindow::popMenu);
    }

    if(true)
    {
        QTreeWidget *folderList = ui->treeWidget_2;
        folderList->setContextMenuPolicy(Qt::CustomContextMenu);        // 必设置为CustomContextMenu模式，否则点击右键无反应！
        QStringList fileNames;
        fileNames << QString::fromLocal8Bit("名称");
        fileNames << QString::fromLocal8Bit("修改日期");
        fileNames << QString::fromLocal8Bit("类型");
        fileNames << QString::fromLocal8Bit("大小");
        folderList->setHeaderLabels(fileNames);
        folderList->setSortingEnabled(true);
        connect(folderList, &QTreeWidget::itemClicked,
                this, &MainWindow::clickedInFV);
        connect(folderList, &QTreeWidget::customContextMenuRequested,
                this, &MainWindow::popMenuInFv);
    }
}

/*
 * 更新当前路径
 */
void MainWindow::updateCurrentPath(const QList<QString>& path)
{
    cutFileItemInFV = nullptr;
    currentPath = path;
    ui->curPathEdit->setText(Utility::pathListToQString(currentPath));
}

void MainWindow::updateCurrentPath(Directory *dir)
{
    cutFileItemInFV = nullptr;
    currentPath.clear();
    FileTreeNode *f = dir;
    while(f)
    {
        currentPath.push_front(QString::fromLocal8Bit(f->getFileName().c_str()));
        f = f->getParent();
    }
    ui->curPathEdit->setText(Utility::pathListToQString(currentPath));
}

/*
 * 点击左侧的文件树
 */
void MainWindow::clickedInTree(QTreeWidgetItem *item, int index)
{
    qDebug() << "clicked in tree";
    if(isRename)
        nameChanged();
    FileTreeNode *file = getFileByItem(item);
    if(file)
    {
        updateFolderView(static_cast<Directory*>(file));
    }
    else
    {
        qDebug() << "clicked: current path:" << Utility::pathListToQString(currentPath);
        qDebug() << "clicked: can not find file " << item->text(0);
    }
}

/*
 * 点击右侧的文件夹显示列表
 */
void MainWindow::clickedInFV(QTreeWidgetItem *item, int index)
{
    qDebug() << "clicked in folder view";
    QList<QString> path = currentPath;
    path.push_back(item->text(0));
    FileTreeNode *file = findFileByPath(path);
    if(file)
    {
        qDebug() << "clickedInFV: current path:" << Utility::pathListToQString(currentPath);
        qDebug() << "clickedInFV: find file " << item->text(0);
    }
    else
    {
        qDebug() << "clickedInFV: current path:" << Utility::pathListToQString(currentPath);
        qDebug() << "clickedInFV: can not find file " << item->text(0);
    }
}

/*
 * 更新右侧的文件夹显示列表
 */
void MainWindow::updateFolderView(Directory *dir)
{
    if(dir)
    {
        // 更新当前路径
        updateCurrentPath(dir);
        // 先清空右侧文件夹显示列表
        clearFolderView();
        // 更新列表
        QTreeWidget *folderList = ui->treeWidget_2;
        QList<QTreeWidgetItem *> rootList;
        for(auto f : dir->fileList)
        {
            QStringList textList;
            textList << QString::fromLocal8Bit(f->getFileName().c_str());
            textList << QString::fromLocal8Bit(f->getModifyTime().c_str());
            if(f->getFileType() == Dir)
            {
                textList << QString::fromLocal8Bit("文件夹");
                textList << " ";
            }
            else
            {
                textList << QString::fromLocal8Bit("文本文件");
                textList << QString::number(f->getFileSize()) + QString::fromLocal8Bit("字节");
            }
            QTreeWidgetItem *file = new QTreeWidgetItem(folderList, textList);
            if(f->getFileType() == Dir)
                file->setIcon(0, QIcon(":/images/foldernew"));
            else
                file->setIcon(0, QIcon(":/images/filenew"));
            rootList << file;
        }
        folderList->insertTopLevelItems(0, rootList);
    }
}

void MainWindow::updateFolderView()
{
    QList<QString> path = currentPath;
    FileTreeNode *dir = findFileByPath(path);
    updateFolderView(static_cast<Directory*>(dir));
}

void MainWindow::addItemToFolderView(FileTreeNode *f)
{
    if(f)
    {
        QTreeWidget *tree = ui->treeWidget_2;

        QStringList textList;
        textList << QString::fromLocal8Bit(f->getFileName().c_str());
        textList << QString::fromLocal8Bit(f->getModifyTime().c_str());
        if(f->getFileType() == Dir)
        {
            textList << QString::fromLocal8Bit("文件夹");
            textList << " ";
        }
        else
        {
            textList << QString::fromLocal8Bit("文本文件");
            textList << QString::number(f->getFileSize()) + QString::fromLocal8Bit("字节");
        }
        QTreeWidgetItem *fileItem = new QTreeWidgetItem(tree, textList);
        if(f->getFileType() == Dir)
            fileItem->setIcon(0, QIcon(":/images/foldernew"));
        else
            fileItem->setIcon(0, QIcon(":/images/filenew"));

        if(tree->topLevelItemCount() == 0)
        {
            QList<QTreeWidgetItem *> rootList;
            rootList << fileItem;
            tree->insertTopLevelItems(0, rootList);
        }
        else
        {
            tree->topLevelItem(0)->addChild(fileItem);
        }

    }
}


MainWindow::~MainWindow()
{
    delete manager;
    delete ui;
}

/*
 * 更新左侧的文件树
 */
QTreeWidgetItem *MainWindow::UpdateFileTreeWidget(QTreeWidgetItem *treeWidget, FileTreeNode *root)
{
    QString name = QString::fromLocal8Bit(root->getFileName().c_str());
    if(root->getFileType() == OrdinaryFile)
        return nullptr;
    else
    {
        Directory *dir = static_cast<Directory*>(root);
        QTreeWidgetItem *newRoot = new QTreeWidgetItem(treeWidget, QStringList(name));
        newRoot->setIcon(0, QIcon(":/images/foldernew"));
        for(auto file : dir->fileList)
        {
            UpdateFileTreeWidget(newRoot, file);
        }
        return newRoot;
    }
}

QTreeWidgetItem * MainWindow::UpdateFileTreeWidget(QTreeWidget *treeWidget, FileTreeNode *root)
{
    if(root == nullptr)
        return nullptr;
    QString name = QString::fromLocal8Bit(root->getFileName().c_str());
    if(root->getFileType() == OrdinaryFile)
        return nullptr;
    else
    {
        Directory *dir = static_cast<Directory*>(root);
        QTreeWidgetItem *newRoot = new QTreeWidgetItem(treeWidget, QStringList(name));
        newRoot->setIcon(0, QIcon(":/images/foldernew"));
        for(auto file : dir->fileList)
        {
            UpdateFileTreeWidget(newRoot, file);
        }
        return newRoot;
    }
}

/*
 * 点击目录项，获取当前路径
 */
void MainWindow::getCurPath(QTreeWidgetItem *curFile , QList<QString> &path)
{
    if(curFile)
    {
        path.clear();
        path.push_front(curFile->text(0));
    }
    QTreeWidgetItem *parent = curFile->parent();
    while(parent)
    {
        path.push_front(parent->text(0));
        parent = parent->parent();
    }
}

/*
 * 找文件
 */
FileTreeNode *MainWindow::getFileByItem(QTreeWidgetItem *curItem)
{
    QList<QString> path;
    getCurPath(curItem, path);
    return findFileByPath(path);
}

FileTreeNode *MainWindow::findFileByPath(QList<QString> &path)
{
    if(path.isEmpty() || manager->root == nullptr)
        return nullptr;

    QString curName = path[0];
    path.pop_front();
    if(QString::fromLocal8Bit(manager->root->getFileName().c_str()) == curName)
    {
        if(path.isEmpty())
            return manager->root;
        else
            return findFileByPath(manager->root, path);
    }
    return nullptr;
}

FileTreeNode *MainWindow::findFileByPath(FileTreeNode *root, QList<QString> &path)
{
    if(root == nullptr || path.isEmpty())
        return nullptr;

    QString curName = path[0];
    path.pop_front();

    Directory *dir = static_cast<Directory*>(root);
    for(auto &file : dir->fileList)
    {
        if(curName == QString::fromLocal8Bit(file->getFileName().c_str()))
        {
            if(path.isEmpty())
                return file;
            else if(file->getFileType() == Dir)
            {
                return findFileByPath(file, path);
            }
            else
                return nullptr;
        }
    }
    return nullptr;
}

/*
 * 在左侧文件中找选中项对应的文件
 */
QTreeWidgetItem *MainWindow::findItemByPathInTree(QList<QString> &path)
{
    if(path.isEmpty() || ui->treeWidget->topLevelItemCount() == 0)
        return nullptr;

    QTreeWidgetItem *rootItem = ui->treeWidget->topLevelItem(0);
    QString curName = path[0];
    path.pop_front();
    if(curName == rootItem->text(0))
    {
        if(path.isEmpty())
            return rootItem;
        else
            return findItemByPathInTree(rootItem, path);
    }
    return nullptr;
}

QTreeWidgetItem *MainWindow::findItemByPathInTree(QTreeWidgetItem *root, QList<QString> &path)
{
    if(root == nullptr || path.isEmpty())
        return nullptr;

    QString curName = path[0];
    path.pop_front();
    int count = root->childCount();
    for(int i = 0; i<count; ++i)
    {
        if(root->child(i)->text(0) == curName)
        {
            if(path.isEmpty())
                return root->child(i);
            else
                return findItemByPathInTree(root->child(i), path);
        }
    }

    return nullptr;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button = QMessageBox::warning(this, QString::fromLocal8Bit("退出文件系统"),
                         QString::fromLocal8Bit("是否继续退出文件系统"), QMessageBox::Yes | QMessageBox::No);

    if(button == QMessageBox::Yes)
    {
        if(SaveFileSystem(*manager))
        {
            TraverseFileTree(manager->root);
            qDebug() << "save fs";
        }
        event->accept();
    }
    else
        event->ignore();

}

void InitDisk(int &cap, int &size)
{
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(QString::fromLocal8Bit("初始化磁盘"));
    dialog->setFixedHeight(200);
    dialog->setFixedWidth(350);

    QLabel *capLabel = new QLabel(QString::fromLocal8Bit("磁盘容量"));
    QLabel *sizeLabel = new QLabel(QString::fromLocal8Bit("盘块大小"));

    QComboBox *capCombo = new QComboBox(dialog);
    vector<int> capVec{32, 64, 128, 254};
    for(auto &cap : capVec)
    {
        capCombo->addItem(QString::number(cap)+"MB");
    }

    QComboBox *sizeCombo = new QComboBox(dialog);
    vector<int> sizeVec{512, 1024, 2048};
    for(auto &size : sizeVec)
    {
        sizeCombo->addItem(QString::number(size)+"B");
    }

    QPushButton *okButton = new QPushButton(dialog);
    okButton->setText(QString::fromLocal8Bit("确认"));
    okButton->setFixedWidth(100);
    okButton->connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(capLabel);
    hLayout->addWidget(capCombo);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(sizeLabel);
    hLayout2->addWidget(sizeCombo);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    vLayout->addLayout(hLayout2);
    vLayout->addWidget(okButton);

    dialog->setLayout(vLayout);
    dialog->exec();

    qDebug() << "init disk";
    int sizeIndex = sizeCombo->currentIndex();
    int capIndex = sizeCombo->currentIndex();
    cap = capVec[capIndex];
    size = sizeVec[sizeIndex];
    qDebug() << sizeIndex;
    qDebug() << capIndex;
}
