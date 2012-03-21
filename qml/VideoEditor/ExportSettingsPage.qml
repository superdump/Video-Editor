// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: settingsPage
    orientationLock: PageOrientation.LockLandscape

    Component.onCompleted: {
        switch(videoeditor.renderWidth) {
        case 640:
            resolution.checkedButton = buttonVGA;
            break;
        case 848:
            resolution.checkedButton = buttonWVGA;
            break;
        default:
            resolution.checkedButton = buttonHD;
            break;
        }
    }

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
        anchors.bottom: toolBar.top
        anchors.left: parent.left
        anchors.right: parent.right

        ButtonRow {
            id: resolution
            checkedButton: buttonHD

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

    ToolBar {
        id: toolBar
        anchors.bottom: parent.bottom
        tools: ToolBarLayout {
            ToolIcon {
                iconId: "toolbar-back"
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
                }
            }
        }
    }
}
