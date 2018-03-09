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
          pinch.target: gameView
          pinch.maximumScale: Math.max(gameView.width / gameWindow.width, gameView.height / gameWindow.height)
          pinch.minimumScale: Math.max(gameWindow.width / gameView.width, gameWindow.height / gameView.height)

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
              patternsList.visible = true
            }
          }
        }
      }
    }

    ListView {
        id: patternsList
        visible: false
        width: gameWindow.width
        height: gameWindow.height * 0.3
        delegate: Item {
            x: 5
            width: 80
            height: 40
            Row {
                id: row1
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                }

                Text {
                    text: name
                    anchors.verticalCenter: parent.verticalCenter
                    font.bold: true
                }
                spacing: 10
            }
        }
        model: ListModel {
          ListElement {
              name: "Grey"
              colorCode: "grey"
          }

          ListElement {
              name: "Red"
              colorCode: "red"
          }

          ListElement {
              name: "Blue"
              colorCode: "blue"
          }

          ListElement {
              name: "Green"
              colorCode: "green"
          }
      }
    }
  }
}
