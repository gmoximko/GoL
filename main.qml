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
        enabled: isNetworkEnabled()
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
        model: [512, 1024, 2048, 4096, 8192]
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
            delegate: Item {
              width: parent.width
              height: 40
              MouseArea {
                anchors.fill: parent
                onClicked: {
                  joinLobby(model.modelData)
                }
              }
              Row {
                spacing: 10
                Text {
                  anchors.verticalCenter: parent.verticalCenter
                  text: model.modelData.name
                }
                Text {
                  anchors.verticalCenter: parent.verticalCenter
                  text: "%1 %2".arg(model.modelData.fieldSize.x).arg(model.modelData.fieldSize.y)
                }
                Text {
                  anchors.verticalCenter: parent.verticalCenter
                  text: model.modelData.gameSpeed
                }
                Text {
                  anchors.verticalCenter: parent.verticalCenter
                  text: model.modelData.playerCount
                }
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
          Column {
            anchors.centerIn: parent
            TextField {
              id: lobbyName
              text: Lobby
              placeholderText: qsTr("Enter lobby name")
            }
            ComboBox {
              id: fieldSize
              clip: true
              visible: true
              currentIndex: 1
              model: [512, 1024, 2048, 4096, 8192]
            }
            ComboBox {
              id: playerCount
              clip: true
              visible: true
              currentIndex: 1
              model: [1, 2, 3, 4]
            }
            ComboBox {
              id: gameSpeed
              clip: true
              visible: true
              currentIndex: 1
              model: [50, 100, 200, 300, 400, 500]
            }
          }
          footer: Button {
            height: 70
            text: qsTr("Create")
            onPressed: {
              gameParams.name = lobbyName.text
              gameParams.fieldSize = Qt.point(fieldSize.currentText, fieldSize.currentText)
              gameParams.playerCount = Number(playerCount.currentText)
              gameParams.gameSpeed = Number(gameSpeed.currentText)
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

