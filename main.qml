import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 1.4
import QtQuick.Controls 2.3
import GoL 1.0

MainWindow {
  id: mainWindow
  visible: true
  width: Screen.desktopAvailableWidth
  height: Screen.desktopAvailableHeight

  Item {
    id: mainMenu
    anchors.fill: parent

    SwipeView {
      id: swipeView
      anchors.fill: parent
      currentIndex: indicator.currentIndex

      Page {
        id: joinRoom

        header: Label {
          text: qsTr("Join room")
          fontSizeMode: Text.Fit
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }

        footer: Button {
          height: 70
          text: qsTr("Join")
          onPressed: {}
        }
      }

      Page {
        id: createRoom

        header: Label {
          text: qsTr("Create room")
          fontSizeMode: Text.Fit
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }

        ComboBox {
          id: comboBox
          clip: true
          visible: true
          anchors.fill: parent
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
            createGameInstance(Qt.point(comboBox.currentText, comboBox.currentText))
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

  function createGameInstance(cells) {
    mainWindow.cells = cells
    mainWindow.createGame()
    mainMenu.enabled = false
    mainMenu.visible = false
  }
}

