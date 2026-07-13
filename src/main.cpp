#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QFontDatabase>
#include <QIcon>
#include <QSocketNotifier>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

#include "ConfigStore.h"
#include "PresetListModel.h"
#include "RadioController.h"
#include "AudioController.h"
#include "EqController.h"
#include "RecordController.h"
#include "ThemeController.h"

namespace {
int g_signalFd[2];

void unixSignalHandler(int)
{
    char a = 1;
    ::write(g_signalFd[0], &a, sizeof(a));
}
} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("ncradio"));
    app.setApplicationName(QStringLiteral("ncradio-gui"));
    QIcon::setThemeName(QStringLiteral("breeze"));

    // Graceful shutdown on SIGTERM/SIGINT (self-pipe trick — signal handlers
    // must stay async-signal-safe, so just wake the event loop via a socket).
    ::socketpair(AF_UNIX, SOCK_STREAM, 0, g_signalFd);
    QSocketNotifier sigNotifier(g_signalFd[1], QSocketNotifier::Read);
    QObject::connect(&sigNotifier, &QSocketNotifier::activated, &app, [] {
        char tmp;
        ::read(g_signalFd[1], &tmp, sizeof(tmp));
        QCoreApplication::quit();
    });
    std::signal(SIGTERM, unixSignalHandler);
    std::signal(SIGINT, unixSignalHandler);

    for (const QString &path : {
             QStringLiteral(":/fonts/JetBrainsMono-Regular.ttf"),
             QStringLiteral(":/fonts/JetBrainsMono-Medium.ttf"),
             QStringLiteral(":/fonts/JetBrainsMono-SemiBold.ttf"),
             QStringLiteral(":/fonts/JetBrainsMono-Bold.ttf"),
             QStringLiteral(":/fonts/SpaceGrotesk-Variable.ttf"),
             QStringLiteral(":/fonts/Orbitron-Variable.ttf"),
         }) {
        const int id = QFontDatabase::addApplicationFont(path);
        if (id < 0) {
            qWarning() << "Failed to load font:" << path;
        } else {
            qWarning() << "Loaded font:" << path << "as" << QFontDatabase::applicationFontFamilies(id);
        }
    }

    ConfigStore configStore;
    configStore.load();

    PresetListModel presetModel;
    presetModel.setConfigStore(&configStore);

    RadioController radio(&configStore);
    const QString device = app.arguments().size() > 1 ? app.arguments().at(1) : QStringLiteral("/dev/radio0");
    if (!radio.open(device))
        qWarning() << "radio.open failed:" << radio.errorMessage();

    EqController eq(&configStore);
    AudioController audio(&configStore);
    audio.setEq(eq.raw());
    audio.applyFromConfig(device);

    RecordController record(&configStore, &audio);
    ThemeController theme;

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &radio, [&] {
        if (record.recording())
            record.stop();
        Config &cfg = configStore.data();
        cfg.volume = radio.volume();
        cfg.last_freq_hz = radio.frequencyHz();
        configStore.save();
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("configStore"), &configStore);
    engine.rootContext()->setContextProperty(QStringLiteral("presetModel"), &presetModel);
    engine.rootContext()->setContextProperty(QStringLiteral("radio"), &radio);
    engine.rootContext()->setContextProperty(QStringLiteral("audio"), &audio);
    engine.rootContext()->setContextProperty(QStringLiteral("eq"), &eq);
    engine.rootContext()->setContextProperty(QStringLiteral("recorder"), &record);
    engine.rootContext()->setContextProperty(QStringLiteral("Theme"), &theme);

    QObject::connect(&engine, &QQmlApplicationEngine::warnings,
                      &app, [](const QList<QQmlError> &warnings) {
                          for (const auto &w : warnings)
                              qWarning() << "QML warning:" << w.toString();
                      });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("ncradiogui", "Main");

    return app.exec();
}
