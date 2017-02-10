#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

struct User
{
    User(const string &name, const string& psw, const string &ctime, const string &mtime):
        name(name), password(psw), creat_time(ctime), modify_time(mtime)
    {}

    string name;                // 用户名
    string password;            // 密码
    string creat_time;          // 用户创建时间
    string modify_time;         // 修改时间
};

#endif // USER_H
