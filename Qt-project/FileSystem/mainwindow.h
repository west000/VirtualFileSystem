#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fsmanager.h"
#include <QMainWindow>
#include <QTextBrowser>
#include <QWindow>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileSystemModel>
#include <QList>
#include <QString>
#include <QGridLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <vector>
#include <QDebug>
#include <QComboBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(FSManager *m, QWidget *parent = 0);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetFileSystem(FSManager *m)
    {
        manager = m;
    }

    QTreeWidgetItem *UpdateFileTreeWidget(QTreeWidgetItem *treeWidget, FileTreeNode *root);
    QTreeWidgetItem *UpdateFileTreeWidget(QTreeWidget *treeWidget, FileTreeNode *root);
public:
    FSManager *manager;
    void getCurPath(QTreeWidgetItem *curFile , QList<QString> &path);
    FileTreeNode *getFileByItem(QTreeWidgetItem *curItem);
    FileTreeNode *findFileByPath(QList<QString> &path);
    FileTreeNode *findFileByPath(FileTreeNode *root, QList<QString> &path);
    QTreeWidgetItem *findItemByPathInTree(QList<QString> &path);
    QTreeWidgetItem *findItemByPathInTree(QTreeWidgetItem *root, QList<QString> &path);

    void closeEvent(QCloseEvent *event);

private slots:
    void clickedInTree(QTreeWidgetItem *item, int index);
    void clickedInFV(QTreeWidgetItem *item, int index);
    void updateFolderView(Directory *dir);
    void updateFolderView();
    void addItemToFolderView(FileTreeNode *file);
    void clearFolderView();

    // 文件树形目录
    void popMenu(const QPoint& pos);
    void popProperty();
    void newItem();
    void deleteItem();
    void copyItem();
    void cutItem();
    void pasteItem();
    void renameItem();
    void nameChanged();

    // 文件夹显示列表, FV为FolderListView
    void popMenuInFv(const QPoint &);
    void popPropertyInFV();
    void newFolderInFV();
    void newFileInFV();
    void openInFV();
    void openFileInFV(File *file);
    void copyInFV();
    void cutInFV();
    void pasteInFV();
    void deleteInFV();
    void renameInFV();

    // 文件内容修改
    void fileReadOnly();
    void fileReadWrite();

    // ui
    void on_preButton_clicked();
    void on_pushButton_clicked();
    void on_aboutButton_clicked();
    void on_helpButton_clicked();
    void on_diskButton_clicked();

private:
    void showPropertyWidget(FileTreeNode *file, const QList<QString> &path);
    void updateCurrentPath(const QList<QString>& path);
    void updateCurrentPath(Directory *dir);

private:
    Ui::MainWindow *ui;
    QTreeWidget *treeWidget;
    QList<QString> curPath;

    bool isCopy = false;
    bool isCut = false;
    bool isRename = false;
    bool isFileNameModify = false;
    bool isFileContentModify = false;
    bool isFileReadOnly = false;

    bool isCopyInFV = false;
    bool isCutInFV = false;
    bool isRenameInFV = false;
    bool isNoItemSelect = true;
    QTextEdit *textEdit;

    FileTreeNode *copyFileNode;
    FileTreeNode *cutFileNode;
    FileTreeNode *renameFileNode;
    QTreeWidgetItem *cutFileItem;
    QTreeWidgetItem *cutFileItemInFV;
    QTreeWidgetItem *renameFileItem;

    QTreeWidget *curTree;
    QList<QString> currentPath;    
};

void InitDisk(int &cap, int &size);

#endif // MAINWINDOW_H
