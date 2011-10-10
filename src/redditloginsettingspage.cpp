/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "closure.h"
#include "logging.h"
#include "network.h"
#include "redditloginsettingspage.h"
#include "redditmodel.h"
#include "ui_redditloginsettingspage.h"

#include <QMessageBox>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

#include <qjson/parser.h>

const char* RedditLoginSettingsPage::kLoginUrl = "http://www.reddit.com/api/login/%1";


RedditLoginSettingsPage::RedditLoginSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_RedditLoginSettingsPage),
    network_(new NetworkAccessManager)
{
  ui_->setupUi(this);

  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->logout, SIGNAL(clicked()), SLOT(Logout()));

  // Do the busy indicator
  QMovie* movie = new QMovie(":/busyindicator.gif", QByteArray(), this);
  ui_->busyindicator->setMovie(movie);
  ui_->busyindicator->setMaximumSize(QSize(50, 50));
}

RedditLoginSettingsPage::~RedditLoginSettingsPage() {
}

void RedditLoginSettingsPage::Load() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);

  QString username = s.value("username").toString();
  ui_->username->setText(username);

  if (username.isEmpty()) {
    ui_->stack->setCurrentWidget(ui_->login_page);
  } else {
    ui_->stack->setCurrentWidget(ui_->logout_page);
    UpdateLogoutText(username);
  }
}

void RedditLoginSettingsPage::UpdateLogoutText(const QString& username) {
  ui_->logout_text->setText(QString("You are logged in as <b>%1</b>").arg(username));
}

void RedditLoginSettingsPage::Login() {
  // Construct the URL
  QUrl url(QString(kLoginUrl).arg(ui_->username->text()));
  url.addQueryItem("user", ui_->username->text());
  url.addQueryItem("passwd", ui_->password->text());
  url.addQueryItem("api_type", "json");

  QByteArray post_data = url.encodedQuery();
  url.setEncodedQuery(QByteArray());

  qLog(Debug) << url << post_data;

  // Send the request
  QNetworkRequest request(url);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferNetwork);
  QNetworkReply* reply = network_->post(request, post_data);
  NewClosure(reply, SIGNAL(finished()),
             this,  SLOT(LoginFinished(QNetworkReply*)),
             reply);

  // Show the authenticating page
  ui_->busyindicator->movie()->start();
  ui_->stack->setCurrentWidget(ui_->authenticating_page);
}

void RedditLoginSettingsPage::LoginFinished(QNetworkReply* reply) {
  ui_->busyindicator->movie()->stop();

  if (reply->error() != QNetworkReply::NoError) {
    QMessageBox::warning(this, "Error connecting to Reddit",
        "Reddit is probably under heavy load, try again in a few minutes.",
        QMessageBox::Close, QMessageBox::NoButton);

    ui_->stack->setCurrentWidget(ui_->login_page);
    return;
  }

  // Parse the JSON response
  QJson::Parser parser;
  QVariantMap data = parser.parse(reply).toMap();

  if (!data.contains("json") || !data["json"].toMap().contains("errors")) {
    qLog(Warning) << "Invalid JSON response from" << reply->url() << data;
    ui_->stack->setCurrentWidget(ui_->login_page);
    return;
  }

  QVariantMap json = data["json"].toMap();
  QVariantList errors = json["errors"].toList();

  if (!errors.isEmpty()) {
    qLog(Warning) << errors;

    // There was an actual error - this is usually because of bad credentials.
    QMessageBox::warning(this, "Error logging in",
        "Invalid username or password.",
        QMessageBox::Close, QMessageBox::NoButton);

    ui_->stack->setCurrentWidget(ui_->login_page);
    return;
  }

  if (!json.contains("data") || !json["data"].toMap().contains("cookie")) {
    qLog(Warning) << "JSON response from" << reply->url() << "didn't contain cookie" << data;
    ui_->stack->setCurrentWidget(ui_->login_page);
    return;
  }

  // Save the session cookie
  const QString cookie = json["data"].toMap()["cookie"].toString();

  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);
  s.setValue("username", ui_->username->text());
  s.setValue("cookie", cookie);

  UpdateLogoutText(ui_->username->text());
  ui_->stack->setCurrentWidget(ui_->logout_page);
}

void RedditLoginSettingsPage::Logout() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);
  s.remove("username");
  s.remove("cookie");

  ui_->username->clear();
  ui_->password->clear();
  ui_->stack->setCurrentWidget(ui_->login_page);
}

void RedditLoginSettingsPage::Save() {
}
