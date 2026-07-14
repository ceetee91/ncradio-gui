import QtQuick

// Hand-drawn icon set ported from the Penpot design (same geometry as the
// K.icon() drawing logic used to build the style guide), rendered on a
// Canvas rather than relying on the system icon theme.
Canvas {
    id: root
    property string name: ""
    property color color: "white"
    property real strokeWidth: -1 // -1 = auto (proportional to size)

    implicitWidth: 20
    implicitHeight: 20

    onNameChanged: requestPaint()
    onColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    Component.onCompleted: requestPaint()

    onPaint: {
        const ctx = getContext("2d");
        ctx.reset();
        ctx.clearRect(0, 0, width, height);

        const size = Math.min(width, height);
        const sw = strokeWidth > 0 ? strokeWidth : Math.max(1.4, size * 0.09);
        const cx = width / 2, cy = height / 2;

        ctx.strokeStyle = color;
        ctx.fillStyle = color;
        ctx.lineCap = "round";
        ctx.lineJoin = "round";

        function line(x1, y1, x2, y2, w) {
            ctx.lineWidth = w || sw;
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
        }
        function ring(x, y, d, w) {
            ctx.lineWidth = w || sw;
            ctx.beginPath();
            ctx.arc(x + d / 2, y + d / 2, d / 2, 0, Math.PI * 2);
            ctx.stroke();
        }
        function dot(x, y, d) {
            ctx.beginPath();
            ctx.arc(x + d / 2, y + d / 2, d / 2, 0, Math.PI * 2);
            ctx.fill();
        }
        function triangle(x, y, w, h, dir) {
            ctx.beginPath();
            if (dir === "right") { ctx.moveTo(x, y); ctx.lineTo(x, y + h); ctx.lineTo(x + w, y + h / 2); }
            else if (dir === "left") { ctx.moveTo(x + w, y); ctx.lineTo(x + w, y + h); ctx.lineTo(x, y + h / 2); }
            else if (dir === "up") { ctx.moveTo(x, y + h); ctx.lineTo(x + w, y + h); ctx.lineTo(x + w / 2, y); }
            else { ctx.moveTo(x, y); ctx.lineTo(x + w, y); ctx.lineTo(x + w / 2, y + h); }
            ctx.closePath();
            ctx.fill();
        }
        // QML's Canvas 2D context doesn't implement ctx.roundedRect(), so
        // build the path manually with arcTo (widely supported).
        function roundedRectPath(x, y, w, h, r) {
            r = Math.min(r, w / 2, h / 2);
            ctx.beginPath();
            ctx.moveTo(x + r, y);
            ctx.arcTo(x + w, y, x + w, y + h, r);
            ctx.arcTo(x + w, y + h, x, y + h, r);
            ctx.arcTo(x, y + h, x, y, r);
            ctx.arcTo(x, y, x + w, y, r);
            ctx.closePath();
        }
        function rectStroke(x, y, w, h, r) {
            ctx.lineWidth = sw;
            roundedRectPath(x, y, w, h, r);
            ctx.stroke();
        }
        function rectFill(x, y, w, h, r) {
            roundedRectPath(x, y, w, h, r);
            ctx.fill();
        }

        const x = 0, y = 0;

        switch (name) {
        case "settings":
            ring(x + size * 0.12, y + size * 0.12, size * 0.76);
            dot(cx - size * 0.09, cy - size * 0.09, size * 0.18);
            for (let i = 0; i < 6; i++) {
                const ang = i * Math.PI / 3;
                const r1 = size * 0.42, r2 = size * 0.5;
                line(cx + Math.cos(ang) * r1, cy + Math.sin(ang) * r1, cx + Math.cos(ang) * r2, cy + Math.sin(ang) * r2);
            }
            break;
        case "search":
            ring(x + size * 0.08, y + size * 0.08, size * 0.58);
            line(x + size * 0.62, y + size * 0.62, x + size * 0.92, y + size * 0.92);
            break;
        case "plus":
            line(cx, y + size * 0.12, cx, y + size * 0.88);
            line(x + size * 0.12, cy, x + size * 0.88, cy);
            break;
        case "close":
            line(x + size * 0.16, y + size * 0.16, x + size * 0.84, y + size * 0.84);
            line(x + size * 0.84, y + size * 0.16, x + size * 0.16, y + size * 0.84);
            break;
        case "edit":
            line(x + size * 0.16, y + size * 0.84, x + size * 0.68, y + size * 0.32, sw * 1.3);
            triangle(x + size * 0.64, y + size * 0.12, size * 0.24, size * 0.24, "right");
            break;
        case "trash":
            rectStroke(x + size * 0.22, y + size * 0.32, size * 0.56, size * 0.52, size * 0.06);
            line(x + size * 0.12, y + size * 0.3, x + size * 0.88, y + size * 0.3);
            rectStroke(x + size * 0.38, y + size * 0.14, size * 0.24, size * 0.14, 2);
            line(cx, y + size * 0.44, cx, y + size * 0.72, sw * 0.8);
            break;
        case "scan":
            [0.85, 0.6, 0.35].forEach((f) => {
                const d = size * f;
                ring(cx - d / 2, cy - d / 2, d, sw * 0.8);
            });
            dot(cx - size * 0.06, cy - size * 0.06, size * 0.12);
            break;
        case "seek-back":
            triangle(x + size * 0.08, y + size * 0.2, size * 0.36, size * 0.6, "left");
            triangle(x + size * 0.5, y + size * 0.2, size * 0.36, size * 0.6, "left");
            break;
        case "seek-fwd":
            triangle(x + size * 0.12, y + size * 0.2, size * 0.36, size * 0.6, "right");
            triangle(x + size * 0.54, y + size * 0.2, size * 0.36, size * 0.6, "right");
            break;
        case "step-back":
            line(x + size * 0.22, y + size * 0.2, x + size * 0.22, y + size * 0.8);
            triangle(x + size * 0.32, y + size * 0.2, size * 0.5, size * 0.6, "left");
            break;
        case "step-fwd":
            line(x + size * 0.78, y + size * 0.2, x + size * 0.78, y + size * 0.8);
            triangle(x + size * 0.18, y + size * 0.2, size * 0.5, size * 0.6, "right");
            break;
        case "record":
            dot(x + size * 0.14, y + size * 0.14, size * 0.72);
            break;
        case "mute":
        case "volume":
            ctx.beginPath();
            ctx.moveTo(x + size * 0.12, y + size * 0.38);
            ctx.lineTo(x + size * 0.34, y + size * 0.38);
            ctx.lineTo(x + size * 0.56, y + size * 0.18);
            ctx.lineTo(x + size * 0.56, y + size * 0.82);
            ctx.lineTo(x + size * 0.34, y + size * 0.62);
            ctx.lineTo(x + size * 0.12, y + size * 0.62);
            ctx.closePath();
            ctx.fill();
            if (name === "mute") {
                line(x + size * 0.66, y + size * 0.34, x + size * 0.92, y + size * 0.66);
                line(x + size * 0.92, y + size * 0.34, x + size * 0.66, y + size * 0.66);
            } else {
                ring(x + size * 0.62, cy - size * 0.16, size * 0.32, sw * 0.8);
            }
            break;
        case "signal":
            [0.3, 0.5, 0.7, 1.0].forEach((f, i) => {
                const bw = size * 0.14, gap = size * 0.06, bh = size * 0.8 * f;
                rectFill(x + i * (bw + gap), y + size * 0.9 - bh, bw, bh, 1.5);
            });
            break;
        case "eq":
            [0.4, 0.9, 0.6, 0.3].forEach((f, i) => {
                const bw = size * 0.14, gap = size * 0.1, bh = size * 0.8 * f;
                rectFill(x + size * 0.06 + i * (bw + gap), cy - bh / 2, bw, bh, 1.5);
            });
            break;
        case "theme":
            ring(x + size * 0.1, y + size * 0.1, size * 0.8);
            ctx.beginPath();
            ctx.arc(cx, cy, size * 0.4, -Math.PI / 2, Math.PI / 2, false);
            ctx.closePath();
            ctx.fill();
            break;
        case "info":
            ring(x + size * 0.08, y + size * 0.08, size * 0.84);
            dot(cx - size * 0.05, y + size * 0.22, size * 0.1);
            line(cx, y + size * 0.42, cx, y + size * 0.76);
            break;
        case "check":
            ctx.lineWidth = sw * 1.3;
            ctx.beginPath();
            ctx.moveTo(x + size * 0.16, cy);
            ctx.lineTo(x + size * 0.42, y + size * 0.72);
            ctx.lineTo(x + size * 0.86, y + size * 0.22);
            ctx.stroke();
            break;
        case "minimize":
        case "expand": {
            // Two diagonal arrows on the TL↔BR diagonal. "expand" points them
            // outward to the corners (maximise); "minimize" points them inward
            // to the centre (compress). Each arrow = a shaft + a corner bracket
            // arrowhead at one end.
            const inw = name === "minimize";
            const a = size * 0.14, b = size * 0.46, leg = size * 0.18;
            const A = size - a, B = size - b;
            // top-left arrow
            line(a, a, b, b);
            if (inw) { line(b, b, b - leg, b); line(b, b, b, b - leg); }
            else     { line(a, a, a + leg, a); line(a, a, a, a + leg); }
            // bottom-right arrow
            line(A, A, B, B);
            if (inw) { line(B, B, B + leg, B); line(B, B, B, B + leg); }
            else     { line(A, A, A - leg, A); line(A, A, A, A - leg); }
            break;
        }
        case "pin":
            // Thumbtack: round head (ring + centre) with a needle to a point.
            ring(cx - size * 0.18, y + size * 0.12, size * 0.36, sw * 0.9);
            dot(cx - size * 0.06, y + size * 0.24, size * 0.12);
            line(cx, y + size * 0.48, cx, y + size * 0.9);
            break;
        default:
            ring(x, y, size);
        }
    }
}
