#include "engine.h"
#include <iostream>
#include <QApplication>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebFrame>
#include <QNetworkCookieJar>

#include <QPainter>
#include <QImage>

Settings::Settings()
{
    in = "";



    min_font_size = -1;
    verbosity = 0;
    rasterizer = "ichabod";
    looping = false;
    //quantize_method = ;
    crop_rect = QRect();
    css = "";
    selector = "";
    slow_response_ms = 15000;
    statsd_ns = "ichabod";
    statsd = 0;
}

///////

NetAccess::NetAccess(const Settings& s)
    : settings(s)
{
}

NetAccess::~NetAccess()
{
}

QNetworkReply* NetAccess::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData) 
{
    // block things here if necessary
    return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

//////

WebPage::WebPage()
    : QWebPage()
{
}

WebPage::~WebPage()
{
}

void WebPage::javaScriptAlert(QWebFrame *, const QString & msg) 
{
    emit alert(QString("Javascript alert: %1").arg(msg));
}

bool WebPage::javaScriptConfirm(QWebFrame *, const QString & msg) 
{
    emit confirm (QString("Javascript confirm: %1").arg(msg));
    return true;
}

bool WebPage::javaScriptPrompt(QWebFrame *, const QString & msg, const QString & defaultValue, QString * result) 
{
    emit prompt (QString("Javascript prompt: %1 (%2)").arg(msg,defaultValue));
    result = (QString*)&defaultValue;
    return true;
}

void WebPage::javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID) 
{
    emit console(QString("%1:%2 %3").arg(sourceID).arg(lineNumber).arg(message));
}

bool WebPage::shouldInterruptJavaScript() 
{
    emit alert("Javascript alert: slow javascript");
    return false;
}

bool WebPage::supportsExtension ( Extension extension ) const
{
    if (extension == QWebPage::ErrorPageExtension)
    {
        return true;
    }
    return false;
}

bool WebPage::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    if (extension != QWebPage::ErrorPageExtension)
        return false;
    ErrorPageExtensionOption *errorOption = (ErrorPageExtensionOption*) option;
    if(errorOption->domain == QWebPage::QtNetwork)
    {
        emit alert(QString("Network error (%1)").arg(errorOption->error));
    }
    else if(errorOption->domain == QWebPage::Http)
    {
        emit alert(QString("HTTP error (%1)").arg(errorOption->error));
    }
    else if(errorOption->domain == QWebPage::WebKit)
    {
        emit alert(QString("WebKit error (%1)").arg(errorOption->error));
    }
    return false;
}

//////

Engine::Engine(const Settings& s)
    : web_page(),
      settings(s),
      script_result("")
{
    QNetworkAccessManager* net_access = new NetAccess(settings);
    net_access->setCookieJar(new QNetworkCookieJar());
    connect(net_access, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            this, SLOT(netSslErrors(QNetworkReply*, const QList<QSslError>&)));
    connect(net_access, SIGNAL(finished (QNetworkReply *)),
            this, SLOT(netFinished (QNetworkReply *) ) );
    connect(net_access, SIGNAL(warning(const QString &)),
            this, SLOT(netWarning(const QString &)));

    web_page.setNetworkAccessManager(net_access);

    connect(&web_page, SIGNAL(loadStarted()), this, SLOT(webPageLoadStarted()));
    connect(web_page.mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(webPageLoadFinished(bool)));
}

Engine::~Engine()
{
}

void Engine::netSslErrors(QNetworkReply *reply, const QList<QSslError> &) {
    // ignore all
    reply->ignoreSslErrors();
    emit warning("SSL error ignored");
}

void Engine::netWarning(const QString & message)
{
    emit warning(QString("Network warning: %1").arg(message));
}

void Engine::netFinished(QNetworkReply * reply) 
{
    int networkStatus = reply->error();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if ((networkStatus != 0 && networkStatus != 5) || (httpStatus > 399))
    {
        emit warning(QString("Failed to load %1").arg(reply->url().toString()));    
    }
}

void Engine::webPageLoadStarted()
{
}

void Engine::webPageLoadFinished(bool b)
{
    event_loop.exit();

    web_page.setViewportSize(QSize(400, 800));
    
    QPainter painter;
    QImage image;

    QRect rect = QRect(QPoint(0,0), web_page.viewportSize());
    if (rect.width() == 0 || rect.height() == 0) 
    {
        // abort
    }

    image = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied); // draw as fast as possible
    painter.begin(&image);

    painter.fillRect(QRect(QPoint(0,0),web_page.viewportSize()), Qt::white);
    
    painter.translate(-rect.left(), -rect.top());
    web_page.mainFrame()->render(&painter);
    painter.end();

    image.save("/tmp/output.png");

}


QString Engine::scriptResult() const
{
    return script_result;
}

bool Engine::convert()
{
    QFileInfo info(settings.in);
    QUrl url = QUrl::fromLocalFile(info.absoluteFilePath());

    QNetworkRequest r = QNetworkRequest(url);

    web_page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    web_page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    web_page.mainFrame()->load(r);

    event_loop.exec();

    return true;
}

void Engine::setWebSettings(QWebSettings * ws)
{
    //ws->setPrintingMediaType("screen");
    ws->setAttribute(QWebSettings::JavaEnabled, true);
    ws->setAttribute(QWebSettings::JavascriptEnabled, true);
    ws->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    ws->setAttribute(QWebSettings::JavascriptCanAccessClipboard, false);
    ws->setFontSize(QWebSettings::MinimumFontSize, settings.min_font_size);
    ws->setAttribute(QWebSettings::PrintElementBackgrounds, true);
    ws->setAttribute(QWebSettings::AutoLoadImages, true);
    ws->setAttribute(QWebSettings::PluginsEnabled, false);
}
