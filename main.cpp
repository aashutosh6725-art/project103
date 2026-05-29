#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Create the backend — lives for the entire app lifetime
    Backend backend;

    QQmlApplicationEngine engine;

    // Expose backend to QML — after this line, every QML file can use "backend.xxx"
    engine.rootContext()->setContextProperty("backend", &backend);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("MeshTalk", "Main");

    return QGuiApplication::exec();   
}