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
          property int minCellsInScreen: 10
          pinch.maximumScale: Math.min(gameWindow.width / (gameView.pixelsPerCell.x * minCellsInScreen),
                                       gameWindow.height / (gameView.pixelsPerCell.y * minCellsInScreen))
          pinch.minimumScale: Math.max(gameWindow.width / gameView.width, gameWindow.height / gameView.height)

          property real startScale
          property real deltaAngle
          property real sensitivity: 5
          onPinchStarted: {
            startScale = gameView.scale
            deltaAngle = 0
          }
          onPinchUpdated: {
            var scaleRatio = startScale * (pinch.scale - pinch.previousScale)
            zoomGameView(scaleRatio, pinch.startCenter)

            var rotationRatio = sensitivity * (pinch.angle - pinch.previousAngle)
            rotatePattern(rotationRatio)
          }
          onPinchFinished: {
            flickable.returnToBounds()
          }

          function rotatePattern(delta) {
            deltaAngle += delta
            if (Math.abs(deltaAngle) >= 90) {
              gameView.rotatePattern(deltaAngle > 0 ? 90 : -90)
              deltaAngle = 0
            }
          }

          function zoomGameView(ratio, point) {
            var newScale = gameView.scale + ratio
            var maxScale = pinchArea.pinch.maximumScale
            var minScale = pinchArea.pinch.minimumScale
            console.assert(minScale < maxScale, minScale, maxScale)

            if (newScale > maxScale) {
              ratio = maxScale - gameView.scale
              newScale = maxScale
            } else if (newScale < minScale) {
              ratio = minScale - gameView.scale
              newScale = minScale
            }
            console.assert(gameView.scale + ratio <= maxScale, gameView.scale + ratio)
            console.assert(gameView.scale + ratio >= minScale, gameView.scale + ratio)

            gameView.scale = newScale
            flickable.contentX += point.x * ratio
            flickable.contentY += point.y * ratio
          }

          MouseArea {
            id: mouseArea
            anchors.fill: parent
            scrollGestureEnabled: false
            acceptedButtons: { Qt.LeftButton | Qt.RightButton }

            onWheel: {
              var scaleRatio = gameView.scale * wheel.angleDelta.y / 120 / 10
              pinchArea.zoomGameView(scaleRatio * (wheel.inverted ? -1 : 1), Qt.point(wheel.x, wheel.y))
            }
            onClicked: {
              gameView.selectCell(Qt.point(mouse.x, mouse.y))
            }
            onDoubleClicked: {
              if (gameView.currentPattern !== undefined) {
                gameView.selectPattern()
                patternsList.hide()
                mouse.accepted = false
              }
            }
          }

          MultiPointTouchArea {
            id: touchArea
            anchors.fill: parent
            mouseEnabled: false
            minimumTouchPoints: 1
            maximumTouchPoints: 2
            touchPoints: [
              TouchPoint { id: touchPoint1 },
              TouchPoint { id: touchPoint2 }
            ]
            onReleased: {
              gameView.selectCell(touchPoint1)
            }
          }
        }

        function selectCell(point) {
          gameView.pressed(Qt.point(point.x, point.y))
          patternsList.show()
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
      visible: false
      clip: true
      focus: true
      width: gameWindow.width
      height: gameWindow.height * 0.3
      model: gameWindow.patternCount
      delegate: patternModel
      highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
      currentIndex: -1
      onCurrentItemChanged: {
        if (currentIndex >= 0) {
          gameView.currentPattern = currentItem.pattern
        }
      }

      function show() {
        visible = true
      }
      function hide() {
        visible = false
        currentIndex = -1
      }
    }
  }
}
