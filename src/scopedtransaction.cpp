/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "scopedtransaction.h"
#include "logging.h"

#include <QSqlDatabase>
#include <QtDebug>

ScopedTransaction::ScopedTransaction(QSqlDatabase* db)
  : db_(db),
    pending_(true)
{
  db->transaction();
}

ScopedTransaction::~ScopedTransaction() {
  if (pending_) {
    qLog(Warning) << "Rolling back transaction";
    db_->rollback();
  }
}

void ScopedTransaction::Commit() {
  if (!pending_) {
    qLog(Warning) << "Tried to commit a ScopedTransaction twice";
    return;
  }

  db_->commit();
  pending_ = false;
}
