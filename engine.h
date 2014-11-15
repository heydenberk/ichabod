#include <QObject>
#include <QString>
#include <QRect>
#include <QSet>
#include <QUrl>
#include <QWebPage>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include "statsd_client.h"
#include <string>

class Settings
{
public:
    Settings();
    QString in;

    int min_font_size;

    int verbosity;
    QString rasterizer;
    bool looping;
    //QuantizeMethod quantize_method;
    QRect crop_rect;
    QString css;
    QString selector;
    int slow_response_ms;
    std::string statsd_ns; // interop with statsd code
    statsd::StatsdClient* statsd;
};

class NetAccess: public QNetworkAccessManager 
{
    Q_OBJECT
public:
    NetAccess(const Settings& s);
    ~NetAccess();
private:
    Settings settings;
public:
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);
signals:
    void warning(const QString & text);
};

class WebPage: public QWebPage 
{
    Q_OBJECT
public:
    WebPage();
    ~WebPage();
    virtual void javaScriptAlert(QWebFrame * frame, const QString & msg);
    virtual bool javaScriptConfirm(QWebFrame * frame, const QString & msg);
    virtual bool javaScriptPrompt(QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result);
    virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
    virtual bool supportsExtension ( Extension extension ) const;
    virtual bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
public slots:
    bool shouldInterruptJavaScript();

signals:
    void alert(const QString& msg);
    void confirm(const QString& msg);
    void prompt(const QString& msg);
    void console(const QString& msg);
};

class Engine : public QObject 
{
    Q_OBJECT
public:
    Engine( const Settings & settings );
    virtual ~Engine();
    QString scriptResult() const;
signals:
    void warning(const QString & message);
    void error(const QString & message);
    void scriptResult(const QString& result);
    void javascriptEnvironment(QWebPage* page);
    void progressChanged(int progress);
    void finished(bool ok);

public slots:
    bool convert();
    //void cancel();

    void netSslErrors(QNetworkReply *reply, const QList<QSslError> &);
    void netFinished(QNetworkReply * reply);
    void netWarning(const QString & message);

    void webPageLoadStarted();
    void webPageLoadFinished(bool b);

private:
    void setWebSettings(QWebSettings * ws);
    WebPage web_page;
    Settings settings;
    QString script_result;
    QEventLoop event_loop;
};
