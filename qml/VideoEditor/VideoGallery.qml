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
        anchors.fill: parent
        focus:true
        highlight: Rectangle {
            color: "#FFFFFF"
            radius: 5
        }

        model: DocumentGalleryModel {
            rootType: DocumentGallery.Video
            properties: [ "url" ]
            autoUpdate: true


            onProgressChanged: {
                console.log("Model progress: " + progress)
            }
        }

        delegate: Text {
            text: url
            font.pointSize: 26
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index;
                }
            }
        }
    }

    Button {
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: 10
        }

        text: "Add"
        onClicked: {
            console.log("Selected: " + listView.currentItem)
            pageStack.pop(this)
        }
    }

    Button {
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: 10
        }

        text: "Cancel"
        onClicked: {
            pageStack.pop(this)
        }
    }
}
