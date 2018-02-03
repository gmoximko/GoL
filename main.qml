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
          currentIndex: 2
          model: ListModel {
            ListElement { cells: 16 }
            ListElement { cells: 32 }
            ListElement { cells: 64 }
            ListElement { cells: 128 }
            ListElement { cells: 256 }
          }
        }

        footer: Button {
          height: 70
          text: qsTr("Create")
          onPressed: {
            createGame(Qt.point(comboBox.currentText, comboBox.currentText))
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

  function createGame(cells) {
    createGameInstance(cells)
    mainMenu.enabled = false
    mainMenu.visible = false
  }
}

