#ifndef UTILITY_H
#define UTILITY_H

#include <QTime>
#include <QDateTime>
#include <string>
#include <QString>
#include <QList>
#include <QDebug>

using namespace std;

class Utility
{
public:
    static string getCurTime();
    static QString stringToQString(const string &str)
    {
        return QString::fromLocal8Bit(str.c_str());
    }

    static string qstringtoString(const QString &str)
    {
        QByteArray cstr = str.toLocal8Bit();
        char *p = cstr.data();
        return string(p);
    }

    static QString pathListToQString(const QList<QString> &list);
};

#endif // UTILITY_H
