#pragma once

#include <QObject>
#include <QColor>
#include <QList>
#include <QSettings>
#include <QString>
#include <QStringList>

// Design tokens originally copied verbatim from the Penpot design system.
// Now a small theming engine: a registry of named color schemes (built-in
// presets + user-saved custom themes), with one active dark theme and one
// active light theme. The `dark` toggle picks which of the two is showing.
//
// A custom theme is defined by 8 curated colors (2 backgrounds, 4 accent
// slots, 2 text levels); every other token (glass, borders, dim variants,
// tertiary text, track, shadow) is derived so a custom theme is legible by
// construction. Built-in presets use the same 8-color seed + derivation.
//
// Exposed as a context property (like radio/audio/eq/configStore) rather
// than a QML singleton module — context properties are visible from every
// QML file regardless of directory, sidestepping cross-directory
// import/singleton issues.
class ThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY darkChanged)
    Q_PROPERTY(int modalDepth READ modalDepth NOTIFY modalDepthChanged)

    Q_PROPERTY(QStringList darkThemes READ darkThemes NOTIFY themesChanged)
    Q_PROPERTY(QStringList lightThemes READ lightThemes NOTIFY themesChanged)
    Q_PROPERTY(QStringList allThemes READ allThemes NOTIFY themesChanged)
    Q_PROPERTY(QString activeDarkThemeName READ activeDarkThemeName WRITE setActiveDarkThemeName NOTIFY activeThemesChanged)
    Q_PROPERTY(QString activeLightThemeName READ activeLightThemeName WRITE setActiveLightThemeName NOTIFY activeThemesChanged)

    Q_PROPERTY(QColor bgBase READ bgBase NOTIFY paletteChanged)
    Q_PROPERTY(QColor bgBase2 READ bgBase2 NOTIFY paletteChanged)
    Q_PROPERTY(QColor glass READ glass NOTIFY paletteChanged)
    Q_PROPERTY(QColor glassElevated READ glassElevated NOTIFY paletteChanged)
    Q_PROPERTY(QColor glassBorder READ glassBorder NOTIFY paletteChanged)
    Q_PROPERTY(QColor glassBorderStrong READ glassBorderStrong NOTIFY paletteChanged)
    Q_PROPERTY(QColor modalSurface READ modalSurface NOTIFY paletteChanged)
    Q_PROPERTY(QColor cyan READ cyan NOTIFY paletteChanged)
    Q_PROPERTY(QColor cyanDim READ cyanDim NOTIFY paletteChanged)
    Q_PROPERTY(QColor green READ green NOTIFY paletteChanged)
    Q_PROPERTY(QColor greenDim READ greenDim NOTIFY paletteChanged)
    Q_PROPERTY(QColor amber READ amber NOTIFY paletteChanged)
    Q_PROPERTY(QColor red READ red NOTIFY paletteChanged)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY paletteChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY paletteChanged)
    Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY paletteChanged)
    Q_PROPERTY(QColor trackBg READ trackBg NOTIFY paletteChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor NOTIFY paletteChanged)

    Q_PROPERTY(QString fontBrand READ fontBrand CONSTANT)
    Q_PROPERTY(QString fontUi READ fontUi CONSTANT)
    Q_PROPERTY(QString fontMono READ fontMono CONSTANT)
    Q_PROPERTY(QString fontMonoMedium READ fontMonoMedium CONSTANT)
    Q_PROPERTY(QString fontMonoSemiBold READ fontMonoSemiBold CONSTANT)

    Q_PROPERTY(int spacingXs READ spacingXs CONSTANT)
    Q_PROPERTY(int spacingSm READ spacingSm CONSTANT)
    Q_PROPERTY(int spacingMd READ spacingMd CONSTANT)
    Q_PROPERTY(int spacingLg READ spacingLg CONSTANT)
    Q_PROPERTY(int spacingXl READ spacingXl CONSTANT)
    Q_PROPERTY(int spacingXl2 READ spacingXl2 CONSTANT)
    Q_PROPERTY(int spacingXl3 READ spacingXl3 CONSTANT)
    Q_PROPERTY(int spacingXl4 READ spacingXl4 CONSTANT)
    Q_PROPERTY(int spacingXl5 READ spacingXl5 CONSTANT)
    Q_PROPERTY(int spacingXl6 READ spacingXl6 CONSTANT)

    Q_PROPERTY(int radiusSm READ radiusSm CONSTANT)
    Q_PROPERTY(int radiusMd READ radiusMd CONSTANT)
    Q_PROPERTY(int radiusLg READ radiusLg CONSTANT)
    Q_PROPERTY(int radiusXl READ radiusXl CONSTANT)
    Q_PROPERTY(int radiusPill READ radiusPill CONSTANT)

public:
    // The 8 curated color slots a custom theme (and preset seed) is built
    // from. Order matters: it's the order the editor lists them in.
    static const QStringList &editableKeys();

    explicit ThemeController(QObject *parent = nullptr);
    ~ThemeController() override;

    bool dark() const { return m_dark; }
    void setDark(bool d);

    int modalDepth() const { return m_modalDepth; }
    // Popups (Item-based TapHandlers) don't stop event delivery to items
    // stacked below them the way MouseArea does, so a tap inside an open
    // GlassDialog can also land on a background button at the same screen
    // position. GlassDialog calls these on open/close so the main window
    // can disable its background content for the duration.
    Q_INVOKABLE void pushModal();
    Q_INVOKABLE void popModal();

    Q_INVOKABLE QColor withAlpha(const QColor &c, double alpha) const;

    // --- Theme registry ---
    QStringList darkThemes() const;
    QStringList lightThemes() const;
    QStringList allThemes() const;
    QString activeDarkThemeName() const { return m_activeDark; }
    void setActiveDarkThemeName(const QString &name);
    QString activeLightThemeName() const { return m_activeLight; }
    void setActiveLightThemeName(const QString &name);

    Q_INVOKABLE bool isBuiltin(const QString &name) const;
    Q_INVOKABLE bool themeIsDark(const QString &name) const;

    // Create a custom theme seeded from `sourceName`'s curated colors, made
    // active for its mode. Returns the final (uniquified) name, or empty on
    // failure (unknown source).
    Q_INVOKABLE QString duplicateTheme(const QString &sourceName, const QString &newName);
    Q_INVOKABLE void renameCustomTheme(const QString &oldName, const QString &newName);
    Q_INVOKABLE void deleteCustomTheme(const QString &name);

    // Read/write one curated slot (`key` ∈ editableKeys()) of a theme.
    // Writing a built-in is a no-op.
    Q_INVOKABLE QColor themeColor(const QString &name, const QString &key) const;
    Q_INVOKABLE void setThemeColor(const QString &name, const QString &key, const QColor &color);

    QColor bgBase() const;
    QColor bgBase2() const;
    QColor glass() const;
    QColor glassElevated() const;
    QColor glassBorder() const;
    QColor glassBorderStrong() const;
    QColor modalSurface() const;
    QColor cyan() const;
    QColor cyanDim() const;
    QColor green() const;
    QColor greenDim() const;
    QColor amber() const;
    QColor red() const;
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textTertiary() const;
    QColor trackBg() const;
    QColor shadowColor() const;

    QString fontBrand() const { return QStringLiteral("Orbitron"); }
    QString fontUi() const { return QStringLiteral("Space Grotesk"); }
    QString fontMono() const { return QStringLiteral("JetBrains Mono"); }
    QString fontMonoMedium() const { return QStringLiteral("JetBrains Mono Medium"); }
    QString fontMonoSemiBold() const { return QStringLiteral("JetBrains Mono SemiBold"); }

    int spacingXs() const { return 4; }
    int spacingSm() const { return 8; }
    int spacingMd() const { return 12; }
    int spacingLg() const { return 16; }
    int spacingXl() const { return 20; }
    int spacingXl2() const { return 24; }
    int spacingXl3() const { return 32; }
    int spacingXl4() const { return 40; }
    int spacingXl5() const { return 48; }
    int spacingXl6() const { return 64; }

    int radiusSm() const { return 8; }
    int radiusMd() const { return 12; }
    int radiusLg() const { return 16; }
    int radiusXl() const { return 24; }
    int radiusPill() const { return 999; }

signals:
    void darkChanged();
    void modalDepthChanged();
    void themesChanged();
    void activeThemesChanged();
    void paletteChanged();

private:
    // The 8 curated slots a theme is defined by.
    struct Seed {
        QColor bgBase, bgBase2, cyan, green, amber, red, textPrimary, textSecondary;
    };
    // A fully-resolved set of runtime color tokens.
    struct Palette {
        QColor bgBase, bgBase2;
        QColor glass; double glassOpacity = 1.0;
        QColor glassElevated; double glassElevatedOpacity = 1.0;
        QColor glassBorder; double glassBorderOpacity = 1.0;
        QColor glassBorderStrong; double glassBorderStrongOpacity = 1.0;
        QColor cyan, cyanDim, green, greenDim, amber, red;
        QColor textPrimary, textSecondary, textTertiary;
        QColor trackBg; double trackBgOpacity = 1.0;
        QColor shadowColor;
    };
    struct Theme {
        QString name;
        bool isDark = true;
        bool builtin = false;
        Seed seed;
    };

    static Palette expand(const Seed &s, bool dark);
    static QColor seedSlot(const Seed &s, const QString &key);
    static void setSeedSlot(Seed &s, const QString &key, const QColor &c);

    int indexOfTheme(const QString &name) const;
    QString uniqueName(const QString &desired) const;
    void loadCustomThemes();
    void saveCustomThemes();
    void refreshActive();
    const Theme *activeTheme() const;

    bool m_dark = true;
    int m_modalDepth = 0;
    QString m_activeDark;
    QString m_activeLight;
    QList<Theme> m_themes;
    Palette m_active;
    QSettings m_settings;
};
