import QtQuick 2.0

Rectangle {
    id: root

    property string text: "button"
    property bool   checked: false

    readonly property alias progress: privateScope.progress
    property int delay: 1000

    signal activated

    color: "gray"
    width: 100; height: 25

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressedChanged: {
            if (checked)  {
                if (pressed) {                    
                    checked = false;
                }
            } else {
                privateScope.progress = pressed ? 1 : 0;
            }
        }
    }

    onProgressChanged: {
        if (progress == 1.0) {
            checked = true;
            activated();
        }
    }

    onCheckedChanged: {
        if (!checked)
            privateScope.progress = 0;
        state = checked ? "activated" : "normal"
    }

    // Прямоугольник, индицирующий прогресс
    Rectangle {
        id: progressRectangle
        color: "#1d5c97"
        height: parent.height
        width:  root.progress * parent.width
    }

    Text {
        anchors.fill: parent;
        text: root.text
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }

    QtObject {
        id: privateScope

        property real progress: 0.0
        readonly property bool progressing: mouseArea.pressed && progress < 1

        Behavior on progress {
            NumberAnimation {
                duration: Math.max(0, (privateScope.progressing ? progress : 1 - progress) * delay)
            }
        }
    }

    state: "normal"
    states: [
        State {
            name: "normal"
            PropertyChanges {
                target: progressRectangle
                color: "#1d5c97"
            }
        },
        State {
            name: "activated"
            PropertyChanges {
                target: progressRectangle
            }
        }
    ]

    transitions: [
        Transition {
            from: "normal"
            to: "activated"
            SequentialAnimation {
                loops: Animation.Infinite

                ColorAnimation {
                    target: progressRectangle
                    from: "gray"
                    to: "red"
                    duration: 560
                    easing.type: Easing.InOutSine
                }

                ColorAnimation {
                    target: progressRectangle
                    from: "red"
                    to: "gray"
                    duration: 560
                    easing.type: Easing.InOutSine
                }
            }
        }
    ]
}
