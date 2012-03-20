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
                listScale.listScale = list.width * timeline.zoomProportion / videoeditor.getObjDuration(0);
                listScale.scale = 1.0;
            }
        }

        onProgressChanged: {
            console.debug("Position: " + position + " / " + duration)
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

    ButtonStyle {
        id: buttonStyle
        inverted: true
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
            platformStyle: buttonStyle
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
                if (videoeditor.isRendering())
                    videoeditor.cancelRender();
                if (videoeditor.isPlaying)
                    videoeditor.pause();
                videoeditor.removeAll();
            }
            platformStyle: buttonStyle
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
            platformStyle: buttonStyle
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
            platformStyle: buttonStyle
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
            platformStyle: buttonStyle
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
            platformStyle: buttonStyle
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
            property double listScale: 1.0
            property double minimumScale: minUsableWidthPx / maxGranularityNS
            property double maximumScale: minUsableWidthPx / minGranularityNS
        }

        ListView {
            id: list
            model: videoeditor

            width: parent.width - 32
            height: parent.height - 32
            anchors.margins: 16
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: 16

            orientation: ListView.Horizontal

            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                id: delegateButton
                width: duration * listScale.listScale * listScale.scale
                height: list.height

                Rectangle {
                    id: highlight
                    anchors.fill: parent
                    color: delegateButton.ListView.isCurrentItem ? "white" : "transparent"
                    Image {
                        source: "image://videoeditorimageprovider/" + uri + "#1000%"
                        anchors.fill: parent
                        anchors.margins: 2
                        asynchronous: true
                    }
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    font.pixelSize: 20
                    width: parent.width
                    clip: true
                    color: "#FFFFFF"
                    text: fileName
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    property int positionStarted: 0
                    property int positionEnded: 0
                    property int positionsMoved: Math.floor((positionEnded - positionStarted)/delegateButton.width)
                    property int newPosition: index + positionsMoved
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
                        drag.maximumX = (timelineBar.width - delegateButton.width - 1 + list.contentX);
                        drag.minimumX = 0;
                    }
                    onPositionChanged: {
                        positionEnded = delegateButton.x;
                    }
                    onReleased: {
                        if (Math.abs(positionsMoved) < 1 && held == true) {
                            delegateButton.x = positionStarted;
                            delegateButton.opacity = 1;
                            list.interactive = true;
                            dragArea.drag.target = null;
                            held = false;
                        } else {
                            if (held == true) {
                                if (newPosition < 1) {
                                    delegateButton.z = 1;
                                    videoeditor.move(index,0);
                                    list.currentIndex = 0;
                                    delegateButton.opacity = 1;
                                    list.interactive = true;
                                    dragArea.drag.target = null;
                                    held = false;
                                } else if (newPosition > list.count - 1) {
                                    delegateButton.z = 1;
                                    videoeditor.move(index, list.count - 1);
                                    list.currentIndex = list.count - 1;
                                    delegateButton.opacity = 1;
                                    list.interactive = true;
                                    dragArea.drag.target = null;
                                    held = false;
                                }
                                else {
                                    delegateButton.z = 1;
                                    videoeditor.move(index,newPosition);
                                    list.currentIndex = newPosition;
                                    delegateButton.opacity = 1;
                                    list.interactive = true;
                                    dragArea.drag.target = null;
                                    held = false;
                                }
                            }
                        }
                    }
                    onClicked: {
                        console.log("Drag " + index + " deselected")
                        list.currentIndex = -1;
                    }
                    onDoubleClicked: {
                        listScale.listScale = list.width * timeline.zoomProportion / duration;
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
                        console.log("Item " + index + " selected")
                        list.currentIndex = index;
                    }
                    onDoubleClicked: {
                        listScale.listScale = list.width * timeline.zoomProportion / duration;
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

            Rectangle {
                id: playhead
                width: 2
                height: timelineBar.height
                anchors.verticalCenter: parent.verticalCenter
                color: "red"
                z: 1000
                x: parent.contentWidth * (videoeditor.position / videoeditor.duration)
            }

            // x: parent.width / 2 - flickable.contentWidth * flickable.visibleArea.xPosition / (1.0 - flickable.visibleArea.widthRatio)
        }
    }
}
