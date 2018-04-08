#ifndef QTUTILITIES_H
#define QTUTILITIES_H

#include <QSharedPointer>
#include <QException>
#include <QPoint>

namespace Utilities { namespace Qt {

template<class T>
QSharedPointer<T> makeShared()
{
  return QSharedPointer<T>(new T);
}

template<class T, class... Args>
QSharedPointer<T> makeShared(Args&&... args)
{
  return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

}}

#endif // QTUTILITIES_H
