#ifndef FILECONTROLBLOCK_H
#define FILECONTROLBLOCK_H

#include <string>
#include <vector>
#include <cstring>
#include <sstream>

using namespace std;

// 文件访问权限
enum AccessPremission
{
    ReadOnly = 0x1,
    ReadWrite = 0x2
};

// 文件类型
enum FileType
{
    Dir,                // 目录文件
    OrdinaryFile        // 普通文件
};


class FCB                  // 文件控制块，在磁盘中占用128字节
{
public:
    FCB() = default;

    FCB(const string &name, const string &ctime, const string &mtime,
        unsigned block, FileType ft, char rw = ReadWrite, unsigned s = 0):
        filename(name), size(s), readwrite(rw), type(ft),
        creat_time(ctime), modify_time(mtime), first_block(block)
    {}

    FCB(const FCB &f):
        filename(f.filename), size(f.size), readwrite(f.readwrite),
        type(f.type), creat_time(f.creat_time), modify_time(f.modify_time), first_block(f.first_block)
    {}

    void FCBToString(string &result)
    {
        ostringstream out;
        out << filename.size() << "-" << filename << "\t";
        out << size << "\t";
        out << readwrite << "\t";
        out << type << "\t";
        out << creat_time.size() << "-" << creat_time << "\t";
        out << modify_time.size() << "-" << modify_time << "\t";
        out << first_block << "\t";
        result = out.str();
    }

    static void StringToFCB(const string &str, FCB &fcb, unsigned &depth)
    {
        istringstream fin(str);
        if(fin.fail())
            return;

        string depthStr;
        fin >> depthStr;
        depth = depthStr.size();        // 深度

        unsigned fileLength;
        fin >> fileLength;
        char ch;
        fin >> ch;
        char *buf = new char[256];
        fin.read(buf, fileLength);
        buf[fileLength] = '\0';
        fcb.filename = string(buf);     // 文件名

        unsigned size;
        fin >> size;
        fcb.size = size;                // 文件大小

        char rw;
        fin >> rw;
        fcb.readwrite = rw;             // 读写权限

        int type;
        fin >> type;
        fcb.type = static_cast<FileType>(type); // 文件类型

        unsigned timeLength;
        fin >> timeLength;
        fin >> ch;
        fin.read(buf, timeLength);
        buf[timeLength] = '\0';
        fcb.creat_time = string(buf);   // 创建时间

        fin >> timeLength;
        fin >> ch;
        fin.read(buf, timeLength);
        buf[timeLength] = '\0';
        fcb.modify_time = string(buf);  // 修改时间

        unsigned firstBlock;
        fin >> firstBlock;
        fcb.first_block = firstBlock;   // 第一盘块号

        delete buf;

//        cout << fcb.filename << endl;
//        cout << fcb.size << endl;
//        cout << fcb.readwrite << endl;
//        cout << fcb.type << endl;
//        cout << fcb.creat_time << endl;
//        cout << fcb.modify_time << endl;
//        cout << fcb.first_block << endl;
    }

public:
    string filename;        // 文件名（包括后缀名），最大长度为64字节
    unsigned size;          // 文件大小
    char readwrite;         // 读写权限
    FileType type;          // 文件类型
    string creat_time;      // 创建时间
    string modify_time;     // 修改时间
    unsigned first_block;   // 第一块号

    static const unsigned FCBSIZE = 128;
};

#endif // FILECONTROLBLOCK_H
