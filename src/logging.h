/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef LOGGING_H
#define LOGGING_H

#include <QDebug>

#ifdef QT_NO_DEBUG_STREAM
#  define qLog(level) while (false) QNoDebug()
#else
#  define qLog(level) \
     logging::CreateLogger(logging::Level_##level, \
         logging::ParsePrettyFunction(__PRETTY_FUNCTION__), __LINE__)
#endif

namespace logging {
  class NullDevice : public QIODevice {
  protected:
    qint64 readData(char*, qint64) { return -1; }
    qint64 writeData(const char*, qint64 len) { return len; }
  };

  enum Level {
    Level_Error = 0,
    Level_Warning,
    Level_Info,
    Level_Debug,
  };

  void Init();
  void SetLevels(const QString& levels);

  QString ParsePrettyFunction(const char* pretty_function);
  QDebug CreateLogger(Level level, const QString& class_name, int line);

  void GLog(const char* domain, int level, const char* message, void* user_data);

  extern const char* kDefaultLogLevels;
}

#endif // LOGGING_H
