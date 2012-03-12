// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Dialog {
    id: settingsDialog

    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter

    status: DialogStatus.Closed
    visualParent: timeline

    title: Text {
        id: titleField
        color: "white"
        text: "Export settings:"
        font.pixelSize: 28
        anchors.horizontalCenter: parent.horizontalCenter
    }

    content: Item {
        width: parent.width
    }

    buttons: ButtonRow {
        style: ButtonStyle { }
        anchors.horizontalCenter: parent.horizontalCenter
        Button {
            text: "Ok"
            onClicked: settingsDialog.accept()
        }
        Button {
            text: "Cancel"
            onClicked: settingsDialog.reject()
        }
    }

    onRejected: settingsDialog.close()
}
