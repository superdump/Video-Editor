import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: editor
    width: 854
    height: 480

    Item {
        id: preview
        width: 512
        height: 288
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        Text {
            id: previewText
            text: qsTr("Video preview")
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 32
        }
    }

    Item {
        id: leftButtons
        anchors.bottom: timeline.top
        anchors.right: preview.left
        anchors.top: parent.top
        anchors.left: parent.left

        Button {
            id: projectsButton
            text: "Projects"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height / 2.0
        }
        Button {
            id: addMediaButton
            text: "Add Media"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: projectsButton.bottom
            anchors.bottom: parent.bottom
        }
    }

    Item {
        id: rightButtons
        anchors.bottom: timeline.top
        anchors.left: preview.right
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
            id: playButton
            text: "Play"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height / 2.0
        }
        Button {
            id: cameraButton
            text: "Camera"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: playButton.bottom
            anchors.bottom: parent.bottom
        }
    }

    Item {
        id: timeline

        anchors.top: preview.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            id: background
            anchors.fill: parent
            color: "grey"
        }

        Text {
            id: testText

            anchors.verticalCenter: parent.verticalCenter

            text: qsTr("Flickable, pinchable timeline -     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o     p     q     r     s     t     u     v     w     x     y     z     1     2     3     4     5     6     7     8     9     0")
            font.bold: true
            font.pixelSize: 32
            color: "white"

            x: -flickable.visibleArea.xPosition * flickable.contentWidth
        }

        Flickable {
            id: flickable

            anchors.fill: parent

            contentWidth: testText.width
            contentHeight: testText.height
        }
    }
}
