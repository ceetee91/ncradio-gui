#include "ThemeController.h"

namespace {
struct Palette {
    QColor bgBase, bgBase2;
    QColor glass; double glassOpacity;
    QColor glassElevated; double glassElevatedOpacity;
    QColor glassBorder; double glassBorderOpacity;
    QColor glassBorderStrong; double glassBorderStrongOpacity;
    QColor cyan, cyanDim, green, greenDim, amber, red;
    QColor textPrimary, textSecondary, textTertiary;
    QColor trackBg; double trackBgOpacity;
    QColor shadowColor;
};

const Palette kDark{
    QColor("#0A0B16"), QColor("#12081F"),
    QColor("#FFFFFF"), 0.12,
    QColor("#FFFFFF"), 0.20,
    QColor("#FFFFFF"), 0.20,
    QColor("#8FF5FF"), 0.45,
    QColor("#2DE8EC"), QColor("#1B9EA3"), QColor("#3CFF9A"), QColor("#1FAE68"), QColor("#FFC24B"), QColor("#FF4D6D"),
    QColor("#F3F6FC"), QColor("#9AA6C0"), QColor("#7C86AC"),
    QColor("#FFFFFF"), 0.08,
    QColor("#000000"),
};

const Palette kLight{
    QColor("#EEF1FB"), QColor("#E4E8F7"),
    QColor("#FFFFFF"), 0.62,
    QColor("#FFFFFF"), 0.82,
    QColor("#1B2340"), 0.14,
    QColor("#0EA5B7"), 0.42,
    QColor("#0A8C99"), QColor("#0EA5B7"), QColor("#0E9A5C"), QColor("#12B36B"), QColor("#B8720A"), QColor("#D8324D"),
    QColor("#12162A"), QColor("#4B5468"), QColor("#6B7690"),
    QColor("#12162A"), 0.06,
    QColor("#22284A"),
};

QColor alpha(QColor c, double a)
{
    c.setAlphaF(float(a));
    return c;
}
} // namespace

ThemeController::ThemeController(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("ncradio"), QStringLiteral("ncradio-gui"))
{
    m_dark = m_settings.value(QStringLiteral("Appearance/dark"), true).toBool();
}

void ThemeController::setDark(bool d)
{
    if (m_dark == d)
        return;
    m_dark = d;
    m_settings.setValue(QStringLiteral("Appearance/dark"), d);
    emit darkChanged();
}

void ThemeController::pushModal()
{
    ++m_modalDepth;
    emit modalDepthChanged();
}

void ThemeController::popModal()
{
    --m_modalDepth;
    emit modalDepthChanged();
}

QColor ThemeController::withAlpha(const QColor &c, double a) const
{
    return alpha(c, a);
}

const Palette &pal(bool dark) { return dark ? kDark : kLight; }

QColor ThemeController::bgBase() const { return pal(m_dark).bgBase; }
QColor ThemeController::bgBase2() const { return pal(m_dark).bgBase2; }
QColor ThemeController::glass() const { return alpha(pal(m_dark).glass, pal(m_dark).glassOpacity); }
QColor ThemeController::glassElevated() const { return alpha(pal(m_dark).glassElevated, pal(m_dark).glassElevatedOpacity); }
QColor ThemeController::glassBorder() const { return alpha(pal(m_dark).glassBorder, pal(m_dark).glassBorderOpacity); }
QColor ThemeController::glassBorderStrong() const { return alpha(pal(m_dark).glassBorderStrong, pal(m_dark).glassBorderStrongOpacity); }
// Modal dialogs sit over arbitrary content (or the desktop, if the window
// itself is translucent), so they need a near-opaque surface — unlike
// in-page glass panels, which only need to read subtly against the app's
// own already-dark/light background.
QColor ThemeController::modalSurface() const { return alpha(pal(m_dark).bgBase2, 0.96); }
QColor ThemeController::cyan() const { return pal(m_dark).cyan; }
QColor ThemeController::cyanDim() const { return pal(m_dark).cyanDim; }
QColor ThemeController::green() const { return pal(m_dark).green; }
QColor ThemeController::greenDim() const { return pal(m_dark).greenDim; }
QColor ThemeController::amber() const { return pal(m_dark).amber; }
QColor ThemeController::red() const { return pal(m_dark).red; }
QColor ThemeController::textPrimary() const { return pal(m_dark).textPrimary; }
QColor ThemeController::textSecondary() const { return pal(m_dark).textSecondary; }
QColor ThemeController::textTertiary() const { return pal(m_dark).textTertiary; }
QColor ThemeController::trackBg() const { return alpha(pal(m_dark).trackBg, pal(m_dark).trackBgOpacity); }
QColor ThemeController::shadowColor() const { return pal(m_dark).shadowColor; }
