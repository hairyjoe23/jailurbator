/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SUBREDDITSETTINGSPAGE_H
#define SUBREDDITSETTINGSPAGE_H

#include "settingspage.h"

class SubredditModel;
class Ui_SubredditSettingsPage;

class QSortFilterProxyModel;

class SubredditSettingsPage : public SettingsPage {
  Q_OBJECT

public:
  SubredditSettingsPage(SettingsDialog* dialog);
  ~SubredditSettingsPage();

  void Load();
  void Save();

private:
  Ui_SubredditSettingsPage* ui_;

  SubredditModel* model_;
  QSortFilterProxyModel* proxy_;
};

#endif // SUBREDDITSETTINGSPAGE_H
