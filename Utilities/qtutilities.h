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

template<typename T>
void hashCombine(uint& seed, T&& value)
{
  seed ^= qHash(std::forward<T>(value)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T, typename ...Args>
void hashCombine(uint& seed, T&& value, Args&& ...args)
{
  hashCombine(seed, std::forward<T>(value));
  hashCombine(seed, std::forward<Args>(args)...);
}

}} // Utilities::Qt

inline uint qHash(QPoint const& point, uint seed)
{
  return qHash(qMakePair(point.x(), point.y()), seed);
}

#endif // QTUTILITIES_H
