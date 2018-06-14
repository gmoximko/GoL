import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import QtSensors 5.9
import GoL 1.0

Item {
  id: gameWindow
  width: parent.width
  height: parent.height

  states: [
    State {
      name: "choosePattern"
      when: patternsList.visible
      PropertyChanges {
        target: gameView
        height: gameWindow.height - patternsList.height
      }
    }
  ]

  GameView {
    id: gameView
    width: gameWindow.width
    height: gameWindow.height
    fillColor: "#000000"
    anchors.top: parent.top
    anchors.horizontalCenter: parent.horizontalCenter
    fieldScale: maxScale

    property point start: Qt.point(0, 0)
    function dragField(point) {
      var offset = fieldOffset
      fieldOffset = Qt.point(offset.x + point.x - start.x, offset.y + point.y - start.y)
      start = point
    }
    function selectCell(point) {
      pressed(point)
      patternsList.show()
    }
    function selectPatt() {
      selectPattern()
      patternsList.hide()
    }
    PinchArea {
      id: pinchArea
      anchors.fill: parent

      property real startScale: gameView.maxScale
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

      property real deltaAngle: 0
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

        onPressed: {
          gameView.start = Qt.point(mouseX, mouseY)
        }
        onPositionChanged: {
          gameView.dragField(Qt.point(mouseX, mouseY))
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
            gameView.selectPatt()
            mouse.accepted = true
          }
        }
        onPressAndHold: {
          if (gameView.currentPattern !== undefined) {
            gameView.selectPatt()
            mouse.accepted = true
          }
        }
      }

      MultiPointTouchArea {
        id: touchArea
        anchors.fill: parent
        mouseEnabled: false
        minimumTouchPoints: 1
        maximumTouchPoints: 1
        touchPoints: [
          TouchPoint { id: touchPoint }
        ]

        property bool wasHeld: false
        property real releasedTime: 0
        onReleased: {
          if (pressAndHoldTimer.running && !wasHeld) {
            gameView.selectCell(Qt.point(touchPoint.sceneX, touchPoint.sceneY))
          }
          pressAndHoldTimer.stop()
          releasedTime = new Date().getTime()
        }
        onPressed: {
          wasHeld = false
          var currentTime = new Date().getTime()
          if (currentTime - releasedTime < mouseArea.pressAndHoldInterval) {
            if (gameView.currentPattern === undefined) {
              gameView.selectPatt()
              wasHeld = true
            }
          }
          gameView.start = Qt.point(touchPoint.startX, touchPoint.startY)
          pressAndHoldTimer.restart()
        }
        onTouchUpdated: {
          gameView.dragField(Qt.point(touchPoint.sceneX, touchPoint.sceneY))
        }
        Timer {
          id: pressAndHoldTimer
          interval: mouseArea.pressAndHoldInterval
          running: false
          repeat: false
          onTriggered: {
            touchArea.wasHeld = true
            if (gameView.currentPattern !== undefined) {
              gameView.selectPatt()
            }
          }
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

    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
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
