import QtQuick 2.0

Item {
  Column {
    id: column
    anchors.fill: parent

    ListView {
      id: listView
      width: 110
      height: 160
        delegate: Item {
          x: 5
          width: 80
          height: 40
          Row {
            id: row1
            Rectangle {
              width: 40
              height: 40
              color: colorCode
            }

            Text {
              text: name
              anchors.verticalCenter: parent.verticalCenter
              font.bold: true
            }
            spacing: 10
          }
        }
        model: ListModel {
          ListElement {
            name: "Grey"
            colorCode: "grey"
          }

          ListElement {
            name: "Red"
            colorCode: "red"
          }

          ListElement {
            name: "Blue"
            colorCode: "blue"
          }

          ListElement {
            name: "Green"
            colorCode: "green"
          }
        }
      }

      ListView {
        id: listView1
        width: 110
        height: 160
          delegate: Item {
              x: 5
              width: 80
              height: 40
              Row {
                  id: row2
                  Rectangle {
                      width: 40
                      height: 40
                      color: colorCode
                  }

                  Text {
                      text: name
                      anchors.verticalCenter: parent.verticalCenter
                      font.bold: true
                  }
                  spacing: 10
              }
          }
          model: ListModel {
              ListElement {
                  name: "Grey"
                  colorCode: "grey"
              }

              ListElement {
                  name: "Red"
                  colorCode: "red"
              }

              ListElement {
                  name: "Blue"
                  colorCode: "blue"
              }

              ListElement {
                  name: "Green"
                  colorCode: "green"
              }
          }
      }
  }

}
