/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <QCoreApplication>
#include <QDateTime>
#include <QStringList>

#include "logging.h"


namespace logging {

static Level sDefaultLevel = Level_Debug;
static QMap<QString, Level>* sClassLevels = NULL;
static QIODevice* sNullDevice = NULL;

const char* kDefaultLogLevels = "GstEnginePipeline:2,*:3";

static const char* kMessageHandlerMagic = "__logging_message__";
static const int kMessageHandlerMagicLength = strlen(kMessageHandlerMagic);
static QtMsgHandler sOriginalMessageHandler = NULL;


static void MessageHandler(QtMsgType type, const char* message) {
  if (strncmp(kMessageHandlerMagic, message, kMessageHandlerMagicLength) == 0) {
    fprintf(stderr, "%s\n", message + kMessageHandlerMagicLength);
    return;
  }

  Level level = Level_Debug;
  switch (type) {
    case QtFatalMsg:
    case QtCriticalMsg: level = Level_Error;   break;
    case QtWarningMsg:  level = Level_Warning; break;
    case QtDebugMsg:
    default:            level = Level_Debug;   break;
  }

  foreach (const QString& line, QString::fromLocal8Bit(message).split('\n')) {
    CreateLogger(level, "unknown", -1) << line.toLocal8Bit().constData();
  }

  if (type == QtFatalMsg) {
    abort();
  }
}


void Init() {
  delete sClassLevels;
  delete sNullDevice;

  sClassLevels = new QMap<QString, Level>();
  sNullDevice = new NullDevice;

  // Catch other messages from Qt
  if (!sOriginalMessageHandler) {
    sOriginalMessageHandler = qInstallMsgHandler(MessageHandler);
  }
}

void SetLevels(const QString& levels) {
  if (!sClassLevels)
    return;

  foreach (const QString& item, levels.split(',')) {
    const QStringList class_level = item.split(':');

    QString class_name;
    bool ok = false;
    int level = Level_Error;

    if (class_level.count() == 1) {
      level = class_level.last().toInt(&ok);
    } else if (class_level.count() == 2) {
      class_name = class_level.first();
      level = class_level.last().toInt(&ok);
    }

    if (!ok || level < Level_Error || level > Level_Debug) {
      continue;
    }

    if (class_name.isEmpty() || class_name == "*") {
      sDefaultLevel = (Level) level;
    } else {
      sClassLevels->insert(class_name, (Level) level);
    }
  }
}

QString ParsePrettyFunction(const char * pretty_function) {
  // Get the class name out of the function name.
  QString class_name = pretty_function;
  const int paren = class_name.indexOf('(');
  if (paren != -1) {
    const int colons = class_name.lastIndexOf("::", paren);
    if (colons != -1) {
      class_name = class_name.left(colons);
    } else {
      class_name = class_name.left(paren);
    }
  }

  const int space = class_name.lastIndexOf(' ');
  if (space != -1) {
    class_name = class_name.mid(space+1);
  }

  return class_name;
}

QDebug CreateLogger(Level level, const QString& class_name, int line) {
  // Map the level to a string
  const char* level_name = NULL;
  switch (level) {
    case Level_Debug:   level_name = " DEBUG "; break;
    case Level_Info:    level_name = " INFO  "; break;
    case Level_Warning: level_name = " WARN  "; break;
    case Level_Error:   level_name = " ERROR "; break;
  }

  // Check the settings to see if we're meant to show or hide this message.
  Level threshold_level = sDefaultLevel;
  if (sClassLevels && sClassLevels->contains(class_name)) {
    threshold_level = sClassLevels->value(class_name);
  }

  if (level > threshold_level) {
    return QDebug(sNullDevice);
  }

  QString function_line = class_name;
  if (line != -1) {
    function_line += ":" + QString::number(line);
  }

  QDebug ret(QtDebugMsg);
  ret.nospace() << kMessageHandlerMagic
      << QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toAscii().constData()
      << level_name << function_line.leftJustified(32).toAscii().constData();

  return ret.space();
}

} // namespace logging
