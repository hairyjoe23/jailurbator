/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef NETWORK_H
#define NETWORK_H

#include <QAbstractNetworkCache>
#include <QMutex>
#include <QNetworkAccessManager>

class QNetworkDiskCache;

class ThreadSafeNetworkDiskCache : public QAbstractNetworkCache {
public:
  ThreadSafeNetworkDiskCache(QObject* parent);

  static void SetMaximumCacheSize(qint64 bytes);
  static QIODevice* Data(const QUrl& url);

  // QAbstractNetworkCache
  qint64 cacheSize() const;
  QIODevice* data(const QUrl& url);
  void insert(QIODevice* device);
  QNetworkCacheMetaData metaData(const QUrl& url);
  QIODevice* prepare(const QNetworkCacheMetaData& metaData);
  bool remove(const QUrl& url);
  void updateMetaData(const QNetworkCacheMetaData& metaData);
  void clear();

private:
  static QMutex sMutex;
  static QNetworkDiskCache* sCache;
};

class NetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT

public:
  NetworkAccessManager(QObject* parent = 0);

protected:
  QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                               QIODevice* outgoingData);
};

class NetworkTimeouts : public QObject {
  Q_OBJECT

public:
  NetworkTimeouts(int timeout_msec, QObject* parent = 0);

  void AddReply(QNetworkReply* reply);
  void SetTimeout(int msec) { timeout_msec_ = msec; }

protected:
  void timerEvent(QTimerEvent* e);

private slots:
  void ReplyFinished();

private:
  int timeout_msec_;
  QMap<QNetworkReply*, int> timers_;
};

#endif // NETWORK_H
