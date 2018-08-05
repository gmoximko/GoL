import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import GoL 1.0

MainWindow {
  id: mainWindow
  visible: true
  width: Screen.desktopAvailableWidth
  height: Screen.desktopAvailableHeight

  function createGameInstance(gameParams) {
    var gameView = mainWindow.createGame(gameParams)
    gameView.width = mainMenu.width
    gameView.height = mainMenu.height
    mainMenu.push(gameView).focus = true
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
        text: mainMenu.currentItem.title
        anchors.centerIn: parent
      }
    }

    StackView {
      id: mainMenu
      width: mainWindow.width
      height: mainWindow.height - toolBar.contentHeight

      function back() {
        if (mainWindow.destroyGame()) {}
        mainMenu.pop(null)
      }

      Keys.onReleased: {
        if (event.key === Qt.Key_Back && mainMenu.depth > 1) {
          event.accepted = true
          mainMenu.back()
        }
      }
      initialItem: Page {
        title: qsTr("Main Menu")
      }
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
          mainMenu.push(singleplayerMenu)
          drawer.close()
        }
      }
      ItemDelegate {
        text: qsTr("Multiplayer")
        width: parent.width
        visible: false
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
    id: singleplayerMenu

    Page {
      GameParameters {
        id: gameParams
      }
      title: qsTr("Single player")

      footer: Button {
        height: 70
        text: qsTr("Start")
        onPressed: {
          createGameInstance(gameParams.params)
        }
      }
    }
  }

  Component {
    id: multiplayerMenu

    Item {
      Connections {
        target: gameParams.params
        onStart: {
          createGameInstance(params)
        }
      }

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
                  gameParams.params.setParams(model.modelData)
                  joinLobby(gameParams.params)
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

          GameParameters {
            id: gameParams
            isMultiplayer: true
          }
          footer: Button {
            height: 70
            text: qsTr("Create")
            onPressed: {
              createLobby(gameParams.params)
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

