import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import GoL 1.0

MainWindow {
  id: mainWindow
  visible: true
  width: Screen.desktopAvailableWidth
  height: Screen.desktopAvailableHeight

  function createGameInstance(cells) {
    gameParams.fieldSize = cells
    var game_view = mainWindow.createGame()
    game_view.width = mainMenu.width
    game_view.height = mainMenu.height
    mainMenu.push(game_view)
  }

  Page {
    anchors.fill: parent

    header: ToolBar {
      id: toolBar
      contentHeight: toolButton.implicitHeight

      ToolButton {
        id: toolButton
        text: mainMenu.depth > 1 ? "\u25C0" : "\u2630"
        font.pixelSize: Qt.application.font.pixelSize * 1.6
        onClicked: {
          if (mainMenu.depth > 1) {
            mainMenu.back()
          } else {
            drawer.open()
          }
        }
      }

      Label {
//        text: mainMenu.currentItem.title
        anchors.centerIn: parent
      }
    }

    StackView {
      id: mainMenu
      width: mainWindow.width
      height: mainWindow.height - toolBar.contentHeight

      function back() {
        mainMenu.pop()
        if (mainWindow.destroyGame()) {}
      }

      Keys.onReleased: {
        if (event.key === Qt.Key_Back && mainMenu.depth > 1) {
          event.accepted = true
          mainMenu.back()
        }
      }
      initialItem: Item {}
    }
  }

  Drawer {
    id: drawer
    width: mainWindow.width * 0.66
    height: mainWindow.height

    Column {
      anchors.fill: parent

      ItemDelegate {
        text: qsTr("Single player")
        width: parent.width
        onClicked: {
          mainMenu.push(singlePlayerMenu)
          drawer.close()
        }
      }
      ItemDelegate {
        text: qsTr("Multiplayer")
        width: parent.width
        onClicked: {
          mainMenu.push(multiplayerMenu)
          drawer.close()
        }
      }
      ItemDelegate {
        text: qsTr("Quit")
        width: parent.width
        onClicked: {
          Qt.quit()
        }
      }
    }
  }

  Component {
    id: singlePlayerMenu

    Page {
      title: qsTr("Single player")
      ComboBox {
        id: fieldSize
        clip: true
        visible: true
        anchors.centerIn: parent
        currentIndex: 1
        model: ListModel {
          ListElement { cells: 512 }
          ListElement { cells: 1024 }
          ListElement { cells: 2048 }
          ListElement { cells: 4096 }
          ListElement { cells: 8192 }
        }
      }

      footer: Button {
        height: 70
        text: qsTr("Start")
        onPressed: {
          createGameInstance(Qt.point(fieldSize.currentText, fieldSize.currentText))
        }
      }
    }
  }
  Component {
    id: multiplayerMenu

    Item {
      SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: indicator.currentIndex

        Page {
          id: joinRoom

          ListView {
            anchors.fill: parent
            model: lobbyList
            delegate: Rectangle {
              height: 25
              width: 100
              Text {
                text: model.modelData.name + " " + model.modelData.lobbyId
              }
            }
          }

          footer: Button {
            height: 70
            text: qsTr("Join")
            onPressed: {}
          }
        }

        Page {
          id: createRoom

          ComboBox {
            id: fieldSize
            clip: true
            visible: true
            anchors.centerIn: parent
            currentIndex: 1
            model: ListModel {
              ListElement { cells: 512 }
              ListElement { cells: 1024 }
              ListElement { cells: 2048 }
              ListElement { cells: 4096 }
              ListElement { cells: 8192 }
            }
          }

          footer: Button {
            height: 70
            text: qsTr("Create")
            onPressed: {
              createLobby()
            }
          }
        }
      }

      PageIndicator {
        id: indicator

        currentIndex: swipeView.currentIndex
        count: swipeView.count

        anchors.bottom: swipeView.bottom
        anchors.horizontalCenter: swipeView.horizontalCenter
      }
    }
  }
}

