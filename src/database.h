/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class Database : public QObject {
  Q_OBJECT

public:
  Database(QObject* parent = 0, const QString& filename = QString());

  static const int kSchemaVersion;

  void Open();
  QSqlDatabase& db() { return db_; }

  bool CheckErrors(const QSqlQuery& query);

private:
  void ApplySchemaUpdate(int version);

  void ExecFromFile(const QString& filename);
  void ExecCommands(const QString& schema);

private:
  QString filename_;

  QSqlDatabase db_;
};

#endif // DATABASE_H
