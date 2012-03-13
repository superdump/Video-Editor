// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: settingsPage

    Text {
        id: titleField
        text: "Export settings"
        font.bold: true
        font.pixelSize: 40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }

    Rectangle {
        width: parent.width

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Column {
            id: col
            spacing: 10

            RadioButton {
                id: buttonHD
                text: "HD"
                checked: true
                onCheckedChanged: {
                    if(checked) {
                        buttonWVGA.checked = false
                        buttonVGA.checked = false
                    }
                }
            }
            RadioButton {
                id: buttonWVGA
                text: "WVGA"
                onCheckedChanged: {
                    if(checked) {
                        buttonHD.checked = false
                        buttonVGA.checked = false
                    }
                }
            }
            RadioButton {
                id: buttonVGA
                text: "VGA"
                onCheckedChanged: {
                    if(checked) {
                        buttonHD.checked = false
                        buttonWVGA.checked = false
                    }
                }
            }
        }

    }

    ButtonRow {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 16

        style: ButtonStyle { }

        Button {
            text: "Export"
            onClicked: {
                if(buttonHD.checked) {
                    videoeditor.setRenderSettings(1280, 720, 30, 1);
                } else if(buttonWVGA.checked) {
                    videoeditor.setRenderSettings(848, 480, 30, 1);
                } else {
                    videoeditor.setRenderSettings(640, 480, 30, 1);
                }
                pageStack.pop(this)
                progressDialog.open()
                videoeditor.render()
            }
        }
        Button {
            text: "Cancel"
            onClicked: pageStack.pop(this)
        }
    }
}
