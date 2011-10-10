/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef IMAGESTATE_H
#define IMAGESTATE_H

#include "image.h"

#include <QFrame>
#include <QScopedPointer>

class Application;
class Ui_ImageState;

class QWebView;

class ImageState : public QFrame {
  Q_OBJECT

public:
  ImageState(QWidget* parent = 0);
  ~ImageState();

  void Init(Application* app, QWebView* web_view);

public slots:
  void SetCurrentImage(const Image& image);
  void ToggleLove();

private slots:
  void ReloadSettings();
  void Love(bool on);

private:
  QString SaveCurrentImage(const QUrl& url) const;

private:
  QScopedPointer<Ui_ImageState> ui_;
  Application* app_;
  QWebView* web_view_;

  // Settings
  QString loved_folder_;
  QString findimages_js_source_;

  // State
  Image current_image_;
};

#endif // IMAGESTATE_H
