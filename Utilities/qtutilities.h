#ifndef QTUTILITIES_H
#define QTUTILITIES_H

#include <QSharedPointer>
#include <QException>

namespace Utilities { namespace Qt {

template<class T>
class NotNullPtr
{
public:
  T const& get() const
  {
    Q_ASSERT(ptr_ != nullptr);
    return *ptr_;
  }
  T& getMutable()
  {
    Q_ASSERT(ptr_ != nullptr);
    return *ptr_;
  }
  void set(T* ptr)
  {
    ptr_ = ptr;
    Q_ASSERT(ptr_ != nullptr);
  }

private:
  T* ptr_ = nullptr;
};

template<class T, class... Args>
QSharedPointer<T> makeShared(Args&&... args)
{
  return QSharedPointer<T>(new T(std::forward<Args...>(args...)));
}

template<class T>
T& getChildRef(QObject const& self,
               QString const& name = QString(),
               ::Qt::FindChildOptions options = ::Qt::FindChildrenRecursively)
{
  auto result = self.findChild<T*>(name, options);
  return result == 0 ? throw QException() : *result;
}

}}

#endif // QTUTILITIES_H
