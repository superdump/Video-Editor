// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: settingsPage
    orientationLock: PageOrientation.LockLandscape

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Text {
        id: titleField
        text: "Export settings"
        font.bold: true
        font.pixelSize: 40
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }

    Column {
        anchors.margins: 16
        anchors.top: titleField.bottom
        anchors.bottom: buttonExport.top
        anchors.left: parent.left
        anchors.right: parent.right

        ButtonRow {
            id: resolution
            checkedButton: buttonHD
            platformStyle: ButtonStyle {
                inverted: true
            }

            Button {
                id: buttonHD
                text: "HD"
            }
            Button {
                id: buttonWVGA
                text: "WVGA"
            }
            Button {
                id: buttonVGA
                text: "VGA"
            }
        }
    }

    ButtonStyle {
        id: invertedStyle
        inverted: true
    }

    Button {
        id: buttonExport
        text: "Export"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 16
        platformStyle: invertedStyle
        onClicked: {
            switch(resolution.checkedButton) {
            case buttonVGA:
                videoeditor.setRenderSettings(640, 480, 30, 1);
                break;
            case buttonWVGA:
                videoeditor.setRenderSettings(848, 480, 30, 1);
                break;
            case buttonHD:
                // fall through
            default:
                videoeditor.setRenderSettings(1280, 720, 30, 1);
                break;
            }

            pageStack.pop(this)
            progressDialog.open()
            videoeditor.render()
        }
    }

    Button {
        id: buttonCancel
        text: "Cancel"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        platformStyle: invertedStyle
        onClicked: pageStack.pop(this)
    }
}
