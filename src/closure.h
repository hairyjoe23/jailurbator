/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef CLOSURE_H
#define CLOSURE_H

#include <tr1/functional>

#include <QMetaMethod>
#include <QObject>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "logging.h"

class ClosureArgumentWrapper {
 public:
  virtual ~ClosureArgumentWrapper() {}

  virtual QGenericArgument arg() const = 0;
};

template<typename T>
class ClosureArgument : public ClosureArgumentWrapper {
 public:
  ClosureArgument(const T& data) : data_(data) {}

  virtual QGenericArgument arg() const {
    return Q_ARG(T, data_);
  }

 private:
  T data_;
};

class Closure : public QObject, boost::noncopyable {
  Q_OBJECT

 public:
  Closure(QObject* sender, const char* signal,
          QObject* receiver, const char* slot,
          const ClosureArgumentWrapper* val0 = 0,
          const ClosureArgumentWrapper* val1 = 0);

  Closure(QObject* sender, const char* signal,
          std::tr1::function<void()> callback);

 private slots:
  void Invoked();
  void Cleanup();

 private:
  void Connect(QObject* sender, const char* signal);

  QMetaMethod slot_;
  std::tr1::function<void()> callback_;

  boost::scoped_ptr<const ClosureArgumentWrapper> val0_;
  boost::scoped_ptr<const ClosureArgumentWrapper> val1_;
};

#define C_ARG(type, data) new ClosureArgument<type>(data)

Closure* NewClosure(
    QObject* sender,
    const char* signal,
    QObject* receiver,
    const char* slot);

template <typename T>
Closure* NewClosure(
    QObject* sender,
    const char* signal,
    QObject* receiver,
    const char* slot,
    const T& val0) {
  return new Closure(
      sender, signal, receiver, slot,
      C_ARG(T, val0));
}

template <typename T0, typename T1>
Closure* NewClosure(
    QObject* sender,
    const char* signal,
    QObject* receiver,
    const char* slot,
    const T0& val0,
    const T1& val1) {
  return new Closure(
      sender, signal, receiver, slot,
      C_ARG(T0, val0), C_ARG(T1, val1));
}

#endif  // CLOSURE_H
