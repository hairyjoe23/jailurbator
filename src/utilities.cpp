/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "utilities.h"
#include "logging.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QIODevice>
#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>
#include <QtDebug>
#include <QtGlobal>

#if defined(Q_OS_UNIX)
#  include <sys/statvfs.h>
#elif defined(Q_OS_WIN32)
#  include <windows.h>
#endif

#ifdef Q_OS_DARWIN
#  include "core/mac_startup.h"
#endif

#include <boost/scoped_array.hpp>

namespace Utilities {

static QString tr(const char* str) {
  return QCoreApplication::translate("", str);
}

QString PrettyTimeDelta(int seconds) {
  return (seconds >= 0 ? "+" : "-") + PrettyTime(seconds);
}

QString PrettyTime(int seconds) {
  // last.fm sometimes gets the track length wrong, so you end up with
  // negative times.
  seconds = qAbs(seconds);

  int hours = seconds / (60*60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours)
    ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
  else
    ret.sprintf("%d:%02d", minutes, seconds);

  return ret;
}

QString WordyTime(quint64 seconds) {
  quint64 days = seconds / (60*60*24);

  // TODO: Make the plural rules translatable
  QStringList parts;

  if (days)
    parts << (days == 1 ? tr("1 day") : tr("%1 days").arg(days));
  parts << PrettyTime(seconds - days*60*60*24);

  return parts.join(" ");
}

QString Ago(int seconds_since_epoch, const QLocale& locale) {
  const QDateTime now = QDateTime::currentDateTime();
  const QDateTime then = QDateTime::fromTime_t(seconds_since_epoch);
  const int days_ago = then.date().daysTo(now.date());
  const QString time = then.time().toString(locale.timeFormat(QLocale::ShortFormat));

  if (days_ago == 0)
    return tr("Today") + " " + time;
  if (days_ago == 1)
    return tr("Yesterday") + " " + time;
  if (days_ago <= 7)
    return tr("%1 days ago").arg(days_ago);

  return then.date().toString(locale.dateFormat());
}

QString PrettySize(quint64 bytes) {
  QString ret;

  if (bytes > 0) {
    if (bytes <= 1000)
      ret = QString::number(bytes) + " bytes";
    else if (bytes <= 1000*1000)
      ret.sprintf("%.1f KB", float(bytes) / 1000);
    else if (bytes <= 1000*1000*1000)
      ret.sprintf("%.1f MB", float(bytes) / (1000*1000));
    else
      ret.sprintf("%.1f GB", float(bytes) / (1000*1000*1000));
  }
  return ret;
}

quint64 FileSystemCapacity(const QString& path) {
#if defined(Q_OS_UNIX)
  struct statvfs fs_info;
  if (statvfs(path.toLocal8Bit().constData(), &fs_info) == 0)
    return quint64(fs_info.f_blocks) * quint64(fs_info.f_bsize);
#elif defined(Q_OS_WIN32)
  _ULARGE_INTEGER ret;
  if (GetDiskFreeSpaceEx(QDir::toNativeSeparators(path).toLocal8Bit().constData(),
                         NULL, &ret, NULL) != 0)
    return ret.QuadPart;
#endif

  return 0;
}

quint64 FileSystemFreeSpace(const QString& path) {
#if defined(Q_OS_UNIX)
  struct statvfs fs_info;
  if (statvfs(path.toLocal8Bit().constData(), &fs_info) == 0)
    return quint64(fs_info.f_bavail) * quint64(fs_info.f_bsize);
#elif defined(Q_OS_WIN32)
  _ULARGE_INTEGER ret;
  if (GetDiskFreeSpaceEx(QDir::toNativeSeparators(path).toLocal8Bit().constData(),
                         &ret, NULL, NULL) != 0)
    return ret.QuadPart;
#endif

  return 0;
}

QString MakeTempDir(const QString template_name) {
  QString path;
  {
    QTemporaryFile tempfile;
    if (!template_name.isEmpty())
      tempfile.setFileTemplate(template_name);

    tempfile.open();
    path = tempfile.fileName();
  }

  QDir d;
  d.mkdir(path);

  return path;
}

void RemoveRecursive(const QString& path) {
  QDir dir(path);
  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden))
    RemoveRecursive(path + "/" + child);

  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden))
    QFile::remove(path + "/" + child);

  dir.rmdir(path);
}

bool CopyRecursive(const QString& source, const QString& destination) {
  // Make the destination directory
  QString dir_name = source.section('/', -1, -1);
  QString dest_path = destination + "/" + dir_name;
  QDir().mkpath(dest_path);

  QDir dir(source);
  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
    if (!CopyRecursive(source + "/" + child, dest_path)) {
      qLog(Warning) << "Failed to copy dir" << source + "/" + child << "to" << dest_path;
      return false;
    }
  }

  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
    if (!QFile::copy(source + "/" + child, dest_path + "/" + child)) {
      qLog(Warning) << "Failed to copy file" << source + "/" + child << "to" << dest_path;
      return false;
    }
  }
  return true;
}

bool Copy(QIODevice* source, QIODevice* destination) {
  if (!source->open(QIODevice::ReadOnly))
    return false;

  if (!destination->open(QIODevice::WriteOnly))
    return false;

  const qint64 bytes = source->size();
  boost::scoped_array<char> data(new char[bytes]);
  qint64 pos = 0;

  qint64 bytes_read;
  do {
    bytes_read = source->read(data.get() + pos, bytes - pos);
    if (bytes_read == -1)
      return false;

    pos += bytes_read;
  } while (bytes_read > 0 && pos != bytes);

  pos = 0;
  qint64 bytes_written;
  do {
    bytes_written = destination->write(data.get() + pos, bytes - pos);
    if (bytes_written == -1)
      return false;

    pos += bytes_written;
  } while (bytes_written > 0 && pos != bytes);

  return true;
}

QString ColorToRgba(const QColor& c) {
  return QString("rgba(%1, %2, %3, %4)")
      .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

void OpenInFileBrowser(const QStringList& filenames) {
  QSet<QString> dirs;

  foreach (const QString& filename, filenames) {
    // Ignore things that look like URLs
    if (filename.contains("://"))
      continue;

    if (!QFile::exists(filename))
      continue;

    const QString directory = QFileInfo(filename).dir().path();

    if (dirs.contains(directory))
      continue;
    dirs.insert(directory);

    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
  }
}

QString PrettySize(const QSize& size) {
  return QString::number(size.width()) + "x" +
         QString::number(size.height());
}

}  // namespace Utilities


ScopedWCharArray::ScopedWCharArray(const QString& str)
  : chars_(str.length()),
    data_(new wchar_t[chars_ + 1])
{
  str.toWCharArray(data_.get());
  data_[chars_] = '\0';
}
