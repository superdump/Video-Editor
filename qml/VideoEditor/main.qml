/* VideoEditor QML main
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


PageStackWindow {
    id: appWindow
    showStatusBar: false
    showToolBar: false

    initialPage: timeline

    Timeline {
        orientationLock: PageOrientation.LockLandscape
        id: timeline
    }

    VideoGallery {
        orientationLock: PageOrientation.LockLandscape
        id: gallery
    }

    Component.onCompleted: {
        theme.inverted = true;
    }
}
