import QtQuick 2.10
import QtQuick.Window 2.10
import QtSensors 5.9
import GoL 1.0

Item {
  id: root
  width: Window.width
  height: Window.height

  Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: gameView.width * gameView.scale
    contentHeight: gameView.height * gameView.scale

    GameView {
      id: gameView
      width: fieldSize.x
      height: fieldSize.y
      fillColor: "#000000"
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter

      PinchArea {
        id: pinchArea
        anchors.fill: parent
        pinch.target: gameView
        pinch.maximumScale: Math.max(gameView.width / root.width, gameView.height / root.height)
        pinch.minimumScale: Math.max(root.width / gameView.width, root.height / gameView.height)

        MultiPointTouchArea {
          id: touchArea
          anchors.fill: parent
          mouseEnabled: true
          minimumTouchPoints: 1
          maximumTouchPoints: 1
          touchPoints: [
            TouchPoint { id: touchPoint }
          ]
          onPressed: {
            gameView.pressed(Qt.point(touchPoint.x, touchPoint.y))
          }
        }
      }
    }
  }
}
