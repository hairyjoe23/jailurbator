/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QColor>
#include <QLocale>
#include <QCryptographicHash>
#include <QSize>
#include <QString>

#include <boost/scoped_array.hpp>

class QIODevice;

namespace Utilities {
  QString PrettyTime(int seconds);
  QString PrettyTimeDelta(int seconds);
  QString PrettySize(quint64 bytes);
  QString PrettySize(const QSize& size);
  QString WordyTime(quint64 seconds);
  QString Ago(int seconds_since_epoch, const QLocale& locale);

  QString ColorToRgba(const QColor& color);

  quint64 FileSystemCapacity(const QString& path);
  quint64 FileSystemFreeSpace(const QString& path);

  QString MakeTempDir(const QString template_name = QString());
  void RemoveRecursive(const QString& path);
  bool CopyRecursive(const QString& source, const QString& destination);
  bool Copy(QIODevice* source, QIODevice* destination);

  void OpenInFileBrowser(const QStringList& filenames);
}

class ScopedWCharArray {
public:
  ScopedWCharArray(const QString& str);

  QString ToString() const { return QString::fromWCharArray(data_.get()); }

  wchar_t* get() const { return data_.get(); }
  operator wchar_t*() const { return get(); }

  int characters() const { return chars_; }
  int bytes() const { return (chars_ + 1) * sizeof(wchar_t); }

private:
  Q_DISABLE_COPY(ScopedWCharArray);

  int chars_;
  boost::scoped_array<wchar_t> data_;
};

#endif // UTILITIES_H
