/* VideoEditor
 * Copyright (C) 2012 Thiago Sousa Santos <thiago.sousa.santos@collabora.co.uk>
 * Copyright (C) 2012 Robert Swain <robert.swain@collabora.co.uk>
 *
 * gstpad.c: Pads for linking elements together
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
        highlightMoveDuration: 1
        spacing: 8

        model: DocumentGalleryModel {
            id: docGalModel
            rootType: DocumentGallery.Video
            properties: [ "url", "fileName" ]
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
            text: fileName
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
