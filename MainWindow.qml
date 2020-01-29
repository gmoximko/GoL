import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import GoL 1.0

MainWindow {
  id: mainWindow
  visible: true
  width: Screen.desktopAvailableWidth
  height: Screen.desktopAvailableHeight

  function createGameInstance(gameParams) {
    var gameView = mainWindow.createGame(gameParams)
    gameView.width = mainWindow.width
    gameView.height = mainWindow.height
    gameView.gameSpeed = gameParams.gameSpeed
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
            height: (parent.height - startGame.height - quitButton.height) / 2
          }
          Button {
            id: startGame
            text: qsTr("Start game")
            width: parent.width
            onClicked: {
              mainMenu.push(singleplayerMenu)
            }
          }
          Button {
            id: quitButton
            text: qsTr("Quit")
            width: parent.width
            onClicked: {
              Qt.quit()
            }
          }
        }
      }
    }
  }

  Component {
    id: singleplayerMenu

    Page {
      GameParams {
        id: gameParams
        gameSpeed: 50
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
        Slider {
          id: gameSpeed
          snapMode: Slider.SnapAlways
          from: 100
          value: 50
          to: 1
          stepSize: 1
          onMoved: () => gameParams.gameSpeed = value
        }
        ComboBox {
          id: fieldSize
          width: gameSpeed.width
          currentIndex: 1
          model: [1024, 2048, 4096, 8192, 16384, 32768]
          onCurrentTextChanged: () => gameParams.fieldSize = Qt.point(currentText, currentText)
        }
      }
      footer: Button {
        text: qsTr("Start")
        onPressed: () => createGameInstance(gameParams)
      }
    }
  }
}

