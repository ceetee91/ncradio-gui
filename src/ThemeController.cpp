#include "ThemeController.h"

namespace {
QColor alpha(QColor c, double a)
{
    c.setAlphaF(float(a));
    return c;
}

// Linear RGB-space blend: t=0 -> a, t=1 -> b.
QColor mix(const QColor &a, const QColor &b, double t)
{
    return QColor::fromRgbF(
        float(a.redF()   * (1 - t) + b.redF()   * t),
        float(a.greenF() * (1 - t) + b.greenF() * t),
        float(a.blueF()  * (1 - t) + b.blueF()  * t));
}

const QColor kWhite("#FFFFFF");
const QColor kBlack("#000000");

// Name of the fallback preset for each mode.
QString defaultName(bool dark)
{
    return dark ? QStringLiteral("Default (Dark)") : QStringLiteral("Default (Light)");
}
} // namespace

// Built-in preset seeds. The two "Default" entries reuse the exact 8 curated
// colors of the original hardcoded dark/light palettes, so their look is
// preserved (the derived tokens land visually equivalent). The other six are
// well-known, contrast-checked palettes.
//
// Seed order: bgBase, bgBase2, cyan(primary), green, amber, red,
//             textPrimary, textSecondary.
const QStringList &ThemeController::editableKeys()
{
    static const QStringList keys{
        QStringLiteral("bgBase"), QStringLiteral("bgBase2"),
        QStringLiteral("cyan"), QStringLiteral("green"),
        QStringLiteral("amber"), QStringLiteral("red"),
        QStringLiteral("textPrimary"), QStringLiteral("textSecondary"),
    };
    return keys;
}

namespace {
struct BuiltinDef {
    const char *name;
    bool isDark;
    const char *bgBase, *bgBase2, *cyan, *green, *amber, *red, *textPrimary, *textSecondary;
};

const BuiltinDef kBuiltins[] = {
    // --- Dark ---
    {"Default (Dark)", true,
     "#0A0B16", "#12081F", "#2DE8EC", "#3CFF9A", "#FFC24B", "#FF4D6D", "#F3F6FC", "#9AA6C0"},
    {"Nord Night", true,
     "#2E3440", "#21262E", "#88C0D0", "#A3BE8C", "#EBCB8B", "#BF616A", "#ECEFF4", "#AEB6C6"},
    {"Synthwave", true,
     "#14091F", "#1E0B2E", "#FF4D9D", "#39E0C8", "#FFC857", "#FF3B6B", "#F6EDFF", "#B49AD0"},
    {"Amber Terminal", true,
     "#0B0A08", "#14100A", "#FFB627", "#7CFF6B", "#FFD166", "#FF5C5C", "#F5EFE0", "#B8AE96"},
    // --- Light ---
    {"Default (Light)", false,
     "#EEF1FB", "#E4E8F7", "#0A8C99", "#0E9A5C", "#B8720A", "#D8324D", "#12162A", "#4B5468"},
    {"Solarized Light", false,
     "#FDF6E3", "#EEE8D5", "#268BD2", "#859900", "#B58900", "#DC322F", "#073642", "#586E75"},
    {"Rosé Dawn", false,
     "#FAF4ED", "#F2E9DE", "#286983", "#56949F", "#EA9D34", "#B4637A", "#575279", "#797593"},
    {"Paper", false,
     "#FBF7EF", "#F1EADB", "#0E7C86", "#2E7D32", "#B45309", "#C62828", "#1A1712", "#5B554A"},
};
} // namespace

ThemeController::Palette ThemeController::expand(const Seed &s, bool dark)
{
    Palette p;
    p.bgBase = s.bgBase;
    p.bgBase2 = s.bgBase2;

    p.glass = kWhite;                 p.glassOpacity = dark ? 0.12 : 0.62;
    p.glassElevated = kWhite;         p.glassElevatedOpacity = dark ? 0.20 : 0.82;
    p.glassBorder = dark ? kWhite : s.textPrimary;
    p.glassBorderOpacity = dark ? 0.20 : 0.14;
    p.glassBorderStrong = s.cyan;     p.glassBorderStrongOpacity = dark ? 0.45 : 0.42;

    p.cyan = s.cyan;
    p.cyanDim = mix(s.cyan, s.bgBase, 0.35);
    p.green = s.green;
    p.greenDim = mix(s.green, s.bgBase, 0.30);
    p.amber = s.amber;
    p.red = s.red;

    p.textPrimary = s.textPrimary;
    p.textSecondary = s.textSecondary;
    p.textTertiary = mix(s.textSecondary, s.bgBase, 0.30);

    p.trackBg = dark ? kWhite : s.textPrimary;
    p.trackBgOpacity = dark ? 0.08 : 0.06;
    // Light themes need a dark shadow (near-text color); dark themes drop to
    // pure black. bgBase2.darker() would stay light on a light theme, so use
    // the primary text color as the shadow tint instead.
    p.shadowColor = dark ? kBlack : s.textPrimary;
    return p;
}

QColor ThemeController::seedSlot(const Seed &s, const QString &key)
{
    if (key == QLatin1String("bgBase")) return s.bgBase;
    if (key == QLatin1String("bgBase2")) return s.bgBase2;
    if (key == QLatin1String("cyan")) return s.cyan;
    if (key == QLatin1String("green")) return s.green;
    if (key == QLatin1String("amber")) return s.amber;
    if (key == QLatin1String("red")) return s.red;
    if (key == QLatin1String("textPrimary")) return s.textPrimary;
    if (key == QLatin1String("textSecondary")) return s.textSecondary;
    return QColor();
}

void ThemeController::setSeedSlot(Seed &s, const QString &key, const QColor &c)
{
    if (key == QLatin1String("bgBase")) s.bgBase = c;
    else if (key == QLatin1String("bgBase2")) s.bgBase2 = c;
    else if (key == QLatin1String("cyan")) s.cyan = c;
    else if (key == QLatin1String("green")) s.green = c;
    else if (key == QLatin1String("amber")) s.amber = c;
    else if (key == QLatin1String("red")) s.red = c;
    else if (key == QLatin1String("textPrimary")) s.textPrimary = c;
    else if (key == QLatin1String("textSecondary")) s.textSecondary = c;
}

ThemeController::ThemeController(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("ceetee91"), QStringLiteral("ncradio-gui"))
{
    for (const BuiltinDef &b : kBuiltins) {
        Theme t;
        t.name = QString::fromUtf8(b.name);
        t.isDark = b.isDark;
        t.builtin = true;
        t.seed = Seed{QColor(b.bgBase), QColor(b.bgBase2), QColor(b.cyan),
                      QColor(b.green), QColor(b.amber), QColor(b.red),
                      QColor(b.textPrimary), QColor(b.textSecondary)};
        m_themes.append(t);
    }
    loadCustomThemes();

    m_dark = m_settings.value(QStringLiteral("Appearance/dark"), true).toBool();
    m_activeDark = m_settings.value(QStringLiteral("Appearance/darkTheme"),
                                    defaultName(true)).toString();
    m_activeLight = m_settings.value(QStringLiteral("Appearance/lightTheme"),
                                     defaultName(false)).toString();

    // Fall back to the mode's Default if a stored name no longer resolves
    // (e.g. a custom theme was deleted out-of-band, or an older config
    // predates this preset), or resolves to the wrong mode.
    auto validate = [this](QString &name, bool wantDark) {
        int i = indexOfTheme(name);
        if (i < 0 || m_themes[i].isDark != wantDark)
            name = defaultName(wantDark);
    };
    validate(m_activeDark, true);
    validate(m_activeLight, false);

    refreshActive();
}

ThemeController::~ThemeController() = default;

int ThemeController::indexOfTheme(const QString &name) const
{
    for (int i = 0; i < m_themes.size(); ++i)
        if (m_themes[i].name == name)
            return i;
    return -1;
}

const ThemeController::Theme *ThemeController::activeTheme() const
{
    int i = indexOfTheme(m_dark ? m_activeDark : m_activeLight);
    if (i < 0)
        i = indexOfTheme(defaultName(m_dark));
    return i >= 0 ? &m_themes[i] : nullptr;
}

void ThemeController::refreshActive()
{
    // Resolve the active theme for the current mode, falling back to that
    // mode's Default preset if the stored name can't be found.
    const Theme *t = nullptr;
    const QString &want = m_dark ? m_activeDark : m_activeLight;
    for (const Theme &e : m_themes)
        if (e.name == want && e.isDark == m_dark) { t = &e; break; }
    if (!t) {
        const QString fallback = defaultName(m_dark);
        for (const Theme &e : m_themes)
            if (e.name == fallback && e.isDark == m_dark) { t = &e; break; }
    }
    if (t)
        m_active = expand(t->seed, t->isDark);
    emit paletteChanged();
}

QStringList ThemeController::darkThemes() const
{
    QStringList out;
    for (const Theme &t : m_themes)
        if (t.isDark) out << t.name;
    return out;
}

QStringList ThemeController::lightThemes() const
{
    QStringList out;
    for (const Theme &t : m_themes)
        if (!t.isDark) out << t.name;
    return out;
}

QStringList ThemeController::allThemes() const
{
    QStringList out;
    for (const Theme &t : m_themes)
        out << t.name;
    return out;
}

bool ThemeController::isBuiltin(const QString &name) const
{
    int i = indexOfTheme(name);
    return i >= 0 && m_themes[i].builtin;
}

bool ThemeController::themeIsDark(const QString &name) const
{
    int i = indexOfTheme(name);
    return i >= 0 ? m_themes[i].isDark : true;
}

void ThemeController::setActiveDarkThemeName(const QString &name)
{
    if (m_activeDark == name || indexOfTheme(name) < 0)
        return;
    m_activeDark = name;
    m_settings.setValue(QStringLiteral("Appearance/darkTheme"), name);
    emit activeThemesChanged();
    if (m_dark)
        refreshActive();
}

void ThemeController::setActiveLightThemeName(const QString &name)
{
    if (m_activeLight == name || indexOfTheme(name) < 0)
        return;
    m_activeLight = name;
    m_settings.setValue(QStringLiteral("Appearance/lightTheme"), name);
    emit activeThemesChanged();
    if (!m_dark)
        refreshActive();
}

QString ThemeController::uniqueName(const QString &desired) const
{
    QString base = desired.trimmed();
    if (base.isEmpty())
        base = QStringLiteral("Custom");
    if (indexOfTheme(base) < 0)
        return base;
    for (int n = 2;; ++n) {
        const QString candidate = QStringLiteral("%1 %2").arg(base).arg(n);
        if (indexOfTheme(candidate) < 0)
            return candidate;
    }
}

QString ThemeController::duplicateTheme(const QString &sourceName, const QString &newName)
{
    int i = indexOfTheme(sourceName);
    if (i < 0)
        return QString();

    Theme t;
    t.name = uniqueName(newName);
    t.isDark = m_themes[i].isDark;
    t.builtin = false;
    t.seed = m_themes[i].seed;
    m_themes.append(t);

    saveCustomThemes();
    emit themesChanged();

    // Make the new theme active for its mode.
    if (t.isDark)
        setActiveDarkThemeName(t.name);
    else
        setActiveLightThemeName(t.name);
    return t.name;
}

void ThemeController::renameCustomTheme(const QString &oldName, const QString &newName)
{
    int i = indexOfTheme(oldName);
    if (i < 0 || m_themes[i].builtin)
        return;
    const QString unique = uniqueName(newName);
    const bool wasActiveDark = (m_activeDark == oldName);
    const bool wasActiveLight = (m_activeLight == oldName);
    m_themes[i].name = unique;
    saveCustomThemes();
    emit themesChanged();

    if (wasActiveDark) { m_activeDark = unique;
        m_settings.setValue(QStringLiteral("Appearance/darkTheme"), unique); emit activeThemesChanged(); }
    if (wasActiveLight) { m_activeLight = unique;
        m_settings.setValue(QStringLiteral("Appearance/lightTheme"), unique); emit activeThemesChanged(); }
}

void ThemeController::deleteCustomTheme(const QString &name)
{
    int i = indexOfTheme(name);
    if (i < 0 || m_themes[i].builtin)
        return;
    const bool wasActiveDark = (m_activeDark == name);
    const bool wasActiveLight = (m_activeLight == name);
    m_themes.removeAt(i);
    saveCustomThemes();
    emit themesChanged();

    if (wasActiveDark)
        setActiveDarkThemeName(defaultName(true));
    if (wasActiveLight)
        setActiveLightThemeName(defaultName(false));
}

QColor ThemeController::themeColor(const QString &name, const QString &key) const
{
    int i = indexOfTheme(name);
    return i >= 0 ? seedSlot(m_themes[i].seed, key) : QColor();
}

void ThemeController::setThemeColor(const QString &name, const QString &key, const QColor &color)
{
    int i = indexOfTheme(name);
    if (i < 0 || m_themes[i].builtin || !editableKeys().contains(key))
        return;
    setSeedSlot(m_themes[i].seed, key, color);
    saveCustomThemes();
    // Re-theme live if this theme is the one currently on screen.
    const Theme &t = m_themes[i];
    if ((t.isDark && m_dark && m_activeDark == name) ||
        (!t.isDark && !m_dark && m_activeLight == name))
        refreshActive();
}

void ThemeController::loadCustomThemes()
{
    const int count = m_settings.beginReadArray(QStringLiteral("CustomThemes"));
    for (int i = 0; i < count; ++i) {
        m_settings.setArrayIndex(i);
        Theme t;
        t.name = m_settings.value(QStringLiteral("name")).toString();
        t.isDark = m_settings.value(QStringLiteral("isDark"), true).toBool();
        t.builtin = false;
        if (t.name.isEmpty() || indexOfTheme(t.name) >= 0)
            continue; // skip blank/duplicate names
        Seed s;
        for (const QString &key : editableKeys())
            setSeedSlot(s, key, QColor(m_settings.value(key).toString()));
        t.seed = s;
        m_themes.append(t);
    }
    m_settings.endArray();
}

void ThemeController::saveCustomThemes()
{
    // Clear the group first: beginWriteArray only truncates when it can see
    // the prior size, so a shrinking array (e.g. the last custom theme
    // deleted) can otherwise leave orphaned "N\..." entries behind.
    m_settings.remove(QStringLiteral("CustomThemes"));
    m_settings.beginWriteArray(QStringLiteral("CustomThemes"));
    int idx = 0;
    for (const Theme &t : m_themes) {
        if (t.builtin)
            continue;
        m_settings.setArrayIndex(idx++);
        m_settings.setValue(QStringLiteral("name"), t.name);
        m_settings.setValue(QStringLiteral("isDark"), t.isDark);
        for (const QString &key : editableKeys())
            m_settings.setValue(key, seedSlot(t.seed, key).name());
    }
    m_settings.endArray();
}

void ThemeController::setDark(bool d)
{
    if (m_dark == d)
        return;
    m_dark = d;
    m_settings.setValue(QStringLiteral("Appearance/dark"), d);
    emit darkChanged();
    refreshActive();
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

QColor ThemeController::bgBase() const { return m_active.bgBase; }
QColor ThemeController::bgBase2() const { return m_active.bgBase2; }
QColor ThemeController::glass() const { return alpha(m_active.glass, m_active.glassOpacity); }
QColor ThemeController::glassElevated() const { return alpha(m_active.glassElevated, m_active.glassElevatedOpacity); }
QColor ThemeController::glassBorder() const { return alpha(m_active.glassBorder, m_active.glassBorderOpacity); }
QColor ThemeController::glassBorderStrong() const { return alpha(m_active.glassBorderStrong, m_active.glassBorderStrongOpacity); }
// Modal dialogs sit over arbitrary content (or the desktop, if the window
// itself is translucent), so they need a near-opaque surface — unlike
// in-page glass panels, which only need to read subtly against the app's
// own already-dark/light background.
QColor ThemeController::modalSurface() const { return alpha(m_active.bgBase2, 0.96); }
QColor ThemeController::cyan() const { return m_active.cyan; }
QColor ThemeController::cyanDim() const { return m_active.cyanDim; }
QColor ThemeController::green() const { return m_active.green; }
QColor ThemeController::greenDim() const { return m_active.greenDim; }
QColor ThemeController::amber() const { return m_active.amber; }
QColor ThemeController::red() const { return m_active.red; }
QColor ThemeController::textPrimary() const { return m_active.textPrimary; }
QColor ThemeController::textSecondary() const { return m_active.textSecondary; }
QColor ThemeController::textTertiary() const { return m_active.textTertiary; }
QColor ThemeController::trackBg() const { return alpha(m_active.trackBg, m_active.trackBgOpacity); }
QColor ThemeController::shadowColor() const { return m_active.shadowColor; }
