/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "database.h"
#include "iconloader.h"
#include "imagebackend.h"

Application::Application()
  : database_(new Database),
    icon_loader_(new IconLoader),
    image_backend_(new ImageBackend(database_)),
    settings_dialog_(NULL)
{
  database_->Open();
}

Application::~Application() {
  delete image_backend_;
  delete database_;
}

SettingsDialog* Application::settings_dialog() {
  if (!settings_dialog_) {
    settings_dialog_ = new SettingsDialog;
    connect(settings_dialog_, SIGNAL(accepted()), SIGNAL(SettingsChanged()));
  }

  return settings_dialog_;
}

void Application::ShowSettingsDialog() {
  settings_dialog()->show();
}

void Application::ShowSettingsDialogAtPage(SettingsDialog::Page page) {
  settings_dialog()->OpenAtPage(page);
}
