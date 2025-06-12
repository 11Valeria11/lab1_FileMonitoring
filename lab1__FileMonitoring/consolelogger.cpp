#include "consolelogger.h"

void ConsoleLogger::log(const QString& message) {
    QTextStream out(stdout);
    out << QTime::currentTime().toString("hh:mm:ss.zzz") << " | " << message << Qt::endl;
}
