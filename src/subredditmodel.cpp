/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "logging.h"
#include "subredditmodel.h"

#include <QFile>
#include <QSettings>
#include <QStringList>

#include <qjson/parser.h>

const char* SubredditModel::kSettingsGroup = "Subreddits";


SubredditModel::SubredditModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  LoadSubredditList();
}

void SubredditModel::LoadSubredditList() {
  QFile file(":/topreddits.json");
  file.open(QIODevice::ReadOnly);

  QJson::Parser parser;
  QVariantList data = parser.parse(&file).toList();

  beginResetModel();
  subreddits_.clear();
  foreach (const QVariant& subreddit_variant, data) {
    QVariantMap subreddit_map = subreddit_variant.toMap();

    Subreddit subreddit;
    subreddit.name_ = subreddit_map["name"].toString();
    subreddit.subscribers_ = subreddit_map["subs"].toInt();

    subreddits_ << subreddit;
  }
  endResetModel();
}

void SubredditModel::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  const QStringList subscribed_subreddits =
      s.value("subscribed_subreddits", default_subreddits()).toStringList();

  beginResetModel();
  subscribed_subreddits_ = QSet<QString>::fromList(subscribed_subreddits);
  endResetModel();
}

Qt::ItemFlags SubredditModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags ret = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  if (index.column() == 0)
    ret |= Qt::ItemIsUserCheckable;

  return ret;
}

QVariant SubredditModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  const Subreddit& subreddit = subreddits_[index.row()];

  switch (role) {
  case Qt::DisplayRole:
    switch (index.column()) {
    case Column_Name:
      return subreddit.name_;

    case Column_Subscribers:
      return subreddit.subscribers_;
    }
    break;

  case Qt::CheckStateRole:
    if (index.column() == Column_Name)
      return subscribed_subreddits_.contains(subreddit.name_) ?
            Qt::Checked : Qt::Unchecked;
    break;

  default:
    break;
  }

  return QVariant();
}

QVariant SubredditModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (section) {
  case Column_Name:
    return "Name";

  case Column_Subscribers:
    return "Subscribers";
  }

  return QVariant();
}

int SubredditModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;
  return subreddits_.count();
}

int SubredditModel::columnCount(const QModelIndex& parent) const {
  return ColumnCount;
}

bool SubredditModel::hasChildren(const QModelIndex& parent) const {
  if (parent.isValid())
    return false;
  return true;
}

QModelIndex SubredditModel::index(int row, int column, const QModelIndex& parent) const {
  if (row < 0 || row >= subreddits_.count() || column < 0 || column >= ColumnCount ||
      parent.isValid())
    return QModelIndex();
  return createIndex(row, column);
}

QModelIndex SubredditModel::parent(const QModelIndex& child) const {
  return QModelIndex();
}

bool SubredditModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || index.column() != 0 || role != Qt::CheckStateRole)
    return false;

  if (value.toInt() == Qt::Checked) {
    subscribed_subreddits_.insert(index.data(Qt::DisplayRole).toString());
  } else {
    subscribed_subreddits_.remove(index.data(Qt::DisplayRole).toString());
  }

  return true;
}

QStringList SubredditModel::default_subreddits() {
  return QStringList() << "jailbait";
}

QStringList SubredditModel::subscribed_subreddits() const {
  return subscribed_subreddits_.toList();
}
