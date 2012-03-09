import QtQuick 1.1
import com.nokia.meego 1.0
import VideoEditor 1.0

Page {
    id: timeline
    width: 854
    height: 480

    VideoEditor {
        id: videoeditor

        //onError: {
        //    console.debug("Error: " + message + " (" + debug + ")");
        //}

        onRenderComplete: {
            progressDialog.close();
            messageTitleField.text = "Export complete";
            messageDialog.open();
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }


    Item {
        id: preview
        width: 512
        height: 288
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        Rectangle {
            id: previewBackground

            anchors.fill: parent

            // This is the RGB hex value for the color key in omapxvsink
            color: "#080810"
        }

        Text {
            id: previewText
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Video preview\n(Coming soon!)")
            color: "white"
            font.bold: true
            font.pixelSize: 32
        }
    }

    Item {
        id: leftButtons
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: preview.left
        anchors.bottom: timelineBar.top

        Button {
            id: addMediaButton
            text: "Add Media"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height/2 - 24
            onClicked: {
                var component = Qt.createComponent("VideoGallery.qml")
                pageStack.push(component);
            }
        }

        Button {
            id: removeAllButton
            text: "Clear"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height/2 - 24
            onClicked: {
                videoeditor.removeAll();
            }
        }
    }

    Dialog {
        id: progressDialog
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        status: DialogStatus.Closed
        visualParent: timeline

        title: Text {
            id: titleField
            color: "white"
            text: "Export progress:"
            font.pixelSize: 28
            anchors.horizontalCenter: parent.horizontalCenter
        }

        content: Item {
            width: parent.width
            height: 32
            ProgressBar {
                id: progressBar
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                value: videoeditor.progress
            }
        }

        buttons: Button {
            text: "Cancel"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: progressDialog.reject()
        }

        onRejected: videoeditor.cancelRender()
    }

    Dialog {
        id: messageDialog
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        status: DialogStatus.Closed
        visualParent: timeline

        title: Text {
            id: messageTitleField
            color: "white";
            font.pixelSize: 28
            anchors.horizontalCenter: parent.horizontalCenter
        }

        content: Item {
            height: 8
        }

        buttons: Button {
            text: "OK"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: messageDialog.close()
        }
    }

    Item {
        id: rightButtons
        anchors.bottom: timelineBar.top
        anchors.left: preview.right
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
            id: exportButton
            text: "Export"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height - 24

            onClicked: {
                progressDialog.open()
                videoeditor.render()
            }
        }
    }

    Item {
        id: timelineBar

        anchors.top: preview.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            id: timelineBackground
            anchors.fill: parent
            color: "grey"
        }

        ListView {
            id: list
            model: videoeditor

            width: parent.width - 32
            height: parent.height - 32
            anchors.margins: 16
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            highlightRangeMode: ListView.ApplyRange
            spacing: 16

            orientation: ListView.Horizontal

            boundsBehavior: Flickable.StopAtBounds

            delegate: Column {
                anchors.verticalCenter: parent.verticalCenter
                Rectangle {
                    color: "light grey"
                    width: list.width / 3
                    height: list.height
                    Text {
                        width: parent.width - 16
                        height: parent.height - 16
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        text: fileName
                        font.pointSize: 20
                        color: "black"
                        wrapMode: Text.WrapAnywhere
                        maximumLineCount: 4
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // x: parent.width / 2 - flickable.contentWidth * flickable.visibleArea.xPosition / (1.0 - flickable.visibleArea.widthRatio)
        }
    }
}
