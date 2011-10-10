/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "closure.h"

Closure::Closure(QObject* sender,
                 const char* signal,
                 QObject* receiver,
                 const char* slot,
                 const ClosureArgumentWrapper* val0,
                 const ClosureArgumentWrapper* val1)
    : QObject(receiver),
      callback_(NULL),
      val0_(val0),
      val1_(val1) {
  const QMetaObject* meta_receiver = receiver->metaObject();

  QByteArray normalised_slot = QMetaObject::normalizedSignature(slot + 1);
  const int index = meta_receiver->indexOfSlot(normalised_slot.constData());
  Q_ASSERT(index != -1);
  slot_ = meta_receiver->method(index);

  Connect(sender, signal);
}

Closure::Closure(QObject* sender,
                 const char* signal,
                 std::tr1::function<void()> callback)
    : callback_(callback) {
  Connect(sender, signal);
}

void Closure::Connect(QObject* sender, const char* signal) {
  bool success = connect(sender, signal, SLOT(Invoked()));
  Q_ASSERT(success);
  success = connect(sender, SIGNAL(destroyed()), SLOT(Cleanup()));
  Q_ASSERT(success);
  Q_UNUSED(success);
}

void Closure::Invoked() {
  if (callback_) {
    callback_();
  } else {
    slot_.invoke(
        parent(),
        val0_ ? val0_->arg() : QGenericArgument(),
        val1_ ? val1_->arg() : QGenericArgument());
  }
  deleteLater();
}

void Closure::Cleanup() {
  disconnect();
  deleteLater();
}

Closure* NewClosure(
    QObject* sender, const char* signal,
    QObject* receiver, const char* slot) {
  return new Closure(sender, signal, receiver, slot);
}
