import QtQuick 2.10
import QtQuick.Window 2.10
import QtSensors 5.9
import GoL 1.0

GameWindow {
  id: gameWindow
  width: Window.width
  height: Window.height

  Column {
    id: column
    anchors.fill: parent

    Flickable {
      id: flickable
      clip: true
      width: gameWindow.width
      height: patternsList.visible ? gameWindow.height - patternsList.height : gameWindow.height
      contentWidth: gameView.width * gameView.scale
      contentHeight: gameView.height * gameView.scale

      GameView {
        id: gameView
        width: fieldSize.x
        height: fieldSize.y
        fillColor: "#000000"
        scale: pinchArea.pinch.maximumScale
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        PinchArea {
          id: pinchArea
          anchors.fill: parent
          pinch.maximumScale: Math.max(gameView.width / gameWindow.width, gameView.height / gameWindow.height)
          pinch.minimumScale: Math.max(gameWindow.width / gameView.width, gameWindow.height / gameView.height)

          property real startScale
          onPinchStarted: {
            startScale = gameView.scale
          }
          onPinchUpdated: {
            var scaleRatio = startScale * (pinch.scale - pinch.previousScale)
            zoomGameView(scaleRatio, pinch.startCenter)
          }
          onPinchFinished: {
            flickable.returnToBounds()
          }

          function zoomGameView(ratio, point) {
            var newScale = gameView.scale + ratio
            var maxScale = pinchArea.pinch.maximumScale
            var minScale = pinchArea.pinch.minimumScale

            if (newScale > maxScale) {
              ratio = maxScale - gameView.scale
              newScale = maxScale
            } else if (newScale < minScale) {
              ratio = minScale - gameView.scale
              newScale = minScale
            }
            console.assert(gameView.scale + ratio <= maxScale)
            console.assert(gameView.scale + ratio >= minScale)

            gameView.scale = newScale
            flickable.contentX += point.x * ratio
            flickable.contentY += point.y * ratio
          }

          MultiPointTouchArea {
            id: touchArea
            anchors.fill: parent
            mouseEnabled: true
            minimumTouchPoints: 1
            maximumTouchPoints: 1
            touchPoints: [
              TouchPoint { id: touchPoint }
            ]
            onReleased: {
              gameView.pressed(Qt.point(touchPoint.x, touchPoint.y))
            }
          }
        }
      }
    }

    Component {
      id: patternModel

      Item {
        property var pattern: gameWindow.patternModelAt(index)
        x: 5
        width: parent.width
        height: 40
        MouseArea {
          anchors.fill: parent
          onClicked: {
            parent.ListView.view.currentIndex = index
          }
        }
        Column {
          Row {
            Text {
              text: pattern.name
              anchors.verticalCenter: parent.verticalCenter
              font.bold: true
            }
            Text {
              text: "scores: " + pattern.scores
              anchors.verticalCenter: parent.verticalCenter
            }
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
          }
          Text {
            text: "size: " + pattern.size
          }
          spacing: 10
        }
      }
    }

    ListView {
      id: patternsList
      visible: true
      clip: true
      focus: true
      width: gameWindow.width
      height: gameWindow.height * 0.3
      model: gameWindow.patternCount
      delegate: patternModel
      highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
      currentIndex: -1
      onCurrentItemChanged: {
        gameView.currentPattern = currentItem.pattern
      }
    }
  }
}
