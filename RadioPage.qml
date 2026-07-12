import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: radioPage
    focus: true

    property int vol: carControlManager ? carControlManager.radioVolume : 0
    property bool muted: carControlManager ? carControlManager.radioMuted : true
    property int src: carControlManager ? carControlManager.radioSource : 0
    property int bass: carControlManager ? carControlManager.radioBass : 16
    property int treble: carControlManager ? carControlManager.radioTreble : 16
    property int mid: 16
    property int fader: carControlManager ? carControlManager.radioFader : 0
    property bool loudness: carControlManager ? carControlManager.radioLoudness : false

    Connections {
        target: carControlManager
        function onRadioChanged() {
            vol = carControlManager.radioVolume
            muted = carControlManager.radioMuted
            src = carControlManager.radioSource
            bass = carControlManager.radioBass
            treble = carControlManager.radioTreble
            mid = Math.max(treble - 3, 0)
            fader = carControlManager.radioFader
            loudness = carControlManager.radioLoudness
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "RADIO CONTROLS"
            color: themeManager.textPrimary
            font.pixelSize: 24; font.bold: true
        }

        // Volume section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            color: themeManager.bgCard
            radius: 10
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 10
                Text { text: "VOLUME"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true; spacing: 12
                    Button {
                        Layout.preferredWidth: 50; Layout.preferredHeight: 50
                        text: "-"; font.pixelSize: 20
                        onClicked: carControlManager.setRadioVolume(vol - 5)
                    }
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 4
                        Text {
                            text: muted ? "MUTED" : vol + " dB"
                            color: muted ? themeManager.statusRed : themeManager.carBlue
                            font.pixelSize: 28; font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }
                        Slider {
                            Layout.fillWidth: true
                            from: -31; to: 20; value: vol; stepSize: 1
                            enabled: !muted
                            onMoved: carControlManager.setRadioVolume(value)
                        }
                    }
                    Button {
                        Layout.preferredWidth: 50; Layout.preferredHeight: 50
                        text: "+"; font.pixelSize: 20
                        onClicked: carControlManager.setRadioVolume(vol + 5)
                    }
                    Button {
                        Layout.preferredWidth: 70; Layout.preferredHeight: 50
                        text: muted ? "UNMUTE" : "MUTE"
                        highlighted: muted
                        palette.button: muted ? themeManager.carOrange : themeManager.bgPanel
                        onClicked: carControlManager.setRadioMuted(!muted)
                    }
                }
            }
        }

        // Source section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: themeManager.bgCard
            radius: 10
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 8
                Text { text: "INPUT SOURCE"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Repeater {
                        model: [
                            { label: "SRC 1", idx: 0 },
                            { label: "SRC 2", idx: 1 },
                            { label: "SRC 3", idx: 2 },
                            { label: "SRC 4", idx: 3 }
                        ]
                        Button {
                            Layout.fillWidth: true; Layout.preferredHeight: 36
                            text: modelData.label; font.pixelSize: 12
                            highlighted: src === modelData.idx
                            palette.button: src === modelData.idx ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: carControlManager.setRadioSource(modelData.idx)
                        }
                    }
                }
            }
        }

        // Bass + Mid + Treble section
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100
                color: themeManager.bgCard; radius: 10
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 6
                    Text { text: "BASS"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                    Text {
                        text: {
                            var v = bass - 16
                            if (v > 0) return "+" + v
                            else if (v < 0) return "" + v
                            return "0"
                        }
                        color: themeManager.carBlue; font.pixelSize: 24; font.bold: true
                        horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: 0; to: 31; value: bass; stepSize: 1
                        onMoved: carControlManager.setRadioBass(value)
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100
                color: themeManager.bgCard; radius: 10
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 6
                    Text { text: "MID"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                    Text {
                        text: {
                            var v = mid - 16
                            if (v > 0) return "+" + v
                            else if (v < 0) return "" + v
                            return "0"
                        }
                        color: themeManager.carOrange; font.pixelSize: 24; font.bold: true
                        horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
                    }
                    Slider {
                        id: midSlider
                        Layout.fillWidth: true
                        from: 0; to: 31; value: mid; stepSize: 1
                        onMoved: {
                            mid = value
                            var newTreble = Math.min(mid + 3, 31)
                            treble = newTreble
                            carControlManager.setRadioTreble(newTreble)
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100
                color: themeManager.bgCard; radius: 10
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 6
                    Text { text: "TREBLE"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                    Text {
                        text: {
                            var v = treble - 16
                            if (v > 0) return "+" + v
                            else if (v < 0) return "" + v
                            return "0"
                        }
                        color: themeManager.carBlue; font.pixelSize: 24; font.bold: true
                        horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
                    }
                    Slider {
                        id: trebleSlider
                        Layout.fillWidth: true
                        from: 0; to: 31; value: treble; stepSize: 1
                        onMoved: {
                            treble = value
                            mid = Math.max(treble - 3, 0)
                            carControlManager.setRadioTreble(value)
                        }
                    }
                }
            }
        }

        // Fader + Loudness section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: themeManager.bgCard
            radius: 10
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: "FADER"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Text { text: "Rear"; color: themeManager.textSecondary; font.pixelSize: 12 }
                    Slider {
                        Layout.fillWidth: true
                        from: -15; to: 15; value: fader; stepSize: 1
                        onMoved: carControlManager.setRadioFader(value)
                    }
                    Text { text: "Front"; color: themeManager.textSecondary; font.pixelSize: 12 }
                }
                Text {
                    text: fader === 0 ? "Center" : (fader < 0 ? (-fader) + " dB Rear" : fader + " dB Front")
                    color: themeManager.carBlue; font.pixelSize: 14; font.bold: true
                    horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
                }
            }
        }

        Button {
            Layout.fillWidth: true; Layout.preferredHeight: 40
            text: loudness ? "LOUDNESS: ON" : "LOUDNESS: OFF"
            font.pixelSize: 12
            highlighted: loudness
            palette.button: loudness ? themeManager.carOrange : themeManager.bgPanel
            onClicked: carControlManager.setRadioLoudness(!loudness)
        }

        // Connection status
        Text {
            text: carControlManager && carControlManager.connected ? "Body Controller: CONNECTED" : "Body Controller: OFFLINE"
            color: carControlManager && carControlManager.connected ? themeManager.statusGreen : themeManager.statusRed
            font.pixelSize: 12
        }
    }
}
