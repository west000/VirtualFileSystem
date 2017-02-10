#ifndef DISK_H
#define DISK_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

class Disk
{
public:
    Disk() : capacity(0), block_size(0),block_num(0)
    {}

    Disk(unsigned cap, unsigned bsize):
        capacity(cap), block_size(bsize), block_num(cap/bsize),
        fat(cap/bsize, 0), blocks_lave(cap/bsize, bsize), blocks_content(cap/bsize, "")
    {
        // 为FAT分配连续的磁盘空间
        unsigned needBlockNum = block_num / (block_size / FATITEMSIZE) + 1;
        cout << "need block num:" << needBlockNum << endl;
        for(int i = 1; i<needBlockNum; ++i)
        {
            fat[i] = i + 1;
            blocks_lave[i] = 0;
        }
        blocks_lave[needBlockNum] = 0;
        fat[needBlockNum] = 0;
    }

    bool isBlockAllocated(unsigned id) const
    {
        if(id >= block_num)
            return true;
        if(blocks_lave[id] == block_size)
            return true;
        return false;
    }

    bool allocateBlock(unsigned size, unsigned &firstBlock);
    void recycleBlocks(unsigned index);

    void writeContentToDisk(unsigned index, const string &str)
    {
        blocks_content[index] += "\n---" + str + "---\n";
    }

    void saveContentToDisk(unsigned index, const string &content);
    void readContentFromDisk(unsigned index, string &content);
    unsigned modifyContentInDisk(unsigned index, const string &content, unsigned oldSize);

    // 将Disk转换为字符串
    void DiskToString(string &result);
    // 从文件中读取Disk信息
    static bool GetDiskFromFile(Disk &disk, const string &file);

public:
    unsigned capacity;        // 磁盘容量，单位字节
    unsigned block_size;      // 盘块大小，单位字节
    unsigned block_num;       // 盘块数量
    unsigned lave_block_num;        // 空闲盘块的数量
    vector<unsigned> fat;           // 文件分配表
    vector<unsigned> blocks_lave;   // 每一盘块的剩余容量
    vector<string> blocks_content;  // 每一块对应的内容

    static const unsigned FATITEMSIZE = 32;
};

#endif // DISK_H
