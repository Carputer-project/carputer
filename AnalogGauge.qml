import QtQuick 2.15

Item {
    id: root
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property real warnValue: 0.8
    property real dangerValue: 0.9
    property alias gaugeColor: root._gaugeColor
    property alias warnColor: root._warnColor
    property alias dangerColor: root._dangerColor
    property alias bgColor: root._bgColor
    property alias tickColor: root._tickColor
    property alias textColor: root._textColor
    property alias needleColor: root._needleColor
    property color _gaugeColor: themeManager.gaugeColor
    property color _warnColor: themeManager.warnColor
    property color _dangerColor: themeManager.dangerColor
    property color _bgColor: themeManager.bgCard
    property color _tickColor: themeManager.tickColor
    property color _textColor: themeManager.gaugeTextColor
    property color _needleColor: themeManager.needleColor
    property int majorTicks: 5
    property int minorTicks: 4
    property real startAngle: 135
    property real endAngle: 405
    property bool showValue: true
    property bool showLabel: true
    property bool showNeedle: true
    property string label: ""
    property int fontSize: 14
    property real thickness: 0.15
    property real smoothDuration: 400
    width: 200
    height: 200

    property real _smoothValue: value
    Behavior on _smoothValue {
        NumberAnimation { duration: root.smoothDuration; easing.type: Easing.OutCubic }
    }
    onValueChanged: _smoothValue = value

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            var cx = width / 2
            var cy = height / 2
            var radius = Math.min(width, height) / 2 - 10
            var innerRadius = radius * (1 - thickness)
            ctx.clearRect(0, 0, width, height)

            var range = maxValue - minValue
            var fraction = range > 0 ? Math.max(0, Math.min(1, (_smoothValue - minValue) / range)) : 0
            var currentAngle = startAngle + fraction * (endAngle - startAngle)

            ctx.beginPath()
            ctx.arc(cx, cy, radius, startAngle * Math.PI / 180, endAngle * Math.PI / 180)
            ctx.arc(cx, cy, innerRadius, endAngle * Math.PI / 180, startAngle * Math.PI / 180, true)
            ctx.closePath()
            ctx.fillStyle = bgColor
            ctx.fill()

            if (fraction > 0) {
                var warnAngle = startAngle + warnValue * (endAngle - startAngle)
                var dangerAngle = startAngle + dangerValue * (endAngle - startAngle)
                var safeEnd = Math.min(currentAngle, warnAngle)
                if (safeEnd > startAngle) {
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, startAngle * Math.PI / 180, safeEnd * Math.PI / 180)
                    ctx.arc(cx, cy, innerRadius, safeEnd * Math.PI / 180, startAngle * Math.PI / 180, true)
                    ctx.closePath()
                    ctx.fillStyle = gaugeColor
                    ctx.fill()
                }
                if (currentAngle > warnAngle) {
                    var warnEnd = Math.min(currentAngle, dangerAngle)
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, warnAngle * Math.PI / 180, warnEnd * Math.PI / 180)
                    ctx.arc(cx, cy, innerRadius, warnEnd * Math.PI / 180, warnAngle * Math.PI / 180, true)
                    ctx.closePath()
                    ctx.fillStyle = warnColor
                    ctx.fill()
                }
                if (currentAngle > dangerAngle) {
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, dangerAngle * Math.PI / 180, currentAngle * Math.PI / 180)
                    ctx.arc(cx, cy, innerRadius, currentAngle * Math.PI / 180, dangerAngle * Math.PI / 180, true)
                    ctx.closePath()
                    ctx.fillStyle = dangerColor
                    ctx.fill()
                }
            }

            var tickRange = endAngle - startAngle
            var totalTicks = majorTicks * minorTicks
            for (var i = 0; i <= totalTicks; i++) {
                var angle = startAngle + (i / totalTicks) * tickRange
                var rad = angle * Math.PI / 180
                var isMajor = i % minorTicks === 0
                var tickLen = isMajor ? 8 : 4
                var tickRadius = isMajor ? innerRadius - 2 : innerRadius - 1
                ctx.beginPath()
                ctx.moveTo(cx + tickRadius * Math.cos(rad), cy + tickRadius * Math.sin(rad))
                ctx.lineTo(cx + (tickRadius - tickLen) * Math.cos(rad), cy + (tickRadius - tickLen) * Math.sin(rad))
                ctx.strokeStyle = isMajor ? textColor : tickColor
                ctx.lineWidth = isMajor ? 2 : 1
                ctx.stroke()
                if (isMajor) {
                    var lr = tickRadius - 14
                    ctx.fillStyle = textColor
                    ctx.font = "bold " + (fontSize - 3) + "px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(Math.round(minValue + (i / totalTicks) * range), cx + lr * Math.cos(rad), cy + lr * Math.sin(rad))
                }
            }

            if (showNeedle && fraction >= 0) {
                var needleAngleRad = (startAngle + fraction * (endAngle - startAngle)) * Math.PI / 180
                var needleLength = radius - 12
                var needleBase = 10
                var needleWidth = 3
                ctx.save()
                ctx.translate(cx, cy)
                ctx.rotate(needleAngleRad)
                ctx.shadowColor = needleColor
                ctx.shadowBlur = 8
                ctx.beginPath()
                ctx.moveTo(0, -needleLength)
                ctx.lineTo(-needleWidth, needleBase)
                ctx.lineTo(0, needleBase - 4)
                ctx.lineTo(needleWidth, needleBase)
                ctx.closePath()
                ctx.fillStyle = needleColor
                ctx.fill()
                ctx.shadowBlur = 0
                ctx.beginPath()
                ctx.arc(0, 0, 5, 0, Math.PI * 2)
                ctx.fillStyle = bgColor
                ctx.fill()
                ctx.strokeStyle = needleColor
                ctx.lineWidth = 2
                ctx.stroke()
                ctx.restore()
            }

            if (showValue) {
                var vColor = fraction >= dangerValue ? dangerColor : (fraction >= warnValue ? warnColor : gaugeColor)
                ctx.fillStyle = vColor
                ctx.font = "bold " + fontSize + "px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                ctx.fillText(Math.round(value), cx, cy - (showLabel && label !== "" ? fontSize * 0.4 : 0))
            }

            if (showLabel && label !== "") {
                ctx.fillStyle = textColor
                ctx.font = (fontSize - 4) + "px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                ctx.fillText(label, cx, cy + fontSize * 0.8)
            }
        }
        Connections {
            target: root
            function on_SmoothValueChanged() { canvas.requestPaint() }
            function onMinValueChanged() { canvas.requestPaint() }
            function onMaxValueChanged() { canvas.requestPaint() }
            function onWarnValueChanged() { canvas.requestPaint() }
            function onDangerValueChanged() { canvas.requestPaint() }
            function onStartAngleChanged() { canvas.requestPaint() }
            function onEndAngleChanged() { canvas.requestPaint() }
        }
    }
}
