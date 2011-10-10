/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SCOPEDTRANSACTION_H
#define SCOPEDTRANSACTION_H

#include <boost/noncopyable.hpp>

class QSqlDatabase;

// Opens a transaction on a database.
// Rolls back the transaction if the object goes out of scope before Commit()
// is called.
class ScopedTransaction : boost::noncopyable {
 public:
  ScopedTransaction(QSqlDatabase* db);
  ~ScopedTransaction();

  void Commit();

 private:
  QSqlDatabase* db_;
  bool pending_;
};

#endif // SCOPEDTRANSACTION_H
