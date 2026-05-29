import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 400
    height: 700
    visible: true
    title: "MeshTalk"
    color: "#0f0f0f"

    property string selectedPeer: ""

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: nicknameScreen
    }

    // ─── Screen 1: Nickname ──────────────────────────────────────────────────
    Component {
        id: nicknameScreen

        Rectangle {
            color: "#0f0f0f"

            Column {
                anchors.centerIn: parent
                spacing: 20
                width: 280

                Text {
                    text: "MeshTalk"
                    font.pixelSize: 36
                    font.bold: true
                    font.family: "Consolas"
                    color: "#00e5ff"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "OFFLINE MESH MESSENGER"
                    font.pixelSize: 13
                    font.family: "Consolas"
                    color: "#555555"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle { height: 16; color: "transparent"; width: 1 }

                TextField {
                    id: nickInput
                    width: 280
                    height: 48
                    font.pixelSize: 15
                    font.family: "Consolas"
                    color: "#ffffff"
                    placeholderText: "Enter Your Nickname"
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter

                    onTextChanged: {
                        var pos = cursorPosition
                        var words = text.split(" ")
                        var result = words.map(function(word) {
                            if (word.length === 0) return ""
                            return word.charAt(0).toUpperCase() + word.slice(1)
                        }).join(" ")
                        if (text !== result) {
                            text = result
                            cursorPosition = pos
                        }
                    }

                    onAccepted: {
                        if (nickInput.text.trim() === "") return
                        backend.setNickname(nickInput.text.trim())
                        stack.push(chatScreen)
                    }

                    background: Rectangle {
                        radius: 8
                        color: "#1a1a1a"
                        border.color: nickInput.activeFocus ? "#00e5ff" : "#2a2a2a"
                        border.width: 1
                        anchors.fill: parent
                    }
                }

                Rectangle {
                    width: 280
                    height: 48
                    radius: 8
                    color: nickInput.text.trim() !== "" ? "#00e5ff" : "#1a1a1a"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "Join Mesh"
                        font.pixelSize: 15
                        font.bold: true
                        font.family: "Consolas"
                        color: nickInput.text.trim() !== "" ? "#0f0f0f" : "#333333"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (nickInput.text.trim() === "") return
                            backend.setNickname(nickInput.text.trim())
                            stack.push(chatScreen)
                        }
                    }
                }
            }
        }
    }

    // ─── Screen 2: Chat ──────────────────────────────────────────────────────
    Component {
        id: chatScreen

        Rectangle {
            color: "#0f0f0f"

            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: backend.checkInbox()
            }

            Connections {
                target: root
                function onSelectedPeerChanged() {
                    if (root.selectedPeer !== "") {
                        toInput.text = root.selectedPeer
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Top bar
                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    color: "#141414"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16

                        Text {
                            text: "⬡ " + backend.myNickname
                            font.pixelSize: 16
                            font.bold: true
                            font.family: "Consolas"
                            color: "#00e5ff"
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            width: 28
                            height: 28
                            radius: 14
                            color: backend.peers.length > 0 ? "#00e5ff" : "#2a2a2a"

                            Text {
                                anchors.centerIn: parent
                                text: backend.peers.length
                                font.pixelSize: 12
                                font.bold: true
                                font.family: "Consolas"
                                color: "#0f0f0f"
                            }
                        }

                        Rectangle {
                            width: 80
                            height: 32
                            radius: 6
                            color: "#1a1a1a"
                            border.color: "#2a2a2a"

                            Text {
                                anchors.centerIn: parent
                                text: "Peers"
                                font.pixelSize: 13
                                font.family: "Consolas"
                                color: "#aaaaaa"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: stack.push(peersScreen)
                            }
                        }
                    }
                }

                // Message list
                ListView {
                    id: messageList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 6
                    model: backend.messages
                    contentY: contentHeight > height ? contentHeight - height : 0

                    delegate: Rectangle {
                        width: messageList.width - 24
                        x: 12
                        height: msgText.implicitHeight + 20
                        radius: 8
                        color: modelData.startsWith("You →") ? "#003d4d" : "#1a1a1a"

                        Text {
                            id: msgText
                            text: modelData
                            font.pixelSize: 13
                            font.family: "Consolas"
                            color: "#dddddd"
                            wrapMode: Text.Wrap
                            width: parent.width - 24
                            anchors.centerIn: parent
                        }
                    }
                }

                // Input bar
                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    color: "#141414"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        // ── Peer selector button ──────────────────────────
                        Rectangle {
                            id: peerSelectorBtn
                            width: 110
                            height: 36
                            radius: 6
                            color: toInput.text !== "" ? "#002a33" : "#1a1a1a"
                            border.color: toInput.text !== "" ? "#00e5ff" : "#2a2a2a"
                            border.width: 1

                            // Hidden TextField keeps the existing backend wiring intact
                            TextField {
                                id: toInput
                                visible: false
                                text: ""
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 6
                                spacing: 4

                                Text {
                                    id: selectorLabel
                                    Layout.fillWidth: true
                                    text: toInput.text !== "" ? toInput.text : "To"
                                    font.pixelSize: 13
                                    font.family: "Consolas"
                                    color: toInput.text !== "" ? "#00e5ff" : "#555555"
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: "▾"
                                    font.pixelSize: 11
                                    color: toInput.text !== "" ? "#00e5ff" : "#444444"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: peerPopup.open()
                            }

                            // ── Dropdown popup ────────────────────────────
                            Popup {
                                id: peerPopup
                                // Anchor above the button
                                y: -(implicitHeight + 6)
                                x: 0
                                width: 160
                                padding: 0
                                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                                background: Rectangle {
                                    color: "#1a1a1a"
                                    border.color: "#2a2a2a"
                                    border.width: 1
                                    radius: 8
                                }

                                contentItem: Column {
                                    spacing: 0

                                    // "All" option
                                    Rectangle {
                                        width: 160
                                        height: 40
                                        color: allArea.containsMouse ? "#002a33" : "transparent"
                                        radius: 8

                                        Behavior on color { ColorAnimation { duration: 80 } }

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 12
                                            spacing: 8

                                            Rectangle {
                                                width: 8; height: 8; radius: 4
                                                color: "#00e5ff"
                                            }

                                            Text {
                                                text: "All"


                                                font.pixelSize: 13
                                                font.bold: true
                                                font.family: "Consolas"
                                                color: "#00e5ff"
                                                Layout.fillWidth: true
                                            }

                                            Text {
                                                text: "broadcast"
                                                font.pixelSize: 10
                                                font.family: "Consolas"
                                                color: "#444444"
                                            }
                                        }

                                        MouseArea {
                                            id: allArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                toInput.text = "All"
                                                peerPopup.close()
                                            }
                                        }
                                    }

                                    // Divider
                                    Rectangle {
                                        width: 140
                                        height: 1
                                        color: "#2a2a2a"
                                        x: 10
                                        visible: backend.peers.length > 0
                                    }

                                    // Per-peer rows
                                    Repeater {
                                        model: backend.peers

                                        Rectangle {
                                            width: 160
                                            height: 40
                                            color: peerItemArea.containsMouse ? "#002a33" : "transparent"
                                            radius: 8

                                            Behavior on color { ColorAnimation { duration: 80 } }

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 12
                                                spacing: 8

                                                Rectangle {
                                                    width: 8; height: 8; radius: 4
                                                    color: "#00ff88"
                                                }

                                                Text {
                                                    text: modelData
                                                    font.pixelSize: 13
                                                    font.family: "Consolas"
                                                    color: "#ffffff"
                                                    Layout.fillWidth: true
                                                    elide: Text.ElideRight
                                                }
                                            }

                                            MouseArea {
                                                id: peerItemArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                onClicked: {
                                                    toInput.text = modelData
                                                    peerPopup.close()
                                                }
                                            }
                                        }
                                    }

                                    // Empty state inside popup
                                    Rectangle {
                                        width: 160
                                        height: 36
                                        color: "transparent"
                                        visible: backend.peers.length === 0

                                        Text {
                                            anchors.centerIn: parent
                                            text: "No peers yet"
                                            font.pixelSize: 11
                                            font.family: "Consolas"
                                            color: "#333333"
                                        }
                                    }
                                }
                            }
                        }

                        TextField {
                            id: msgInput
                            Layout.fillWidth: true
                            height: 36
                            font.pixelSize: 13
                            font.family: "Consolas"
                            color: "#ffffff"
                            placeholderText: "Enter Your Message"
                            onAccepted: {
                                if (toInput.text.trim() === "" || msgInput.text.trim() === "") return
                                backend.sendMessage(toInput.text.trim(), msgInput.text.trim())
                                msgInput.text = ""
                            }

                            background: Rectangle {
                                radius: 6
                                color: "#1a1a1a"
                                border.color: msgInput.activeFocus ? "#00e5ff" : "#2a2a2a"
                            }
                        }

                        Rectangle {
                            width: 44
                            height: 36
                            radius: 6
                            color: (toInput.text !== "" && msgInput.text !== "") ? "#00e5ff" : "#1a1a1a"

                            Behavior on color { ColorAnimation { duration: 100 } }

                            Text {
                                anchors.centerIn: parent
                                text: "↑"
                                font.pixelSize: 18
                                font.bold: true
                                font.family: "Consolas"
                                color: (toInput.text !== "" && msgInput.text !== "") ? "#0f0f0f" : "#333333"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (toInput.text.trim() === "" || msgInput.text.trim() === "") return
                                    backend.sendMessage(toInput.text.trim(), msgInput.text.trim())
                                    msgInput.text = ""
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ─── Screen 3: Peers ─────────────────────────────────────────────────────
    Component {
        id: peersScreen

        Rectangle {
            color: "#0f0f0f"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    color: "#141414"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16

                        Rectangle {
                            width: 60
                            height: 32
                            radius: 6
                            color: "#1a1a1a"
                            border.color: "#2a2a2a"

                            Text {
                                anchors.centerIn: parent
                                text: "← Back"
                                font.pixelSize: 13
                                font.family: "Consolas"
                                color: "#aaaaaa"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: stack.pop()
                            }
                        }

                        Text {
                            text: "Nearby Peers (" + backend.peers.length + ")"
                            font.pixelSize: 16
                            font.bold: true
                            font.family: "Consolas"
                            color: "#ffffff"
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                // Empty state
                Item {
                    visible: backend.peers.length === 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        anchors.centerIn: parent
                        spacing: 12

                        Text {
                            text: "○"
                            font.pixelSize: 48
                            font.family: "Consolas"
                            color: "#222222"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "No peers nearby"
                            font.pixelSize: 16
                            font.family: "Consolas"
                            color: "#333333"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "Waiting for others to join the mesh..."
                            font.pixelSize: 12
                            font.family: "Consolas"
                            color: "#2a2a2a"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                // Peers list
                ListView {
                    visible: backend.peers.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 1
                    model: backend.peers

                    delegate: Rectangle {
                        width: parent.width
                        height: 64
                        color: peerArea.containsMouse ? "#1a1a1a" : "#141414"

                        Behavior on color { ColorAnimation { duration: 80 } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: "#00e5ff"
                            }

                            Column {
                                Layout.fillWidth: true
                                spacing: 3

                                Text {
                                    text: modelData
                                    font.pixelSize: 15
                                    font.bold: true
                                    font.family: "Consolas"
                                    color: "#ffffff"
                                }

                                Text {
                                    text: "tap to chat"
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                    color: "#444444"
                                }
                            }

                            Rectangle {
                                width: 70
                                height: 30
                                radius: 15
                                color: "#00e5ff"

                                Text {
                                    anchors.centerIn: parent
                                    text: "Chat →"
                                    font.pixelSize: 12
                                    font.bold: true
                                    font.family: "Consolas"
                                    color: "#0f0f0f"
                                }
                            }
                        }

                        MouseArea {
                            id: peerArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                root.selectedPeer = modelData
                                stack.pop()
                            }
                        }
                    }
                }
            }
        }
    }
}
