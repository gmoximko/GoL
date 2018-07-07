import QtQuick 2.10
import QtQuick.Controls 2.3
import GoL 1.0

Column {
  anchors.centerIn: parent

  property alias params: params
  property bool isMultiplayer: false

  GameParams {
    id: params
  }

  TextField {
    id: lobbyName
    enabled: isMultiplayer
    visible: isMultiplayer
    text: qsTr("Lobby")
    placeholderText: qsTr("Enter lobby name")

    onTextChanged: {
      params.name = text
    }
  }
  ComboBox {
    id: playerCount
    enabled: isMultiplayer
    visible: isMultiplayer
    clip: true
    currentIndex: 1
    model: [1, 2, 3, 4]

    onCurrentTextChanged: {
      params.playerCount = Number(currentText)
    }
  }
  ComboBox {
    id: fieldSize
    clip: true
    currentIndex: 1
    model: [512, 1024, 2048, 4096, 8192]

    onCurrentTextChanged: {
      params.fieldSize = Qt.point(currentText, currentText)
    }
  }
  ComboBox {
    id: gameSpeed
    clip: true
    currentIndex: 1
    model: [50, 100, 200, 300, 400, 500]

    onCurrentTextChanged: {
      params.gameSpeed = Number(currentText)
    }
  }
  ComboBox {
    id: initialScores
    clip: true
    currentIndex: 1
    model: [500, 1000, 1500, 2000, 2500]

    onCurrentTextChanged: {
      params.initialScores = Number(currentText)
    }
  }
}

