import QtQuick 2.0

Rectangle {
    id: root

    property string text: "button"
    property bool   checked: false

    signal clicked

    color: "gray"
    width: 100; height: 25

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            root.clicked();
        }
    }

    onCheckedChanged: {
        state = checked ? "checked" : ""
    }


    Text {
        anchors.fill: parent;
        text: root.text
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }

    states: [
        State {
            name: "checked"
            PropertyChanges {
                target: root
                color: "#1d5c97"
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "checked"

            ColorAnimation {
                target: root
                duration: 300
                easing.type: Easing.InOutSine
            }
        },
        Transition {
            from: "checked"
            to: ""

            ColorAnimation {
                target: root
                duration: 300
                easing.type: Easing.InOutSine
            }
        }
    ]
}
