#include <QApplication>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("TaskScript");
    QApplication::setOrganizationName("USAC - LFP 1S2026");
    QApplication::setApplicationDisplayName("TaskScript");

    taskscript::MainWindow w;
    w.show();
    return app.exec();
}
