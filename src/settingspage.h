/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class SettingsDialog;

class SettingsPage : public QWidget {
  Q_OBJECT

public:
  SettingsPage(SettingsDialog* dialog);

  // Return false to grey out the page's item in the list.
  virtual bool IsEnabled() const { return true; }

  // Load is called when the dialog is shown, Save when the user clicks OK.
  virtual void Load() = 0;
  virtual void Save() = 0;

  // The dialog that this page belongs to.
  SettingsDialog* dialog() const { return dialog_; }

private:
  SettingsDialog* dialog_;
};

#endif // SETTINGSPAGE_H
