/* VideoEditor Timeline
 * Copyright (C) 2012 Thiago Sousa Santos <thiago.sousa.santos@collabora.co.uk>
 * Copyright (C) 2012 Robert Swain <robert.swain@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import VideoEditor 1.0

Page {
    id: timeline
    property int screenWidthPx: 854;
    property int screenHeightPx: 480;
    property double screenWidthMM: 86.3382727
    property double minUsableWidthMM: 10.0
    property double zoomProportion: 1.0/3.0
    property double minGranularityNS: 1000000000.0 / 30.0
    property double maxGranularityNS: 1000000000.0 * 3.0 * 60.0 * 60.0
    property double screenHDPMM: screenWidthPx / screenWidthMM
    property double minUsableWidthPx: minUsableWidthMM * screenHDPMM
    orientationLock: PageOrientation.LockLandscape
    width: 854
    height: 480

    VideoEditor {
        id: videoeditor

        winId: XWinId;

        onError: {
            console.debug("Error: " + message + " (" + debug + ")");
            progressDialog.close();
            messageTitleField.text = message + "\n(" + debug + ")";
            messageDialog.open();
        }

        onRenderComplete: {
            progressDialog.close();
            messageTitleField.text = "Export complete";
            messageDialog.open();
        }

        onDataChanged: {
            if (list.count == 1) {
                listScale.calculatedScale = list.width * timeline.zoomProportion / videoeditor.getObjDuration(0);
                listScale.scale = 1.0;
            }
        }

        onProgressChanged: {
            var curPos = position * listScale.currentScale;
            if (curPos < list.width / 2) {
                list.contentX = 0;
            } else if (curPos > list.listContentWidth - list.width / 2) {
                list.contentX = list.listContentWidth - list.width;
            } else {
                list.contentX = curPos - list.width / 2;
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }


    Item {
        id: preview
        property bool isPlaying: false
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
            text: "Video preview\nTap to play/pause"
            color: "white"
            font.bold: true
            font.pixelSize: 32
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (preview.isPlaying) {
                    console.log("Video preview: playing -> paused");
                    preview.isPlaying = false;
                    videoeditor.pause();
                    previewText.text = "Video preview\nTap to play/pause";
                } else {
                    console.log("Video preview: paused -> playing");
                    preview.isPlaying = true;
                    previewText.text = "";
                    videoeditor.play();
                }
            }
        }
    }


    QueryDialog {
        id: removeAllDialog
        acceptButtonText: "Clear"
        rejectButtonText: "Cancel"
        message: "Clear all clips?"
        onAccepted: {
            if (videoeditor.isRendering())
                videoeditor.cancelRender();
            if (videoeditor.isPlaying)
                videoeditor.pause();
            videoeditor.removeAll();
            close();
        }
    }
    QueryDialog {
        id: removeDialog
        acceptButtonText: "Remove"
        rejectButtonText: "Cancel"
        message: "Remove the selected clip?"
        onAccepted: {
            console.log("Removing item " + list.currentIndex);
            if (videoeditor.isRendering())
                videoeditor.cancelRender();
            if (videoeditor.isPlaying)
                videoeditor.pause();
            videoeditor.removeAt(list.currentIndex);
            close();
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
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
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
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: addMediaButton.bottom
            onClicked: {
                if (list.count > 0)
                    removeAllDialog.open();
            }
        }

        Button {
            id: removeButton
            text: "Remove"
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: list.count > 0
            onClicked: {
                if (list.count > 0)
                    removeDialog.open();
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
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        status: DialogStatus.Closed
        visualParent: timeline

        title: Text {
            id: messageTitleField
            width: parent.width
            color: "white";
            font.pixelSize: 28
            horizontalAlignment: Text.AlignHCenter
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
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top

            onClicked: {
                progressDialog.open()
                videoeditor.render()
            }
        }
        Button {
            id: settingsButton
            text: "Settings"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: exportButton.bottom

            onClicked: {
                var component = Qt.createComponent("ExportSettingsPage.qml")
                pageStack.push(component);
            }
        }
        Button {
            id: fitButton
            text: "Fit"
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: list.count > 0
            onClicked: {
                listScale.calculatedScale = list.width / videoeditor.duration;
                listScale.scale = 1.0;
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
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.fill: parent
        }

        PinchArea {
            id: timelinePinch
            anchors.fill: parent
            pinch {
                minimumScale: minUsableWidthPx / (list.width * zoomProportion)
                maximumScale: 1 / zoomProportion
                target: listScale
                dragAxis: Pinch.NoDrag
            }
        }

        Item {
            id: listScale
            property double calculatedScale: 1.0
            property double currentScale: calculatedScale * scale
            property double minimumScale: minUsableWidthPx / maxGranularityNS
            property double maximumScale: minUsableWidthPx / minGranularityNS
        }

        Item {
            id: scrollBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16

            height: 8

            property real position: list.contentX / list.listContentWidth
            property real pageSize: list.width < list.listContentWidth ? (list.width / list.listContentWidth) : 1

            visible: pageSize < 1

            Rectangle {
                anchors.fill: parent
                radius: (height / 2 - 1)
                color: "white"
                opacity: 0.2
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                x: scrollBar.position * (scrollBar.width - 2) + 1
                width: scrollBar.pageSize * (scrollBar.width - 2)
                height: parent.height - 2
                radius: height / 2 - 1
                color: "white"
                opacity: 0.6
            }
        }

        ListView {
            id: list
            model: videoeditor

            property double listContentWidth: listScale.currentScale * videoeditor.duration

            anchors.topMargin: 8
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.top: scrollBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                id: delegateButton
                width: duration * listScale.currentScale
                height: list.height

                Rectangle {
                    id: highlight
                    anchors.fill: parent
                    color: delegateButton.ListView.isCurrentItem ? "#bfffffff" : "#4dffffff"
                    Image {
                        id: image
                        source: "image://videoeditorimageprovider/" + uri + "#1000%"
                        anchors.centerIn: parent
                        width: parent.width - 4
                        height: parent.height - 4
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                        clip: true
                    }
                    Text {
                        anchors.centerIn: parent
                        text: image.status === Image.Loading ? "Loading..." : ""
                        font.pixelSize: 20
                    }
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.margins: 16
                    font.pixelSize: 20
                    width: parent.width - 32
                    clip: true
                    color: "white"
                    text: fileName
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    property int positionStarted: 0
                    property int positionEnded: 0
                    property bool held: false
                    drag.axis: Drag.XAxis
                    enabled: delegateButton.ListView.isCurrentItem
                    onPressed: {
                        delegateButton.z = 2;
                        positionStarted = delegateButton.x;
                        dragArea.drag.target = delegateButton;
                        delegateButton.opacity = 0.5;
                        list.interactive = false;
                        held = true;
                        drag.minimumX = 0;
                        drag.maximumX = list.contentX + list.width - delegateButton.width;
                    }
                    onPositionChanged: {
                        positionEnded = delegateButton.x;
                    }
                    onReleased: {
                        if (held == true) {
                            delegateButton.x = positionStarted;
                            delegateButton.z = 1;
                            delegateButton.opacity = 1;
                            var newPosition = list.indexAt(positionEnded, 0);
                            if (newPosition !== index) {
                                videoeditor.move(index, newPosition);
                                list.currentIndex = newPosition;
                            }
                            list.interactive = true;
                            dragArea.drag.target = null;
                            held = false;
                        }
                    }
                    onClicked: {
                        list.currentIndex = -1;
                    }
                    onDoubleClicked: {
                        listScale.calculatedScale = list.width * timeline.zoomProportion / duration;
                        listScale.scale = 1.0;
                    }
                }
                // This MouseArea is to workaround the button pressed state that causes
                // flicks and drags to momentarily flash the button as being selected
                MouseArea {
                    id: tapArea
                    anchors.fill: parent
                    enabled: delegateButton.ListView.isCurrentItem ? "false" : "true"
                    onClicked: {
                        list.currentIndex = index;
                    }
                    onDoubleClicked: {
                        listScale.calculatedScale = list.width * timeline.zoomProportion / duration;
                        listScale.scale = 1.0;
                    }
                }
            }

            Text {
                id: timelineText
                color: "#ffffff"
                text: list.count ? "" : "Add clips using the 'Add Media' button"
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 22
            }
            Item {
                id: playhead

                x: if(dragMouseArea.drag.active) { x } else { videoeditor.position * listScale.currentScale - list.contentX }
                z: 1000
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: dragMouseArea.drag.active || (list.count &&
                         (videoeditor.position * listScale.currentScale >= list.contentX) &&
                         (videoeditor.position * listScale.currentScale <= list.contentX + list.width))

                Rectangle {
                    id: seekBall

                    width: 15
                    height: 30
                    radius: 0
                    color: "red"
                    anchors.verticalCenter: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter

                }

                Rectangle {
                    width: 3

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter:  parent.horizontalCenter
                    color: "red"
                }

                MouseArea {
                    id: dragMouseArea

                    anchors.top: seekBall.top
                    anchors.horizontalCenter: seekBall.horizontalCenter
                    width: seekBall.width*4
                    height: seekBall.height*2

                    drag.axis: Drag.XAxis
                    drag.target: parent
                    drag.minimumX: 0
                    drag.maximumX: Math.min(list.width, list.contentWidth)

                    onPositionChanged: {
                        timelineMoveTimer.start();
                    }

                    onReleased: {
                        var pos = videoeditor.duration * (playhead.x + list.contentX) / list.listContentWidth;
                        console.debug("Sending seek to " + pos + " / " + videoeditor.duration);
                        videoeditor.seek(pos);
                    }
                }

                Timer {
                    id: timelineMoveTimer
                    interval: 200
                    repeat: true

                    onTriggered: {
                        if(dragMouseArea.drag.active) {
                            if(playhead.x >= list.x + list.width * 0.8 && list.listContentWidth - list.contentX > list.width) {
                                list.contentX += 5
                            } else if(playhead.x < list.x + list.width * 0.2 && list.contentX > 0) {
                                list.contentX -= 5
                            }
                        } else {
                            stop();
                        }
                    }
                }

            }
        }


        // The ListView does not clip items outside its own area. This looks a bit weird so
        // these two rectangles are to cover up those overflows.
        Rectangle {
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: list.left
        }

        Rectangle {
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: list.right
            anchors.right: parent.right
        }

    }
}
