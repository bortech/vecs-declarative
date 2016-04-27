#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include "vecscontroller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    VecsController vecs;

    qmlRegisterType<VecsDevice>("com.vecs.device", 1, 0, "VecsDevice");

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("vecs", &vecs);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
