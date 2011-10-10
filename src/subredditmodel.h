/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SUBREDDITMODEL_H
#define SUBREDDITMODEL_H

#include <QAbstractItemModel>
#include <QSet>

class SubredditModel : public QAbstractItemModel {
public:
  SubredditModel(QObject* parent = 0);

  static const char* kSettingsGroup;

  enum Column {
    Column_Name = 0,
    Column_Subscribers,

    ColumnCount
  };

  void ReloadSettings();

  static QStringList default_subreddits();
  QStringList subscribed_subreddits() const;

  // QAbstractItemModel
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex& child) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);

private:
  struct Subreddit {
    QString name_;
    int subscribers_;
  };

  void LoadSubredditList();

private:
  QList<Subreddit> subreddits_;
  QSet<QString> subscribed_subreddits_;
};

#endif // SUBREDDITMODEL_H
