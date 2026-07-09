import QtQuick 2.15

Item {
    id: root

    // ── Value & Range ──────────────────────────────────────────────────
    property real value:      0
    property real minValue:   0
    property real maxValue:   100

    // ── Zone thresholds (0.0 – 1.0 fractions of range) ────────────────
    property real warnValue:     0.75
    property real dangerValue:   0.90
    property real redlineStart:  0.85   // permanent redline marking (always drawn)

    // ── Arc geometry ───────────────────────────────────────────────────
    property real startAngle:  135
    property real endAngle:    405
    property real thickness:   0.13

    // ── Labels ─────────────────────────────────────────────────────────
    property string label:      ""
    property string unitLabel:  ""
    property bool   showValue:  true
    property bool   showLabel:  true
    property bool   showNeedle: true
    property int    fontSize:   14
    property int    majorTicks: 5
    property int    minorTicks: 4

    // ── Animation ──────────────────────────────────────────────────────
    property real smoothDuration: 350

    // ── Colors ─────────────────────────────────────────────────────────
    property color gaugeColor:   themeManager.gaugeColor
    property color warnColor:    themeManager.warnColor
    property color dangerColor:  themeManager.dangerColor
    property color needleColor:  themeManager.needleColor
    property color tickColor:    themeManager.tickColor
    property color textColor:    themeManager.gaugeTextColor
    property color bgColor:      themeManager.bgCard
    property color borderColor:  themeManager.gaugeBorderColor

    onGaugeColorChanged:   canvas.requestPaint()
    onWarnColorChanged:    canvas.requestPaint()
    onDangerColorChanged:  canvas.requestPaint()
    onNeedleColorChanged:  canvas.requestPaint()
    onTickColorChanged:    canvas.requestPaint()
    onTextColorChanged:    canvas.requestPaint()
    onBorderColorChanged:  canvas.requestPaint()

    width:  200
    height: 200

    Component.onCompleted: {
        canvas.requestPaint()
        _ready = true
    }
    onValueChanged: canvas.requestPaint()

    Behavior on value {
        enabled: _ready
        NumberAnimation { duration: smoothDuration; easing.type: Easing.OutQuart }
    }

    property bool _ready: false

    property bool p_sweepActive: false
    property real p_sweepValue: 0

    function runSweep() {
        if (p_sweepActive || !_ready) return
        p_sweepActive = true
        sweepAnimUp.from = minValue
        sweepAnimUp.to   = maxValue
        sweepAnimDown.from = maxValue
        sweepAnimDown.to   = minValue
        p_sweepValue = minValue
        sweepAnimUp.start()
    }

    NumberAnimation {
        id: sweepAnimUp
        target: root; property: "p_sweepValue"
        duration: 800; easing.type: Easing.OutQuart
        onStopped: { if (p_sweepActive) sweepAnimDown.start() }
    }

    NumberAnimation {
        id: sweepAnimDown
        target: root; property: "p_sweepValue"
        duration: 700; easing.type: Easing.InQuart
        onStopped: {
            if (p_sweepActive) {
                p_sweepActive = false
                canvas.requestPaint()
            }
        }
    }

    Timer {
        id: sweepPaintTimer
        interval: 16
        repeat: true
        running: p_sweepActive
        onTriggered: canvas.requestPaint()
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var cx      = width  / 2
            var cy      = height / 2
            var radius  = Math.min(width, height) / 2 - 6
            var innerR  = radius * (1.0 - thickness)
            var labelR  = innerR - 18

            var displayVal = root.p_sweepActive ? root.p_sweepValue : root.value
            var range      = maxValue - minValue
            var frac       = range > 0
                          ? Math.max(0, Math.min(1, (displayVal - minValue) / range))
                          : 0

            var sa      = startAngle * Math.PI / 180
            var ea      = endAngle   * Math.PI / 180
            var arcSpan = endAngle - startAngle

            // 1. Outer bezel
            ctx.beginPath()
            ctx.arc(cx, cy, radius + 5, 0, Math.PI * 2)
            var bezel = ctx.createRadialGradient(cx, cy, radius - 2, cx, cy, radius + 5)
            bezel.addColorStop(0, "#252530")
            bezel.addColorStop(1, "#050508")
            ctx.fillStyle = bezel
            ctx.fill()

            // 1b. Accent ring (no shadow)
            ctx.beginPath()
            ctx.arc(cx, cy, radius + 5, 0, Math.PI * 2)
            ctx.strokeStyle = borderColor.toString()
            ctx.lineWidth   = 2.5
            ctx.stroke()

            // 2. Dark background circle
            ctx.beginPath()
            ctx.arc(cx, cy, radius, 0, Math.PI * 2)
            var bg = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius)
            bg.addColorStop(0,    "#1e1e28")
            bg.addColorStop(0.65, "#131318")
            bg.addColorStop(1,    "#080810")
            ctx.fillStyle = bg
            ctx.fill()

            // 3. Full arc track baseline
            ctx.beginPath()
            ctx.arc(cx, cy, radius,  sa, ea)
            ctx.arc(cx, cy, innerR,  ea, sa, true)
            ctx.closePath()
            ctx.fillStyle = "#18181f"
            ctx.fill()

            // 4. Permanent redline zone
            if (redlineStart < 1.0) {
                var redA = (startAngle + redlineStart * arcSpan) * Math.PI / 180
                ctx.beginPath()
                ctx.arc(cx, cy, radius, redA, ea)
                ctx.arc(cx, cy, innerR, ea, redA, true)
                ctx.closePath()
                ctx.fillStyle = "#3d0a0a"
                ctx.fill()
                // bright edge stripe
                ctx.beginPath()
                ctx.arc(cx, cy, radius,     redA - 0.008, ea + 0.008)
                ctx.arc(cx, cy, radius - 3, ea + 0.008, redA - 0.008, true)
                ctx.closePath()
                ctx.fillStyle = "#dd1111"
                ctx.fill()
            }

            // 5. Active arc (no glow/shadow)
            if (frac > 0) {
                var warnA   = (startAngle + warnValue   * arcSpan) * Math.PI / 180
                var dangerA = (startAngle + dangerValue  * arcSpan) * Math.PI / 180
                var curA    = (startAngle + frac         * arcSpan) * Math.PI / 180

                // Green
                var safeEnd = Math.min(curA, warnA)
                if (safeEnd > sa) {
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, sa, safeEnd)
                    ctx.arc(cx, cy, innerR, safeEnd, sa, true)
                    ctx.closePath()
                    ctx.fillStyle = gaugeColor.toString()
                    ctx.fill()
                }
                // Yellow
                if (curA > warnA) {
                    var wEnd = Math.min(curA, dangerA)
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, warnA, wEnd)
                    ctx.arc(cx, cy, innerR, wEnd, warnA, true)
                    ctx.closePath()
                    ctx.fillStyle = warnColor.toString()
                    ctx.fill()
                }
                // Red
                if (curA > dangerA) {
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, dangerA, curA)
                    ctx.arc(cx, cy, innerR, curA, dangerA, true)
                    ctx.closePath()
                    ctx.fillStyle = dangerColor.toString()
                    ctx.fill()
                }
            }

            // 6. Tick marks & labels
            var totalTicks = majorTicks * minorTicks
            for (var i = 0; i <= totalTicks; i++) {
                var tf     = i / totalTicks
                var tAngle = startAngle + tf * arcSpan
                var tRad   = tAngle * Math.PI / 180
                var isMaj  = (i % minorTicks === 0)
                var outerT = innerR - 1
                var innerT = isMaj ? innerR - 16 : innerR - 8
                var lw     = isMaj ? 2.5 : 1.2

                var tc = tf >= dangerValue  ? dangerColor.toString()
                       : tf >= warnValue    ? warnColor.toString()
                       : tickColor.toString()

                ctx.beginPath()
                ctx.moveTo(cx + outerT * Math.cos(tRad), cy + outerT * Math.sin(tRad))
                ctx.lineTo(cx + innerT * Math.cos(tRad), cy + innerT * Math.sin(tRad))
                ctx.strokeStyle = tc
                ctx.lineWidth   = lw
                ctx.stroke()

                if (isMaj) {
                    var lr   = labelR - 2
                    var tVal = Math.round(minValue + tf * range)
                    var tStr = (maxValue >= 1000 && tVal >= 1000)
                               ? (tVal / 1000).toFixed(0)
                               : tVal.toString()
                    ctx.fillStyle    = tf >= dangerValue ? dangerColor.toString() : textColor.toString()
                    ctx.font         = "bold " + (fontSize - 3) + "px sans-serif"
                    ctx.textAlign    = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(tStr, cx + lr * Math.cos(tRad), cy + lr * Math.sin(tRad))
                }
            }

            // 7. Center hub
            ctx.beginPath()
            ctx.arc(cx, cy, innerR * 0.25, 0, Math.PI * 2)
            var hub = ctx.createRadialGradient(cx, cy - 1, 0, cx, cy, innerR * 0.25)
            hub.addColorStop(0, "#383848")
            hub.addColorStop(1, "#0c0c18")
            ctx.fillStyle = hub
            ctx.fill()
            ctx.strokeStyle = "#2a2a3a"
            ctx.lineWidth   = 1.5
            ctx.stroke()

            // 8. Needle (no shadow)
            if (showNeedle) {
                var nAngle   = (startAngle + frac * arcSpan + 90) * Math.PI / 180
                var nLen     = innerR - 4
                var nTail    = innerR * 0.20
                var nW       = 2.5

                var nColor   = frac >= dangerValue ? dangerColor.toString()
                             : frac >= warnValue   ? warnColor.toString()
                             : needleColor.toString()

                ctx.save()
                ctx.translate(cx, cy)
                ctx.rotate(nAngle)

                var nGrad = ctx.createLinearGradient(0, -nLen, 0, nTail)
                nGrad.addColorStop(0,    "#ffffff")
                nGrad.addColorStop(0.12, nColor)
                nGrad.addColorStop(1,    "#1a1a28")

                ctx.beginPath()
                ctx.moveTo(0, -nLen)
                ctx.lineTo(-nW, 0)
                ctx.lineTo(-nW * 0.5, nTail)
                ctx.lineTo( nW * 0.5, nTail)
                ctx.lineTo( nW, 0)
                ctx.closePath()
                ctx.fillStyle = nGrad
                ctx.fill()

                // Pivot cap
                ctx.beginPath()
                ctx.arc(0, 0, nW + 3, 0, Math.PI * 2)
                var cap = ctx.createRadialGradient(0, -1.5, 0, 0, 0, nW + 3)
                cap.addColorStop(0, "#505060")
                cap.addColorStop(1, "#0e0e1c")
                ctx.fillStyle = cap
                ctx.fill()
                ctx.strokeStyle = "#404050"
                ctx.lineWidth   = 1
                ctx.stroke()

                ctx.restore()
            }

            // 9. Value display (no shadow)
            if (showValue) {
                var vColor   = frac >= dangerValue ? dangerColor.toString()
                             : frac >= warnValue   ? warnColor.toString()
                             : gaugeColor.toString()

                var hasUnit  = unitLabel !== ""
                var valY     = cy + (hasUnit ? -(fontSize * 0.7) : 0)

                ctx.fillStyle    = vColor
                ctx.font         = "bold " + (fontSize + 6) + "px sans-serif"
                ctx.textAlign    = "center"
                ctx.textBaseline = "middle"
                ctx.fillText(Math.round(root.value), cx, valY)

                if (hasUnit) {
                    ctx.fillStyle    = textColor.toString()
                    ctx.font         = (fontSize - 1) + "px sans-serif"
                    ctx.textAlign    = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(unitLabel, cx, valY + fontSize + 4)
                }
            }

            // 10. Gauge name
            if (showLabel && label !== "") {
                ctx.fillStyle    = textColor.toString()
                ctx.font         = "bold " + (fontSize - 2) + "px sans-serif"
                ctx.textAlign    = "center"
                ctx.textBaseline = "middle"
                ctx.fillText(label, cx, cy + radius * 0.60)
            }
        }

        Connections {
            target: root
            function onMinValueChanged()       { canvas.requestPaint() }
            function onMaxValueChanged()       { canvas.requestPaint() }
            function onWarnValueChanged()      { canvas.requestPaint() }
            function onDangerValueChanged()    { canvas.requestPaint() }
            function onRedlineStartChanged()   { canvas.requestPaint() }
            function onStartAngleChanged()     { canvas.requestPaint() }
            function onEndAngleChanged()       { canvas.requestPaint() }
            function onUnitLabelChanged()      { canvas.requestPaint() }
            function onLabelChanged()          { canvas.requestPaint() }
        }
    }
}