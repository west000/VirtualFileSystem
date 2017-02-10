#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 文本文件设置为只读状态
 */
void MainWindow::fileReadOnly()
{
    isFileReadOnly = true;
    textEdit->setReadOnly(true);
}

/*
 * 文本文件设置为读写状态
 */
void MainWindow::fileReadWrite()
{
    isFileReadOnly = false;
    textEdit->setReadOnly(false);
}

/*
 * 点击上一目录
 */
void MainWindow::on_preButton_clicked()
{
    if(currentPath.length() <= 1)
        return ;
    currentPath.pop_back();
    updateFolderView();
}

/*
 * 点击搜索
 */
void MainWindow::on_pushButton_clicked()
{
    if(ui->searchLineEidt->text().isEmpty())
        return;
    QString searchStr = ui->searchLineEidt->text();
    qDebug() << "search file " << searchStr;

    vector<string> paths;
    QList<QString> curPath = currentPath;
    FileTreeNode *root = findFileByPath(curPath);
    SearchFileByName(root, Utility::qstringtoString(searchStr), paths);

    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(QString::fromLocal8Bit("文件搜索"));
    QVBoxLayout *vLayout = new QVBoxLayout;

    if(!paths.empty())
    {
        QListWidget *listWidget = new QListWidget(dialog);
        for(const auto &f : paths)
        {
            QListWidgetItem *item = new QListWidgetItem(QIcon(), QString::fromLocal8Bit(f.c_str()));
            listWidget->addItem(item);
            qDebug() << Utility::stringToQString(f);
        }
        vLayout->addWidget(listWidget);
    }
    else
    {
        QLabel *label = new QLabel(QString::fromLocal8Bit("没有与搜索条件匹配的项"));
        label->setParent(dialog);
        label->setFixedHeight(100);
        label->setFixedWidth(200);
        vLayout->addWidget(label);
    }
    dialog->setLayout(vLayout);
    dialog->show();
}

/*
 * 关于
 */
void MainWindow::on_aboutButton_clicked()
{
    QMessageBox::about(this, QString::fromLocal8Bit("关于"),
                       QString::fromLocal8Bit("作者：黄伟塨\n\n\t2017-01-01于广东工业大学"));
}

/*
 * 帮助
 */
void MainWindow::on_helpButton_clicked()
{
    QString helpContent = QString::fromLocal8Bit("使用说明:\n");
    helpContent += QString::fromLocal8Bit("1.初始化系统时，需指定虚拟磁盘容量及盘块大小\n");
    helpContent += QString::fromLocal8Bit("2.磁盘系统的信息保存在文件DiskSetting.dat中\n");
    helpContent += QString::fromLocal8Bit("3.文件树保存在FileTree.dat中\n");
    helpContent += QString::fromLocal8Bit("4.本系统支持文件或文件夹的增删查改\n");
    helpContent += QString::fromLocal8Bit("5.左侧方框显示文件树结构，右侧方框显示当前文件夹内容\n");

    QMessageBox::about(this, QString::fromLocal8Bit("帮助"),helpContent);
}

/*
 * 磁盘信息
 */
void MainWindow::on_diskButton_clicked()
{
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(QString::fromLocal8Bit("磁盘使用情况"));
    dialog->setFixedHeight(600);
    dialog->setFixedWidth(820);

    Disk disk;
    manager->getDiskInfo(disk);

    QLabel *capLabel = new QLabel(QString::fromLocal8Bit("磁盘容量: ")+QString::number(disk.capacity)+QString::fromLocal8Bit("字节"));
    QLabel *sizeLabel = new QLabel(QString::fromLocal8Bit("盘块大小：") + QString::number(disk.block_size));
    QLabel *bnLabel = new QLabel(QString::fromLocal8Bit("盘块数量：")+QString::number(disk.block_num));
    QLabel *spaceLabel = new QLabel(QString::fromLocal8Bit("空闲盘块数：")+QString::number(disk.lave_block_num));

    QTextEdit *mapText = new QTextEdit(dialog);
    mapText->setReadOnly(true);
    QString mapStr;
    unsigned index = 0;
    for(auto i : manager->disk.blocks_lave)
    {
        if(i == disk.block_size)
            mapStr += "0 ";
        else
            mapStr += "1 ";
        ++index;
        if(index % 64 == 0)
            mapStr += "\n";
    }
    mapText->setText(mapStr);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(capLabel);
    vLayout->addWidget(sizeLabel);
    vLayout->addWidget(bnLabel);
    vLayout->addWidget(spaceLabel);
    vLayout->addWidget(mapText);
    dialog->setLayout(vLayout);
    dialog->exec();
}
