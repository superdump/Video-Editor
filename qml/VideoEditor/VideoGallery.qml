// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0
import QtMobility.gallery 1.1

Page {
    id: gallery
    width: 854
    height: 480

    ListView {
        id: listView
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: addButton.top
            margins: 16
        }
        focus:true
        highlight: Rectangle {
            color: "#FFFFFF"
            radius: 5
        }
        spacing: 8

        model: DocumentGalleryModel {
            id: docGalModel
            rootType: DocumentGallery.Video
            properties: [ "url" ]
            autoUpdate: true


            onProgressChanged: {
                console.log("Model progress: " + progress)
            }
        }

        delegate: Text {
            width: listView.width - 32
            height: 64
            anchors.horizontalCenter: parent.horizontalCenter
            verticalAlignment: Text.AlignVCenter
            text: url
            font.pointSize: 26
            elide: Text.ElideLeft
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index;
                }
            }
        }
    }

    Button {
        id: addButton
        width: 128
        height: 92
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: 16
        }

        text: "Add"
        onClicked: {
            console.log("Selected: " + docGalModel.get(listView.currentIndex).url  + "  " + listView.currentItem)
            videoeditor.append(docGalModel.get(listView.currentIndex).url)
            pageStack.pop(this)
        }
    }

    Button {
        id: cancelButton
        width: 128
        height: 92
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: 16
        }

        text: "Cancel"
        onClicked: {
            pageStack.pop(this)
        }
    }
}
