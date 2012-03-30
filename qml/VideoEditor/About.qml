import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: aboutPage
    orientationLock: PageOrientation.LockLandscape

    Text {
        id: titleField
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 16
        font.bold: true
        font.pixelSize: 40
        color: "white"
        text: "About VideoEditor"
    }

    Item {
        anchors.top: titleField.bottom
        anchors.bottom: toolBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        Text {
            id: mainText
            anchors.top: parent.top
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 32
            anchors.right: parent.right
            anchors.rightMargin: 32
            font.pixelSize: 18
            color: "white"
            smooth: true
            wrapMode: Text.WordWrap
            text: "<p>VideoEditor is an LGPL v2.1 or later, touch-friendly video editing application built upon" +
                  " GStreamer and GStreamer Editing Services, sponsored by Collabora Ltd.</p>" +
                  "<p>It aims to support all basic video editing functionality - add/remove clips, select to" +
                  " drag and drop a clip or trim its start/end points, double tap to zoom to the clip," +
                  " pinch to zoom in/out, select between 4:3 VGA, 16:9 WVGA and HD 720p output formats" +
                  " and then export your new video!</p>"
        }
        Item {
            anchors.top: mainText.bottom
            anchors.left: parent.left
            anchors.leftMargin: 32
            anchors.right: parent.right
            anchors.rightMargin: 32
            Text {
                id: ccText
                anchors.right: ccLogo.left
                anchors.rightMargin: 8
                font.pixelSize: 18
                color: "white"
                smooth: true
                wrapMode: Text.WordWrap
                text: "VideoEditor icons by Jaakko Roppola, licensed under"
            }
            Image {
                id: ccLogo
                anchors.verticalCenter: ccText.verticalCenter
                anchors.right: parent.right
                source: "ccbysa3.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        Qt.openUrlExternally("http://creativecommons.org/licenses/by-sa/3.0/");
                    }
                }
            }
        }

        Row {
            id: logos
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 100

            Item {
                width: parent.width / 3
                height: parent.height
                anchors.margins: 4
                Rectangle {
                    id: collaboraLogoBackground
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: logo.height + 16
                    color: "white"
                    radius: 4
                    Image {
                        id: logo
                        width: parent.width - 16
                        anchors.centerIn: parent
                        source: "collabora-logo-small.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally("http://www.collabora.com/");
                        }
                    }
                }
            }
            Item {
                width: parent.width / 3
                height: parent.height
                anchors.margins: 4
                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    source: "VideoEditor256.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally("http://github.com/superdump/Video-Editor/");
                        }
                    }
                }
            }
            Item {
                width: parent.width / 3
                height: parent.height
                anchors.margins: 4
                Rectangle {
                    id: gstreamerLogoBackground
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: gstLogo.height + 16
                    color: "white"
                    radius: 4
                    Image {
                        id: gstLogo
                        width: parent.width - 16
                        anchors.centerIn: parent
                        source: "gstreamer-logo.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally("http://gstreamer.freedesktop.org/");
                        }
                    }
                }
            }
        }
    }

    ToolBar {
        id: toolBar
        anchors.bottom: parent.bottom
        tools: ToolBarLayout {
            ToolIcon {
                id: cancelButton
                iconId: "toolbar-back"
                onClicked: {
                    pageStack.pop(this)
                }
            }
        }
    }
}
