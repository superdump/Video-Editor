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
        text: "Settings"
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
        spacing: 16

        Item {
            height: resolutionLabel.height + resolution.height + 8
            anchors.left: parent.left
            anchors.right: parent.right

            Text {
                id: resolutionLabel
                anchors.top: parent.top
                text: "Export format:"
                color: "white"
                font.pixelSize: 23
            }

            ButtonRow {
                id: resolution
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                anchors.rightMargin: 8
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

        CheckBox {
            id: thumbnails
            checked: videoeditor.showThumbnails
            text: "Show thumbnails of video clips"
            onClicked: {
                videoeditor.showThumbnails = !videoeditor.showThumbnails;
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
