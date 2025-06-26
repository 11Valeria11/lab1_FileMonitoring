#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "ilogger.h"
#include <QTextStream>
#include <QTime>

class ConsoleLogger : public ILogger {
public:
    void log(const QString& message) override;
};

#endif // CONSOLELOGGER_H

