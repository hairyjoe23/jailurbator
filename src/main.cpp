/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "logging.h"
#include "mainwindow.h"

#include <QApplication>


int main(int argc, char** argv) {
  QApplication::setOrganizationName("jailurbator");
  QApplication::setOrganizationDomain("jailurbator.org");
  QApplication::setApplicationName("jailurbator");
  QApplication::setApplicationVersion("1.0");

  logging::Init();

  QApplication a(argc, argv);

  Application app;
  MainWindow w(&app);
  w.showMaximized();

  return a.exec();
}
