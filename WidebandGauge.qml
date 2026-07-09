import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    implicitWidth: 160
    implicitHeight: 120

    property double afr: sensorManager ? sensorManager.wboAFR : 14.7
    property double lambda: sensorManager ? sensorManager.wboLambda : 1.0
    property double targetAfr: sensorManager ? sensorManager.targetAFR : 14.7
    property double correction: sensorManager ? sensorManager.fuelCorrection : 0.0
    property bool piggybackActive: correction > 0.01 || correction < -0.01

    /* Colors based on AFR range - uses engine profile */
    property double _dangerRich: engineProfile ? engineProfile.afrDangerRich : 10.0
    property double _dangerLean: engineProfile ? engineProfile.afrDangerLean : 16.0
    property double _cautionRich: _dangerRich + 1.5
    property double _cautionLean: _dangerLean - 1.5
    property double _mildRich: _dangerRich + 3.0
    property double _mildLean: _dangerLean - 1.0

    function afrColor(a) {
        if (a < _dangerRich || a > _dangerLean + 2.0) return "#FF4444"
        if (a < _cautionRich || a > _cautionLean) return "#FFAA00"
        if (a < _mildRich || a > _mildLean) return "#FFDD44"
        return "#44DD44"
    }

    Column {
        anchors.centerIn: parent
        spacing: 2

        /* AFR value */
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: afr.toFixed(1)
            font.pixelSize: 36
            font.bold: true
            font.family: "monospace"
            color: afrColor(afr)
            style: Text.Raised
            styleColor: "#88000000"
        }

        /* Lambda below */
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "\u03BB " + lambda.toFixed(3)
            font.pixelSize: 14
            font.family: "monospace"
            color: "#AAAAAA"
        }

        /* Fuel correction bar */
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 120
            height: 8
            radius: 4
            color: "#333333"
            clip: true

            Rectangle {
                x: 60 + (correction / 0.3) * 56  /* ±0.3 range centered */
                y: 0
                width: 4
                height: 8
                radius: 2
                color: piggybackActive ? "#FFAA00" : "#555555"
            }

            /* Center line */
            Rectangle {
                x: 59
                y: 0
                width: 2
                height: 8
                color: "#666666"
            }
        }

        /* Label */
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "AFR" + (piggybackActive ? " \u2699" : "")
            font.pixelSize: 10
            color: piggybackActive ? "#FFAA00" : "#888888"
        }
    }
}
