#include "disk.h"
#include "filecontrolblock.h"
const unsigned Disk::FATITEMSIZE;
const unsigned FCB::FCBSIZE;

/*
 * 分配盘块
 */
bool Disk::allocateBlock(unsigned size, unsigned &firstBlock)
{
    firstBlock = 0;
    unsigned lastBlock = 0;
    for(decltype(blocks_lave.size()) i = 1; i<block_num; ++i)
    {
        if(blocks_lave[i] == block_size)
        {
            if(firstBlock == 0)
            {
                firstBlock = i;
            }
            if(lastBlock != 0)
            {
                fat[lastBlock] = i; // FAT中，上一块连接到当前块
            }
            // 分块
            if(size > block_size)
            {
                size -= block_size;
                blocks_lave[i] = 0;
            }
            else if(size <= block_size)
            {
                blocks_lave[i] = block_size - size;
                size = 0;
                break;
            }
            lastBlock = i;
        }
    }
    // 分配失败，磁盘空间回滚
    if(size > 0)
    {
        if(firstBlock != 0) // 不是在第一块分配失败的
        {
            unsigned index = firstBlock;
            while(fat[index])
            {
                index = fat[index];
                fat[index] = 0;
                blocks_lave[index] = block_size;
            }
        }
        return false;
    }
    return true;
}

/*
 * 回收盘块
 */
void Disk::recycleBlocks(unsigned index)
{
    blocks_content[index].clear();
    blocks_lave[index] = block_size;
    while(fat[index])
    {
        unsigned next = fat[index];
        blocks_content[next].clear();
        blocks_lave[next] = block_size;
        fat[index] = 0;
        index = next;
    }
}

/*
 * 保存文件内容到磁盘中
 */
void Disk::saveContentToDisk(unsigned index, const string &content)
{
    unsigned begin = 0;
    while(index)
    {
        blocks_content[index] = content.substr(begin, block_size - blocks_lave[index]);
        begin += block_size - blocks_lave[index];
        index = fat[index];
    }
}

/*
 * 从磁盘中读取文件
 */
void Disk::readContentFromDisk(unsigned index, string &content)
{
    content = "";
    while(index)
    {
        content += blocks_content[index];
        index = fat[index];
    }
}

/*
 * 修改磁盘中的内容，返回分配的第一盘块
 */
unsigned Disk::modifyContentInDisk(unsigned oldFirstBlock, const string &content, unsigned oldSize)
{
    unsigned newSize = content.size();
    if(newSize >= oldSize)      // 新的内容长度不小于旧的内容长度
    {
        if(oldSize == 0)        // 原来的内容为空
        {
            unsigned firstBlock = 0;
            if(allocateBlock(newSize, firstBlock))      // 分配磁盘空间
            {
                saveContentToDisk(firstBlock, content); // 保存到磁盘中
                return firstBlock;                      // 返回第一盘块号
            }
            return 0;       // 分配失败，返回0
        }

        unsigned lastDiskLave = block_size-(oldSize % block_size);   // 最后一块剩余的容量
        if(newSize - oldSize <= lastDiskLave)           // 最后一块刚好能够容纳新增的内容
        {
            unsigned index = oldFirstBlock;
            while(index)
            {
                blocks_lave[index] = 0;
                index = fat[index];
            }
            saveContentToDisk(oldFirstBlock, content);
            return oldFirstBlock;       // 返回第一盘块号
        }
        else        // 最后一块无法容纳新增的内容
        {
            unsigned index = oldFirstBlock;
            while(fat[index])
            {
                index = fat[index];
            }
            unsigned needSize = newSize - oldSize - lastDiskLave;       // 还需要的磁盘空间
            unsigned newAllocBlockIndex;
            if(allocateBlock(needSize, newAllocBlockIndex))     // 分配磁盘空间
            {
                unsigned i = oldFirstBlock;
                while(i)
                {
                    blocks_lave[i] = 0;
                    i = fat[i];
                }
                fat[index] = newAllocBlockIndex;                // 连接FAT中的索引
                saveContentToDisk(oldFirstBlock, content);      // 保存内容到磁盘中
                return oldFirstBlock;
            }
            return 0;       // 分配失败，返回0
        }
    }
    else        // 新的内容长度小于旧的内容长度
    {
        if(newSize == 0)        // 内容清空，但仍然返回第一盘块号
        {
            recycleBlocks(oldFirstBlock);
            return oldFirstBlock;
        }

        unsigned lastDiskUsed = oldSize % block_size;   // 最后一块已经使用的容量
        if(oldSize - newSize < lastDiskUsed)            // 减少的长度小于最后一个盘块的剩余容量
        {
            saveContentToDisk(oldFirstBlock, content);  // 直接进行覆盖
        }
        else        // 减少的长度不大于最后一个盘块的剩余容量
        {
            unsigned laveSize = newSize;
            unsigned index = oldFirstBlock;
            while(laveSize > 0)
            {
                if(laveSize >= block_size)
                {
                    laveSize -= block_size;
                    blocks_lave[index] = 0;
                }
                else
                {
                    blocks_lave[index] = block_size - laveSize;
                    laveSize = 0;
                }
                index = fat[index];
            }
            if(index != 0)  // 所需的盘块数比原来的少
            {
                recycleBlocks(index);
            }
            saveContentToDisk(oldFirstBlock, content);      // 覆盖原来的内容
            return oldFirstBlock;
        }
    }
}

/*
 * 将Disk转换为字符串
 */
void Disk::DiskToString(string &result)
{
    ostringstream out;
    out << capacity << " ";
    out << block_size << " ";
    out << block_num << " ";
    lave_block_num = 0;
    for(auto i : blocks_lave)
        if(i == block_size)
            ++lave_block_num;
    out << lave_block_num << "\n";

    for(auto i : fat)
        out << i << " ";
    out << endl;

    for(auto i : blocks_lave)
        out << i << " ";
    out << endl;

    for(const auto &str : blocks_content)
    {
        out << strlen(str.c_str()) << "-" << str << endl;
    }

    result = out.str();
}

/*
 * 从文件中读取Disk信息
 */
bool Disk::GetDiskFromFile(Disk &disk, const string &file)
{
    ifstream fin(file);
    if(fin.eof() || fin.fail())
        return false;

    unsigned capacity, block_size, block_num, lave_block_num;
    fin >> capacity >> block_size >> block_num >> lave_block_num;
    disk.capacity = capacity;
    disk.block_size = block_size;
    disk.block_num = block_num;
    disk.lave_block_num = lave_block_num;

    cout << "capacity: " << disk.capacity << endl;
    cout << "block_size: " << disk.block_size << endl;
    cout << "block_num: " << disk.block_num << endl;
    cout << "lave_block_num: " << disk.lave_block_num << endl;


    // 加载FAT
    vector<unsigned> fat(block_num, 0);
    for(unsigned i = 0; i<block_num; ++i)
    {
        fin >> fat[i];
    }
    disk.fat = fat;

    // 加载每一块的剩余容量信息
    vector<unsigned> blocks_lave(block_num, 0);
    for(unsigned i = 0; i<block_num; ++i)
    {
        fin >> blocks_lave[i];
    }
    disk.blocks_lave = blocks_lave;

    // 加载每一块的内容
    disk.blocks_content = vector<string>(block_num, "");
    unsigned size;
    char *buf = new char[block_size + 2];
    for(unsigned i = 0; i<block_num; ++i)
    {

        char ch;
        fin >> size >> ch;
        fin.read(buf, size);
        buf[size] = '\0';
        disk.blocks_content[i] = string(buf);
    }
    delete buf;
    fin.close();
    return true;
}
