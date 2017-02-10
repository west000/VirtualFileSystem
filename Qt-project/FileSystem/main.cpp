#include "mainwindow.h"
#include "fsmanager.h"
#include "utility.h"
#include <QApplication>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <QTextCodec>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FSManager *manager = nullptr;

    if(isNeedInitFileSystem())
    {
        int cap, size;
        InitDisk(cap, size);
        Disk disk(cap*1024*1024, size);       // 磁盘分配，64MB，每个盘块1024B
        Directory *root = new Directory(Dir, "root", 0, ReadWrite, Utility::getCurTime(), Utility::getCurTime(), 10);
        manager = new FSManager(disk, root);
    }
    else
    {
        Disk disk;
        manager = new FSManager(disk);
        LoadFileSystem(*manager);
        TraverseFileTree(manager->root);
    }

    MainWindow w(manager, nullptr);
    w.show();
    return a.exec();
}
