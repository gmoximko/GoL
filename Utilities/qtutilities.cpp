#include <QThread>

#include "qtutilities.h"

namespace Utilities { namespace Qt {

ThreadChecker::ThreadChecker(QThread* thread)
  : thread_(thread)
{}

void ThreadChecker::check() const
{
  Q_ASSERT(thread_ == QThread::currentThread());
}

void ThreadChecker::check()
{
  Q_ASSERT((thread_ == nullptr ? thread_ = QThread::currentThread() : thread_)
           == QThread::currentThread());
}

}} // Utilities::Qt
