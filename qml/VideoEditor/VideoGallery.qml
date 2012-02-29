// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: gallery
    width: 854
    height: 480

    Button {
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: 10
        }

        text: "Cancel"
        onClicked: {
            pageStack.pop(this)
        }
    }
}
