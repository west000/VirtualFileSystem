#include "fsmanager.h"

void FSManager::AddOneFileItemToDisk(FileTreeNode *f, Directory &dst)
{
    // 一个盘块能够包含的FCB数目
    const unsigned ContainFCBMaxNum = disk.block_size / FCB::FCBSIZE;

    unsigned fileNum = dst.fileList.size();
    if(fileNum)
    {
        unsigned index = dst.dir.first_block;
        while(disk.fat[index] != 0)
        {
            index = disk.fat[index];
        }
        if((fileNum % ContainFCBMaxNum) == 0)
        {
            unsigned newBlock;
            if(disk.allocateBlock(disk.block_size, newBlock))
            {
                disk.fat[index] = newBlock;
                disk.fat[newBlock] = 0;
                disk.blocks_lave[newBlock] -= FCB::FCBSIZE;
                disk.writeContentToDisk(newBlock, f->getFileName());
            }
        }
        else
        {
            if(disk.blocks_lave[index] >= FCB::FCBSIZE)
                disk.blocks_lave[index] -= FCB::FCBSIZE;
            disk.writeContentToDisk(index, f->getFileName());
        }
    }
    else        // 当前目录的第一项
    {
        unsigned newBlock;
        if(disk.allocateBlock(disk.block_size, newBlock))
        {
            cout << "new block:" << newBlock << endl;
            cout << "file first block" << f->getFirstBlock() << endl;
            dst.dir.first_block = newBlock;
            disk.blocks_lave[newBlock] -= FCB::FCBSIZE;
            disk.writeContentToDisk(newBlock, f->getFileName());
        }
    }
}
