import QtQuick 2.10
import QtQuick.Window 2.10
import QtSensors 5.9
import GoL 1.0

Item {
  id: gameWindow
  width: Window.width
  height: Window.height

  Column {
    id: column
    anchors.fill: parent

    GameView {
      id: gameView
      width: gameWindow.width
      height: patternsList.visible ? gameWindow.height - patternsList.height : gameWindow.height
      fillColor: "#000000"
      anchors.horizontalCenter: parent.horizontalCenter
      fieldScale: maxScale

      function selectCell(point) {
        pressed(point)
        patternsList.show()
      }
      PinchArea {
        id: pinchArea
        anchors.fill: parent

        property real startScale
        property real sensitivity: 5

        onPinchStarted: {
          startScale = gameView.fieldScale
          deltaAngle = 0
        }
        onPinchUpdated: {
          var scaleRatio = startScale * (pinch.scale - pinch.previousScale)
          gameView.zoom(scaleRatio, pinch.startCenter)

          var rotationRatio = sensitivity * (pinch.angle - pinch.previousAngle)
          rotatePattern(rotationRatio)
        }
        onPinchFinished: {}

        property real deltaAngle
        function rotatePattern(delta) {
          deltaAngle += delta
          if (Math.abs(deltaAngle) >= 90) {
            gameView.rotatePattern(deltaAngle > 0 ? 90 : -90)
            deltaAngle = 0
          }
        }

        MouseArea {
          id: mouseArea
          anchors.fill: parent
          scrollGestureEnabled: false
          acceptedButtons: { Qt.LeftButton | Qt.RightButton }

          property real startX
          property real startY
          onPressed: {
            startX = mouseX
            startY = mouseY
          }
          onPositionChanged: {
            var offset = gameView.fieldOffset
            gameView.fieldOffset = Qt.point(offset.x + mouseX - startX, offset.y + mouseY - startY)
            startX = mouseX
            startY = mouseY
          }
          onWheel: {
            var scaleRatio = gameView.fieldScale * wheel.angleDelta.y / 120 / 10
            gameView.zoom(scaleRatio * (wheel.inverted ? -1 : 1), Qt.point(wheel.x, wheel.y))
          }
          onClicked: {
            if (!mouse.wasHeld) {
              gameView.selectCell(Qt.point(mouse.x, mouse.y))
              mouse.accepted = true
            }
          }
          onDoubleClicked: {
            if (gameView.currentPattern === undefined) {
              selectPattern()
              mouse.accepted = true
            }
          }
          onPressAndHold: {
            if (gameView.currentPattern !== undefined) {
              selectPattern()
              mouse.accepted = true
            }
          }
          function selectPattern() {
            gameView.selectPattern()
            patternsList.hide()
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
    }

    Component {
      id: patternModel

      Item {
        property var pattern: gameView.patternModelAt(index)
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
      model: gameView.patternCount
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
