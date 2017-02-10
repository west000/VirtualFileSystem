#ifndef FSMANAGER_H
#define FSMANAGER_H

#include "disk.h"
#include "utility.h"
#include "filecontrolblock.h"
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <stack>

#include <QTime>
#include <QDateTime>

using namespace std;

// 树形结构
class Directory;
struct FileTreeNode
{
    virtual FileType getFileType() = 0;
    virtual const string &getFileName() = 0;
    virtual void setFileName(const string &name) = 0;
    virtual void setModifyTime(const string &t) = 0;
    virtual string getModifyTime() = 0;
    virtual string getCreateTime() = 0;
    virtual void setParent(Directory *p) = 0;
    virtual Directory *getParent() = 0;
    virtual unsigned getFileSize() const = 0;
    virtual unsigned getFirstBlock() = 0;
    virtual char getFilePermission() = 0;
    virtual void setFilePermission(char rw) = 0;
};


class File : public FileTreeNode
{
public:
    File(FileType ft, const string &name, unsigned size, char rw, const string &ctime, const string &mtime, unsigned block):
        file(name, ctime, mtime, block, ft,rw, size), parent(nullptr)
    {}

    File(const FCB &fcb, Directory *p = nullptr): file(fcb), parent(p)
    {}

    FileType getFileType()
    {
        return file.type;
    }

    const string &getFileName()
    {
        return file.filename;
    }

    void setFileName(const string &name)
    {
        file.filename = name;
    }

    void setModifyTime(const string &t)
    {
        file.modify_time = t;
    }

    string getModifyTime()
    {
        return file.modify_time;
    }

    string getCreateTime()
    {
        return file.creat_time;
    }

    void setParent(Directory *p)
    {
        parent = p;
    }

    Directory *getParent()
    {
        return parent;
    }

    unsigned getFileSize() const
    {
        return file.size;
    }

    void setFileSize(unsigned size)
    {
        file.size = size;
    }

    unsigned getFirstBlock()
    {
        return file.first_block;
    }

    void setFirstBlock(unsigned block)
    {
        file.first_block = block;
    }

    char getFilePermission()
    {
        return file.readwrite;
    }

    void setFilePermission(char rw)
    {
        file.readwrite = rw;
    }

    ~File()
    {}

public:
    FCB file;
    Directory *parent;
};

class Directory : public FileTreeNode
{
public:
    Directory(FileType ft, const string &name, unsigned size, char rw, const string &ctime, const string &mtime, unsigned block):
        dir(name, ctime, mtime, block, ft,rw, size), parent(nullptr)
    {}

    Directory(const FCB& fcb, Directory *p = nullptr): dir(fcb), parent(p)
    {}

    FileType getFileType()
    {
        return dir.type;
    }

    const string &getFileName()
    {
        return dir.filename;
    }

    void setFileName(const string &name)
    {
        dir.filename = name;
    }

    void setModifyTime(const string &t)
    {
        dir.modify_time = t;
    }

    string getModifyTime()
    {
        return dir.modify_time;
    }

    string getCreateTime()
    {
        return dir.creat_time;
    }

    void setParent(Directory *p)
    {
        parent = p;
    }

    void addFile(File *f)
    {
        f->setParent(this);
        fileList.push_back(f);
    }

    void addFile(Directory *&d)
    {
        d->setParent(this);
        fileList.push_back(d);
    }

    Directory *getParent()
    {
        return parent;
    }

    // 需要进行遍历，获取到目录下所有文件的大小！
    unsigned getFileSize() const;

    unsigned getFirstBlock()
    {
        return dir.first_block;
    }

    char getFilePermission()
    {
        return dir.readwrite;
    }

    void setFilePermission(char rw)
    {
        dir.readwrite = rw;
    }

    ~Directory()
    {
        for(auto &file : fileList)
        {
            delete file;
        }
        fileList.clear();
    }

public:
    FCB dir;
    Directory *parent;
    list<FileTreeNode*> fileList;
};


/*
* 描述：文件系统管理
* 作者：黄伟塨
* 时间：2016/12/19
*/
class FSManager
{
public:
    FSManager(const Disk &disk, FileTreeNode *node = nullptr):
        disk(disk), root(node)
    {}

    bool AddFile(Directory *dst, bool &hasSameFile, const string &filename, char rw = ReadWrite, const string &content = "",  FileTreeNode **result = nullptr);
    bool AddFolder(Directory *dst, bool &hasSameFolder, const string &name, char rw = ReadWrite, Directory **result = nullptr);
    bool DeleteFile(FileTreeNode *delNode);
    void RenameFile(FileTreeNode &file, const string &filename);
    void ModifyFilePermission(FileTreeNode *file, char rw);
    bool ModifyFileContent(File *file, const string &newName, const string &content);
    bool CutAndPaste(Directory *dst, FileTreeNode *file);
    bool CopyAndPaste(Directory *dst, FileTreeNode *file, bool &hasCrash);

    void readContentFromDisk(unsigned index, string &content)
    {
        disk.readContentFromDisk(index, content);
    }

    void getDiskInfo(Disk &d)
    {
        d.capacity = disk.capacity;
        d.block_size = disk.block_size;
        d.block_num = disk.block_num;
        d.lave_block_num = disk.lave_block_num;
    }

    bool SaveFileTree();
    bool LoadFileTree();
    bool SaveDiskInfo();
    bool LoadDiskInfo();

private:
    void AddOneFileItemToDisk(FileTreeNode *f, Directory &dst);

public:
    Disk disk;                      // 磁盘信息
    FileTreeNode *root;             // 文件树
    //stack<FileTreeNode> curPath;    // 当前路径
};

// 遍历文件树
void TraverseFileTree(FileTreeNode *root);
// 寻找文件
void SearchFileByName(FileTreeNode *root, const string &filename, vector<string>& path);
// 在当前目录下寻找文件
FileTreeNode *SearchFileInCurDir(Directory *dir, const string & filename);

void FileTreeToString(FileTreeNode *root, vector<string> &result);

bool SaveFileSystem(FSManager &manager);
bool LoadFileSystem(FSManager &manager);
bool isNeedInitFileSystem();

#endif // FSMANAGER_H
