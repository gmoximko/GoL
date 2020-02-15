import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtSensors 5.9
import GoL 1.0

Page {
  id: gameWindow
  width: parent.width
  height: parent.height
  title: "Game of life"

  property alias gameSpeed: gameSpeed.value

  StackView.onRemoved: {
    mainWindow.destroyGame()
  }

  GameView {
    id: gameView
    width: gameWindow.width
    height: gameWindow.height
    anchors.top: parent.top
    anchors.horizontalCenter: parent.horizontalCenter
    darkTheme: darkThemeSwitch.checked

    property point start: Qt.point(0, 0)
    function dragField(point) {
      var offset = fieldOffset
      fieldOffset = Qt.point(offset.x + point.x - start.x, offset.y + point.y - start.y)
      start = point
    }
    function selectCell(point) {
      pressed(point)
      if (!patterns.opened && !patterns.enter.running)
      {
        patterns.open()
      }
    }
    function selectPatt() {
      selectPattern()
      patterns.close()
    }

    DragHandler {
      id: dragHandler
      target: null
      minimumPointCount: 1
      maximumPointCount: 1

      onActiveChanged: {
        if (active) {
          gameView.start = centroid.position
        }
      }
      onCentroidChanged: {
        if (active) {
          gameView.dragField(centroid.position)
        }
      }
    }
    PinchHandler {
      target: null
      minimumPointCount: 2
      maximumPointCount: 2
      minimumScale: gameView.minScale
      maximumScale: gameView.maxScale

      property int rotationTimes: 0

      onActiveChanged: {
        rotationTimes = 0
      }
      onScaleChanged: {
        gameView.zoom(scale, centroid.position)
      }
      onRotationChanged: {
        var cmpint = (v1, v2) => {
          if (v1 < v2) return 1
          if (v1 > v2) return -1
          return 0
        }
        var newRotationTimes = parseInt(rotation / 90)
        var rotationTime = cmpint(rotationTimes, newRotationTimes)

        if (rotationTime !== 0) {
          gameView.rotatePattern(90 * rotationTime)
          rotationTimes = newRotationTimes
        }
      }
    }
    Timer {
      id: singleTapTimer
      interval: 200
      onTriggered: {
        gameView.selectCell(pointPosition)
      }

      property point pointPosition: Qt.point(0, 0)
    }
    TapHandler {
      onSingleTapped: {
        if (eventPoint.timeHeld < longPressThreshold) {
          singleTapTimer.pointPosition = eventPoint.position
          singleTapTimer.restart()
          eventPoint.accepted = true
        }
      }
      onDoubleTapped: {
        if (gameView.currentPattern === undefined) {
          gameView.pressed(eventPoint.position)
        }
        singleTapTimer.stop()
        gameView.selectPatt()
        eventPoint.accepted = true
      }
      onLongPressed: {
        if (gameView.currentPattern !== undefined) {
          gameView.selectPatt()
        }
      }
    }
    MouseArea {
      anchors.fill: parent
      scrollGestureEnabled: false
      propagateComposedEvents: true
      acceptedButtons: { Qt.NoButton }

      onWheel: { // TODO
//        var scaleRatio = gameView.fieldScale * wheel.angleDelta.y / 120 / 10
//        gameView.zoom(scaleRatio * (wheel.inverted ? -1 : 1), Qt.point(wheel.x, wheel.y))
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

  RoundButton {
    text: "S"
    anchors.top: parent.top
    anchors.right: parent.right
    visible: !menu.opened
    onClicked: () => menu.open()
  }

  Drawer {
    id: patterns
    width: mainWindow.width
    height: mainWindow.height * 0.33
    edge: Qt.BottomEdge
    closePolicy: Popup.NoAutoClose
    modal: false
    onClosed: {
      gameView.unpress()
      patternsList.currentIndex = -1
    }

    Column {
      Label {
        id: patternsLabel
        text: "Patterns"
        width: patterns.width
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }
      ListView {
        id: patternsList
        clip: true
        focus: true

        width: patterns.width
        height: patterns.height - patternsLabel.height

        model: gameView.patternCount
        delegate: patternModel
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        currentIndex: -1
        onCurrentItemChanged: {
          if (currentItem != null) {
            gameView.currentPattern = currentItem.pattern
          }
        }
      }
    }
    RoundButton {
      text: "F"
      visible: gameView.currentPattern !== undefined
      anchors.top: parent.top
      anchors.right: parent.right
      onClicked: () => gameView.flipPattern()
    }
  }
  RoundButton {
    text: "P"
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    onClicked: () => gameView.stop()
  }

  Drawer {
    id: menu
    edge: Qt.RightEdge
    width: mainWindow.width * 0.33
    height: mainWindow.height

    Column {
      anchors.fill: parent

      Label {
        id: gameSpeedLabel
        text: "Game speed"
        width: quitButton.width
        height: quitButton.height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }
      Label {
        id: fastSpeed
        text: "fast"
        width: quitButton.width
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: gameSpeedLabel.font.pixelSize * 0.9
      }
      Slider {
        id: gameSpeed
        width: quitButton.width
        height: quitButton.width
        snapMode: Slider.SnapAlways
        from: 100
        value: 50
        to: 1
        stepSize: 1
        orientation: Qt.Vertical
        onMoved: () => gameView.gameSpeedChanged(gameSpeed.value)
      }
      Label {
        id: slowSpeed
        text: "slow"
        width: quitButton.width
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: gameSpeedLabel.font.pixelSize * 0.9
      }
      Rectangle {
        id: darkTheme
        width: quitButton.width
        height: quitButton.height
        Switch {
          id: darkThemeSwitch
          text: "Dark theme"
          anchors.centerIn: parent
          checked: true
        }
      }
      Rectangle {
        width: parent.width
        height: parent.height
                - gameSpeedLabel.height
                - fastSpeed.height
                - gameSpeed.height
                - slowSpeed.height
                - darkTheme.height
                - quitButton.height
      }
      Button {
        id: quitButton
        text: "Quit"
        width: parent.width
        onClicked: {
          gameWindow.enabled = false
          mainMenu.back()
          menu.close()
          if (patterns.opened) {
            patterns.close()
          }
        }
      }
    }
  }
}
