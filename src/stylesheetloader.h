/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QString>
#include <QPalette>
#include <QWidget>
#include <QMap>

class StyleSheetLoader : public QObject {
 public:
  StyleSheetLoader(QObject* parent = 0);

  // Sets the given stylesheet on the given widget.
  // If the stylesheet contains strings like %palette-[role], these get replaced
  // with actual palette colours.
  // The stylesheet is reloaded when the widget's palette changes.
  void SetStyleSheet(QWidget* widget, const QString& filename);

 protected:
  bool eventFilter(QObject* obj, QEvent* event);

 private:
  void UpdateStyleSheet(QWidget* widget);
  void ReplaceColor(QString* css, const QString& name, const QPalette& palette,
                    QPalette::ColorRole role) const;

 private:
  QMap<QWidget*, QString> filenames_;
};

#endif // STYLESHEETLOADER_H
