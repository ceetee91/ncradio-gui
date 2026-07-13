#pragma once

#include <QObject>
#include <QColor>
#include <QSettings>

// Design tokens copied verbatim from the Penpot design system (dark/light
// palettes, type scale, spacing, radii). Exposed as a context property
// (like radio/audio/eq/configStore) rather than a QML singleton module —
// context properties are visible from every QML file regardless of
// directory, sidestepping cross-directory import/singleton issues.
class ThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY darkChanged)
    Q_PROPERTY(int modalDepth READ modalDepth NOTIFY modalDepthChanged)

    Q_PROPERTY(QColor bgBase READ bgBase NOTIFY darkChanged)
    Q_PROPERTY(QColor bgBase2 READ bgBase2 NOTIFY darkChanged)
    Q_PROPERTY(QColor glass READ glass NOTIFY darkChanged)
    Q_PROPERTY(QColor glassElevated READ glassElevated NOTIFY darkChanged)
    Q_PROPERTY(QColor glassBorder READ glassBorder NOTIFY darkChanged)
    Q_PROPERTY(QColor glassBorderStrong READ glassBorderStrong NOTIFY darkChanged)
    Q_PROPERTY(QColor modalSurface READ modalSurface NOTIFY darkChanged)
    Q_PROPERTY(QColor cyan READ cyan NOTIFY darkChanged)
    Q_PROPERTY(QColor cyanDim READ cyanDim NOTIFY darkChanged)
    Q_PROPERTY(QColor green READ green NOTIFY darkChanged)
    Q_PROPERTY(QColor greenDim READ greenDim NOTIFY darkChanged)
    Q_PROPERTY(QColor amber READ amber NOTIFY darkChanged)
    Q_PROPERTY(QColor red READ red NOTIFY darkChanged)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY darkChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY darkChanged)
    Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY darkChanged)
    Q_PROPERTY(QColor trackBg READ trackBg NOTIFY darkChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor NOTIFY darkChanged)

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
    explicit ThemeController(QObject *parent = nullptr);

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

private:
    bool m_dark = true;
    int m_modalDepth = 0;
    QSettings m_settings;
};
