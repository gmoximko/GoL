import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import GoL 1.0

MainWindow {
  id: mainWindow
  visible: true
  width: Screen.desktopAvailableWidth
  height: Screen.desktopAvailableHeight

  ApplicationWindow.onWindowChanged: loadGameInstance()

  function loadGameInstance() {
    let gameView = loadGame()
    if (gameView !== null) {
      gameView.gameSpeed = gameSpeed()
      mainMenu.push(singleplayerMenu, {}, StackView.Immediate)
      mainMenu.push(gameView, {}, StackView.Immediate).focus = true
    }
  }

  function createGameInstance(gameParams) {
    let gameView = mainWindow.createGame(gameParams)
    gameView.gameSpeed = gameSpeed()
    mainMenu.push(gameView).focus = true
  }

  Page {
    anchors.fill: parent

    StackView {
      id: mainMenu
      width: mainWindow.width
      height: mainWindow.height

      function back() {
        mainMenu.pop(null)
      }

      Keys.onReleased: {
        if (event.key === Qt.Key_Back && mainMenu.depth > 1) {
          event.accepted = true
          mainMenu.back()
        }
      }
      initialItem: Page {
        header: ToolBar {
          Label {
            text: qsTr("Game of Life")
            anchors.centerIn: parent
          }
        }
        Column {
          anchors.fill: parent
          Rectangle {
            width: parent.width
            height: (parent.height - startGame.height/* - quitButton.height*/) / 2
          }
          Button {
            id: startGame
            text: qsTr("Start game")
            width: parent.width
            onClicked: {
              mainMenu.push(singleplayerMenu)
            }
          }
//          Button {
//            id: quitButton
//            text: qsTr("Quit")
//            width: parent.width
//            onClicked: {
//              Qt.quit()
//            }
//          }
        }
      }
    }
  }

  Component {
    id: singleplayerMenu

    Page {
      GameParams {
        id: gameParams
        gameSpeed: gameSpeedSlider.value
        fieldSize: Qt.point(fieldSize.currentText, fieldSize.currentText)
      }
      header: ToolBar {
        id: toolBar
        contentHeight: toolButton.implicitHeight

        ToolButton {
          id: toolButton
          visible: mainMenu.depth === 2
          text: "‹" //"\u25C0"
          font.pixelSize: Qt.application.font.pixelSize * 1.6
          onClicked: {
            mainMenu.back()
          }
        }
        Label {
          text: qsTr("Field size")
          anchors.centerIn: parent
        }
      }
      Column {
        anchors.centerIn: parent
        Label {
          id: gameSpeedLabel
          text: "Game speed"
          width: gameSpeed.width
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
        }
        Row {
          id: gameSpeed
          Label {
            text: "slow"
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: gameSpeedLabel.font.pixelSize * 0.9
          }
          Slider {
            id: gameSpeedSlider
            snapMode: Slider.SnapAlways
            from: 100
            value: 50
            to: 1
            stepSize: 1
          }
          Label {
            text: "fast"
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: gameSpeedLabel.font.pixelSize * 0.9
          }
        }
        Label {
          text: "Field size"
          width: gameSpeed.width
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
        }
        ComboBox {
          id: fieldSize
          width: gameSpeed.width
          currentIndex: 1
//          model: ListModel {
//            ListElement { name: "very small"; value: 1024 }
//            ListElement { name: "small"; value: 2048 }
//            ListElement { name: "middle"; value: 4096 }
//            ListElement { name: "large"; value: 8192 }
//            ListElement { name: "very large"; value: 16384 }
//            ListElement { name: "huge"; value: 32768 }
//          }
          model: [1024, 2048, 4096, 8192, 16384, 32768]
        }
      }
      footer: Button {
        text: qsTr("Start")
        onPressed: () => createGameInstance(gameParams)
      }
    }
  }
}

