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

template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
bool isPowerOfTwo(T value)
{
  return value > 0 && ((value & (value - 1)) == 0);
}

template<typename T>
qreal normalized(T value, T min, T max)
{
  Q_ASSERT(min < max);
  Q_ASSERT(value >= min);
  Q_ASSERT(value <= max);
  auto const result = 1.0 - (max - value) / static_cast<qreal>(max - min);
  Q_ASSERT(result >= 0.0);
  Q_ASSERT(result <= 1.0);
  return result;
}

template<typename T>
T clamp(T value, T min, T max)
{
  Q_ASSERT(min < max);
  if (value < min)
  {
    return min;
  }
  if (value > max)
  {
    return max;
  }
  return value;
}

class ThreadChecker
{
public:
  explicit ThreadChecker(QThread* thread = nullptr);
  void check() const;
  void check();

private:
  QThread* thread_ = nullptr;
};

}} // Utilities::Qt

inline uint qHash(QPoint const& point, uint seed)
{
  return qHash(qMakePair(point.x(), point.y()), seed);
}

#endif // QTUTILITIES_H
