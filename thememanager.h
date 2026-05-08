#pragma once
#include <QObject>
#include <QColor>

class ThemeManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)

    // Dark theme colors
    Q_PROPERTY(QColor carBlue READ carBlue NOTIFY themeChanged)
    Q_PROPERTY(QColor carBlueDim READ carBlueDim NOTIFY themeChanged)
    Q_PROPERTY(QColor carOrange READ carOrange NOTIFY themeChanged)
    Q_PROPERTY(QColor bgDark READ bgDark NOTIFY themeChanged)
    Q_PROPERTY(QColor bgPanel READ bgPanel NOTIFY themeChanged)
    Q_PROPERTY(QColor bgCard READ bgCard NOTIFY themeChanged)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY themeChanged)

    // Status colors (same across themes)
    Q_PROPERTY(QColor statusGreen READ statusGreen CONSTANT)
    Q_PROPERTY(QColor statusRed READ statusRed CONSTANT)
    Q_PROPERTY(QColor statusYellow READ statusYellow CONSTANT)
    Q_PROPERTY(QColor statusOrange READ statusOrange CONSTANT)
    Q_PROPERTY(QColor warnColor READ warnColor CONSTANT)
    Q_PROPERTY(QColor dangerColor READ dangerColor CONSTANT)

    // Gauge colors
    Q_PROPERTY(QColor gaugeColor READ gaugeColor NOTIFY themeChanged)
    Q_PROPERTY(QColor tickColor READ tickColor NOTIFY themeChanged)
    Q_PROPERTY(QColor gaugeTextColor READ gaugeTextColor NOTIFY themeChanged)
    Q_PROPERTY(QColor needleColor READ needleColor NOTIFY themeChanged)

    // Common
    Q_PROPERTY(QColor black READ black CONSTANT)
    Q_PROPERTY(QColor white READ white CONSTANT)
    Q_PROPERTY(QColor transparent READ transparent CONSTANT)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)

public:
    explicit ThemeManager(QObject *parent = nullptr) : QObject(parent), m_currentTheme("Dark") {
        updateThemeColors();
    }

    QString currentTheme() const { return m_currentTheme; }
    QColor accentColor() const { return m_accentColor; }

    // Dark theme
    QColor carBlue() const { return m_carBlue; }
    QColor carBlueDim() const { return m_carBlueDim; }
    QColor carOrange() const { return m_carOrange; }
    QColor bgDark() const { return m_bgDark; }
    QColor bgPanel() const { return m_bgPanel; }
    QColor bgCard() const { return m_bgCard; }
    QColor textPrimary() const { return m_textPrimary; }
    QColor textSecondary() const { return m_textSecondary; }

    // Status colors
    QColor statusGreen() const { return QColor("#00ff88"); }
    QColor statusRed() const { return QColor("#ff4444"); }
    QColor statusYellow() const { return QColor("#ffaa00"); }
    QColor statusOrange() const { return QColor("#ff6600"); }
    QColor warnColor() const { return QColor("#ff9900"); }
    QColor dangerColor() const { return QColor("#ff4444"); }

    // Gauge
    QColor gaugeColor() const { return m_gaugeColor; }
    QColor tickColor() const { return m_tickColor; }
    QColor gaugeTextColor() const { return m_gaugeTextColor; }
    QColor needleColor() const { return m_needleColor; }

    // Common
    QColor black() const { return QColor("#000000"); }
    QColor white() const { return QColor("#ffffff"); }
    QColor transparent() const { return QColor("transparent"); }
    QString fontFamily() const { return "sans-serif"; }

public slots:
    void setCurrentTheme(const QString &theme) {
        if (m_currentTheme == theme) return;
        m_currentTheme = theme;
        updateThemeColors();
        emit themeChanged();
    }

    void setAccentColor(const QColor &color) {
        if (m_accentColor == color) return;
        m_accentColor = color;
        m_gaugeColor = color;
        emit accentColorChanged();
        emit themeChanged();
    }

signals:
    void themeChanged();
    void accentColorChanged();

private:
    void updateThemeColors() {
        if (m_currentTheme == "Light") {
            m_carBlue = QColor("#0078d4");
            m_carBlueDim = QColor("#004e8c");
            m_carOrange = QColor("#e65c2e");
            m_bgDark = QColor("#f5f5f5");
            m_bgPanel = QColor("#ffffff");
            m_bgCard = QColor("#e8e8e8");
            m_textPrimary = QColor("#1a1a1a");
            m_textSecondary = QColor("#666666");
        } else if (m_currentTheme == "Blue") {
            m_carBlue = QColor("#00d4ff");
            m_carBlueDim = QColor("#005a6e");
            m_carOrange = QColor("#ff8c42");
            m_bgDark = QColor("#0a0f1a");
            m_bgPanel = QColor("#0d1520");
            m_bgCard = QColor("#142030");
            m_textPrimary = QColor("#e0e8ff");
            m_textSecondary = QColor("#7a8aaa");
        } else if (m_currentTheme == "Red") {
            m_carBlue = QColor("#ff2020");
            m_carBlueDim = QColor("#6e0000");
            m_carOrange = QColor("#ff8c00");
            m_bgDark = QColor("#0f0a0a");
            m_bgPanel = QColor("#1a0d0d");
            m_bgCard = QColor("#240f0f");
            m_textPrimary = QColor("#ffffff");
            m_textSecondary = QColor("#aa8888");
        } else if (m_currentTheme == "Green") {
            m_carBlue = QColor("#00ff88");
            m_carBlueDim = QColor("#006633");
            m_carOrange = QColor("#ffaa00");
            m_bgDark = QColor("#0a0f0a");
            m_bgPanel = QColor("#0d1a0d");
            m_bgCard = QColor("#142414");
            m_textPrimary = QColor("#e0ffe0");
            m_textSecondary = QColor("#7aaa7a");
        } else if (m_currentTheme == "Purple") {
            m_carBlue = QColor("#9b59b6");
            m_carBlueDim = QColor("#4a235a");
            m_carOrange = QColor("#ff8c42");
            m_bgDark = QColor("#0f0a14");
            m_bgPanel = QColor("#1a0d24");
            m_bgCard = QColor("#241430");
            m_textPrimary = QColor("#f0e0ff");
            m_textSecondary = QColor("#8a7aaa");
        } else if (m_currentTheme == "Orange") {
            m_carBlue = QColor("#ff6b35");
            m_carBlueDim = QColor("#8c3a1a");
            m_carOrange = QColor("#ffaa00");
            m_bgDark = QColor("#0f0a07");
            m_bgPanel = QColor("#1a0d09");
            m_bgCard = QColor("#241a10");
            m_textPrimary = QColor("#fff5e0");
            m_textSecondary = QColor("#aa8a78");
        } else { // Dark (default)
            m_carBlue = QColor("#00a8e8");
            m_carBlueDim = QColor("#003a5c");
            m_carOrange = QColor("#ff6b35");
            m_bgDark = QColor("#0a0a0f");
            m_bgPanel = QColor("#12121a");
            m_bgCard = QColor("#1a1a24");
            m_textPrimary = QColor("#ffffff");
            m_textSecondary = QColor("#8888aa");
        }

        // Set gauge colors
        m_gaugeColor = m_accentColor.isValid() ? m_accentColor : m_carBlue;
        m_tickColor = QColor("#666666");
        m_gaugeTextColor = m_textPrimary;
        m_needleColor = m_gaugeColor;
    }

    QString m_currentTheme;
    QColor m_accentColor;

    QColor m_carBlue, m_carBlueDim, m_carOrange;
    QColor m_bgDark, m_bgPanel, m_bgCard;
    QColor m_textPrimary, m_textSecondary;
    QColor m_gaugeColor, m_tickColor, m_gaugeTextColor, m_needleColor;
};
