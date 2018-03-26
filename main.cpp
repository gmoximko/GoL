#include <QGuiApplication>
#include <QQuickView>

#include "GameView/gameview.h"
#include "GameView/mainwindow.h"

void registerQmlTypes()
{
  qmlRegisterType<View::GameView>("GoL", 1, 0, "GameView");
  qmlRegisterType<View::MainWindow>("GoL", 1, 0, "MainWindow");
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif 
  QGuiApplication app(argc, argv);
  registerQmlTypes();

  QQuickView view(QUrl(QStringLiteral("qrc:/main.qml")));
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.show();
  return app.exec();
}
