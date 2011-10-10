/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "database.h"
#include "logging.h"
#include "scopedtransaction.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

const int Database::kSchemaVersion = 1;


Database::Database(QObject* parent, const QString& filename)
  : QObject(parent),
    filename_(filename)
{
  if (filename_.isNull()) {
    filename_ = QString("%1/%2.db").arg(
          QDesktopServices::storageLocation(QDesktopServices::DataLocation),
          QCoreApplication::applicationName());
  }
}

void Database::Open() {
  qLog(Debug) << "Using database" << filename_;

  // Create the connection
  QString database_name = QString::number(reinterpret_cast<qlonglong>(this), 16);
  db_ = QSqlDatabase::addDatabase("QSQLITE", database_name);

  // Create the directories if they don't exist already
  QDir dir;
  dir.mkpath(QFileInfo(filename_).dir().path());

  // Open the database
  db_.setDatabaseName(filename_);
  if (!db_.open()) {
    qLog(Error) << "Error opening database" << db_.lastError().text();
    return;
  }

  // Update schema
  int current_version = -1;

  QSqlQuery q("select version from schema_version", db_);
  q.exec();
  if (q.next()) {
    current_version = q.value(0).toInt();
  }

  for (++current_version ; current_version <= kSchemaVersion ; ++current_version) {
    ApplySchemaUpdate(current_version);
  }
}

void Database::ApplySchemaUpdate(int version) {
  QString filename = QString(":/schema/%1.sql").arg(version);
  qLog(Debug) << "Updating to schema version" << version;

  ScopedTransaction t(&db_);
  ExecFromFile(filename);
  t.Commit();
}

void Database::ExecFromFile(const QString& filename) {
  // Open and read the database schema
  QFile schema_file(filename);
  if (!schema_file.open(QIODevice::ReadOnly))
    qFatal("Couldn't open schema file %s", filename.toUtf8().constData());
  ExecCommands(QString::fromUtf8(schema_file.readAll()));
}

void Database::ExecCommands(const QString& schema) {
  // Run each command
  QStringList commands(schema.split(";\n"));

  foreach (const QString& command, commands) {
    QSqlQuery query(db_.exec(command));
    if (!CheckErrors(query))
      qFatal("Failed to execute update commands");
  }
}

bool Database::CheckErrors(const QSqlQuery& query) {
  if (query.lastError().isValid()) {
    qLog(Error) << "Error executing query" << query.lastQuery();
    qLog(Error) << "  " << query.lastError().text();
    return false;
  }
  return true;
}
