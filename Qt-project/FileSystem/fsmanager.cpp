#include "fsmanager.h"
#include <iostream>
#include <fstream>
#include <sstream>

// 需要进行遍历，获取到目录下所有文件的大小！
unsigned Directory::getFileSize() const
{
    unsigned size = 0;
    for(const auto file : fileList)
    {
        size += file->getFileSize();
    }
    return size;
}

/*
 * 从一个结点开始，遍历文件树
 */
void TraverseFileTree(FileTreeNode *root, unsigned depth)
{
    if(!root)
        return;

    string str(depth, '-');
    if(root->getFileType() == OrdinaryFile)
    {
        File *file = static_cast<File*>(root);
        cout << str << " " << file->file.filename << endl;
        return ;
    }

    Directory *dir = static_cast<Directory*>(root);
    cout << str << " " << dir->dir.filename << endl;
    for(auto iter = dir->fileList.begin(); iter != dir->fileList.end(); ++iter)
    {
        TraverseFileTree(*iter, depth+1);
    }
}

void TraverseFileTree(FileTreeNode *root)
{
    TraverseFileTree(root, 1);
}

// 将路径转换为字符串
void PathVecToString(const vector<string>& path, string &pathStr)
{
    for(const auto &p : path)
    {
        pathStr += "/" + p;
    }
}

/*
 * 寻找文件
 * 从当前结点开始寻找文件
 */
void SearchFileByName(Directory *root, const string &filename, vector<string>& pathStrVec, vector<string> &curPath)
{
    if(!root)
        return;

    curPath.push_back(root->dir.filename);
    for(auto iter = root->fileList.begin(); iter != root->fileList.end(); ++iter)
    {
        if((*iter)->getFileName() == filename)
        {
            string path;
            PathVecToString(curPath, path);
            path += "/" + filename;
            pathStrVec.push_back(path);
        }
        if((*iter)->getFileType() == Dir)
        {
            SearchFileByName(static_cast<Directory*>(*iter), filename, pathStrVec, curPath);
            curPath.pop_back();
        }
    }
}

void SearchFileByName(FileTreeNode *root, const string &filename, vector<string>& path)
{
    vector<string> curPath;
    SearchFileByName(static_cast<Directory*>(root), filename, path, curPath);
}

/*
 * 寻找文件
 * 在当前目录下寻找文件或目录
 */
FileTreeNode *SearchFileInCurDir(Directory *dir, const string & filename)
{
    if(!dir)
        return nullptr;

    for(auto iter = dir->fileList.begin(); iter != dir->fileList.end(); ++iter)
    {
        if((*iter)->getFileName() == filename)
        {
            return static_cast<FileTreeNode*>(*iter);
        }
    }
    return nullptr;
}

/*
 * 删除文件: 文件或者目录
 */
bool FSManager::DeleteFile(FileTreeNode *delNode)
{
    if(!delNode)
        return false;

    Directory *parent = delNode->getParent();   // 结点的父目录
    if(delNode->getFileType() == Dir)           // 如果删除的是目录，需要先递归删除目录下的所有文件！
    {
        Directory *dir = static_cast<Directory*>(delNode);
        for(list<FileTreeNode*>::iterator iter = dir->fileList.begin();
            iter != dir->fileList.end(); )
        {
            // cout << "delete" << (*iter)->getFileName() << endl;
            if((*iter)->getFileType() == OrdinaryFile)
            {
                disk.recycleBlocks((*iter)->getFirstBlock());   // 回收普通文件占用的空间
                delete (*iter);
                ++iter;
            }
            else
            {
                disk.recycleBlocks((*iter)->getFirstBlock());
                auto del = iter;
                ++iter;     // 在调用DeleteFile的过程中，iter会失效，故用临时变量del
                DeleteFile(static_cast<FileTreeNode*>(*del));
            }
        }
        dir->fileList.clear();
    }

    // 回收磁盘空间
    disk.recycleBlocks(delNode->getFirstBlock());
    delete delNode;     // 正式删除结点
    if(parent)          // 对应的目录项也删除！
    {
        // 删除对应的目录项
        for(list<FileTreeNode*>::iterator iter = parent->fileList.begin();
            iter != parent->fileList.end(); ++iter)
        {
            if((*iter) == delNode)
            {                
                delNode = nullptr;
                iter = parent->fileList.erase(iter);    // 删除之后不要忘了连接起来！
                break;
            }
        }
    }

    return true;
}

/*
 * 修改文件:包括文件名、文件内容、文件属性
 */
// 重命名文件或文件夹
void FSManager::RenameFile(FileTreeNode &file, const string &filename)
{
    file.setFileName(filename);
    string curTime = Utility::getCurTime();
    file.setModifyTime(curTime);
}

// 修改文件内容
bool FSManager::ModifyFileContent(File *file, const string &newName, const string &content)
{
    unsigned newFirstBlock = disk.modifyContentInDisk(file->getFirstBlock(), content, file->getFileSize());
    if(newFirstBlock == 0)
    {
        return false;
    }
    else
    {
        file->setFileName(newName);
        file->setFirstBlock(newFirstBlock);
        file->setFileSize(content.size());
        file->setModifyTime(Utility::getCurTime());
        return true;
    }
}

// 修改访问权限
void FSManager::ModifyFilePermission(FileTreeNode *file, char rw)
{
    file->setFilePermission(rw);
    file->setModifyTime(Utility::getCurTime());
}

/*
 * 添加文件
 */
bool FSManager::AddFile(Directory *dst, bool &hasSameFile, const string &filename, char rw, const string &content, FileTreeNode **result)
{
    hasSameFile = false;
    FileTreeNode *pFile = SearchFileInCurDir(dst, filename);
    if(pFile != nullptr)
    {
        if(pFile->getFileType() == OrdinaryFile)
        {
            hasSameFile = true;
            return false;
        }
    }

    unsigned firstBlock;
    unsigned fileSize = content.size();
    if(fileSize == 0)
        fileSize = 1;
    if(disk.allocateBlock(fileSize, firstBlock))
    {
        // 成功分配
        string curTime = Utility::getCurTime();
        File *file = new File(OrdinaryFile, filename, content.size(), rw, curTime, curTime, firstBlock);
        // 将文件内容保存到磁盘中
        disk.saveContentToDisk(firstBlock, content);
        // 将目录项添加到磁盘文件中
        AddOneFileItemToDisk(file, *dst);
        // 将文件项添加到内存中的文件树中
        dst->addFile(file);
        // 更新目录修改时间
        dst->dir.modify_time = curTime;

        if(result != nullptr)
            *result = file;

        return true;
    }
    return false;
}

/*
 * 添加文件夹
 */
bool FSManager::AddFolder(Directory *dst, bool &hasSameFolder, const string &name, char rw, Directory **result)
{
    hasSameFolder = false;
    FileTreeNode *pFile = SearchFileInCurDir(dst, name);
    if(pFile != nullptr)
    {
        if(pFile->getFileType() == Dir)
        {
            hasSameFolder = true;
            return false;
        }
    }

    string curTime = Utility::getCurTime();
    Directory *dir = new Directory(Dir, name, 0, rw, curTime, curTime, 0);
    if(result)
        *result = dir;
    // 将目录项添加到磁盘文件中
    AddOneFileItemToDisk(dir, *dst);
    // 将文件夹项添加到内存中的文件树中
    dst->addFile(dir);
    // 更新目录修改时间
    dst->dir.modify_time = curTime;
    return true;
}

/*
 * 剪切后粘贴文件
 * 将文件对应的目录项从目录文件中删除，再将目录项存放到新的目录文件中
 * 返回值：false表示目标目录中有同名同类型文件
 */
bool FSManager::CutAndPaste(Directory *dst, FileTreeNode *file)
{
    string name = file->getFileName();
    FileTreeNode *pFile = SearchFileInCurDir(dst, name);
    if(pFile != nullptr)
    {
        // 同类型、同名，粘贴失败
        if(pFile->getFileType() == file->getFileType())
        {
            return false;
        }
    }

    // 将原来父目录文件中对应的目录项删除
    Directory *oldParent = file->getParent();
    if(oldParent)
    {
        for(list<FileTreeNode*>::iterator iter = oldParent->fileList.begin();
            iter != oldParent->fileList.end(); ++iter)
        {
            if((*iter) == file)
            {
                iter = oldParent->fileList.erase(iter);    // 删除目录项
                break;
            }
        }
    }
    // 修改文件的父目录
    file->setParent(dst);
    // 将文件目录项添加到目录文件对应的磁盘空间中
    AddOneFileItemToDisk(file, *dst);
    // 将目录项添加到内存中的文件树中
    dst->addFile(static_cast<File*>(file));
    // 更新目录文件的修改时间
    dst->dir.modify_time = Utility::getCurTime();
    return true;
}

/*
 * 复制并粘贴文件
 * hasCrash表示目的文件夹中有同名同类型文件
 * 返回值：false表示磁盘空间不足
 */
bool FSManager::CopyAndPaste(Directory *dst, FileTreeNode *file, bool &hasCrash)
{
    hasCrash = false;       // 同名同类型文件的冲突的标志
    string name = file->getFileName();
    FileTreeNode *pFile = SearchFileInCurDir(dst, name);
    if(pFile != nullptr)
    {
        // 同类型、同名，粘贴失败
        if(pFile->getFileType() == file->getFileType())
        {
            hasCrash = true;
            return false;
        }
    }

    if(file->getFileType() == OrdinaryFile)
    {
        bool hasSameFile;
        string content;
        disk.readContentFromDisk(file->getFirstBlock(), content);       // 从磁盘中读文件内容
        return AddFile(dst, hasSameFile, file->getFileName(), file->getFilePermission(), content);
    }
    else if(file->getFileType() == Dir)
    {
        // 添加目录
        Directory *newDir = nullptr;
        bool hasSameFolder;
        AddFolder(dst, hasSameFolder, file->getFileName(), file->getFilePermission(), &newDir);

        // 向新目录添加文件
        Directory *dir = static_cast<Directory*>(file);
        for(list<FileTreeNode*>::iterator iter = dir->fileList.begin();
            iter != dir->fileList.end(); ++iter)
        {
            bool hasCrash;
            if(!CopyAndPaste(newDir, *iter, hasCrash))
                return false;
        }
        return true;
    }
}

// 遍历文件树，将文件树转换为字符串，每个文件一行
void FileTreeToString(FileTreeNode *root, unsigned depth,vector<string> &result)
{
    if(!root)
        return;
    // 将文件加入
    if(root->getFileType() == OrdinaryFile)
    {
        File *file = static_cast<File*>(root);
        string fileStr;
        file->file.FCBToString(fileStr);
        string str(depth, '-');
        str += " " + fileStr + "\n";
        result.push_back(str);
        return;
    }

    // 将该目录加入
    Directory *dir = static_cast<Directory*>(root);
    string dirStr;
    dir->dir.FCBToString(dirStr);
    string str(depth, '-');
    str += " " + dirStr + "\n";
    result.push_back(str);
    // 将目录下的文件加入
    for(auto iter = dir->fileList.begin(); iter != dir->fileList.end(); ++iter)
    {
        FileTreeToString(*iter, depth+1, result);
    }
}

void FileTreeToString(FileTreeNode *root, vector<string> &result)
{
    FileTreeToString(root, 1, result);
}

// 保存文件树
bool FSManager::SaveFileTree()
{
    // 保存文件树
    ofstream fout("FileTree.dat");
    if(fout.fail())
        return false;

    vector<string> result;
    FileTreeToString(root, result);
    for(const auto &str : result)
    {
        fout.write(str.c_str(), sizeof(char)*str.size());
    }
    fout.close();

    return true;
}

// 保存磁盘信息
bool FSManager::SaveDiskInfo()
{
    // 保存Disk的配置信息
    ofstream fout("DiskSetting.dat");
    if(fout.fail())
    {
        return false;
    }

    string diskStr;
    disk.DiskToString(diskStr);
    fout.write(diskStr.c_str(), sizeof(char)*diskStr.size());
    fout.close();

    return true;
}

// 保存文件系统到txt中
bool SaveFileSystem(FSManager &manager)
{
    return manager.SaveDiskInfo() && manager.SaveFileTree();
}

void GenerateFileTree(FileTreeNode *root, list<string> &treeList, unsigned curDepth)
{
    if(treeList.empty())
        return ;

    while(!treeList.empty())
    {
        string treeStr = treeList.front();
        FCB fcb;
        unsigned depth;
        FCB::StringToFCB(treeStr, fcb, depth);

        if(depth > curDepth && depth - curDepth == 1)
        {
            treeList.pop_front();

            if(fcb.type == OrdinaryFile)
            {
                File *file = new File(fcb, static_cast<Directory*>(root));
                Directory *pRoot = static_cast<Directory*>(root);
                pRoot->fileList.push_back(file);
            }
            else if(fcb.type == Dir)
            {
                Directory *dir = new Directory(fcb, static_cast<Directory*>(root));
                Directory *pRoot = static_cast<Directory*>(root);
                pRoot->fileList.push_back(dir);
                GenerateFileTree(dir, treeList, depth);
            }
        }
        else
            return;
    }
}

// 加载文件树
bool FSManager::LoadFileTree()
{
    ifstream fin("FileTree.dat");
    if(fin.fail())
        return false;

    list<string> treeStrList;
    string buf;
    while(getline(fin, buf))
    {
        treeStrList.push_back(buf);
        cout << buf << endl;
    }

    if(!treeStrList.empty())
    {
        string rootStr = treeStrList.front();
        treeStrList.pop_front();

        FCB rootFCB;
        unsigned depth;
        FCB::StringToFCB(rootStr, rootFCB, depth);
        root = new Directory(rootFCB);
        GenerateFileTree(root, treeStrList, 1);
    }
    else
    {
        string curTime = Utility::getCurTime();
        unsigned firstBlock;
        disk.allocateBlock(FCB::FCBSIZE, firstBlock);
        FCB rootFCB("root", curTime, curTime, 0, Dir, ReadWrite, firstBlock);    // 文件目录
        root = new Directory(rootFCB);
    }

    return true;
}

// 加载磁盘信息
bool FSManager::LoadDiskInfo()
{
    return Disk::GetDiskFromFile(disk, "DiskSetting.dat");
}

// 从txt加载文件系统
bool LoadFileSystem(FSManager &manager)
{
    return manager.LoadDiskInfo() && manager.LoadFileTree();
}

// 判断是否要初始化系统
bool isNeedInitFileSystem()
{
    bool isNeedInit = false;
    fstream diskfile;
    diskfile.open("DiskSetting.dat", ios::in);
    fstream ftfile;
    ftfile.open("FileTree.dat", ios::in);
    if(diskfile && ftfile)
    {
        return false;
    }
    return true;
}




