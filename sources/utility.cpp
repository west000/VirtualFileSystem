#include "utility.h"

string Utility::getCurTime()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss");
    QByteArray bytes = current_date.toLatin1();
    char *pDate = bytes.data();
    return string(pDate);
}

QString Utility::pathListToQString(const QList<QString> &list)
{
    QString result;
    int size = list.length();
    result = QString::fromLocal8Bit("/");
    for(int i = 0; i < size; ++i)
    {
        result += list[i] + QString::fromLocal8Bit("/");
    }
    return result;
}
