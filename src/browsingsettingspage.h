/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef BROWSINGSETTINGSPAGE_H
#define BROWSINGSETTINGSPAGE_H

#include "settingspage.h"

class Ui_BrowsingSettingsPage;

class BrowsingSettingsPage : public SettingsPage {
  Q_OBJECT

public:
  BrowsingSettingsPage(SettingsDialog* dialog);
  ~BrowsingSettingsPage();

  void Load();
  void Save();

private slots:
  void BrowseLoved();

private:
  Ui_BrowsingSettingsPage* ui_;
};

#endif // BROWSINGSETTINGSPAGE_H
