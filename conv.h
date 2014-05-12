#ifndef CONVERTER_H
#define CONVERTER_H

#include "progressfeedback.hh"
#include <wkhtmltox/imagesettings.hh>
#include <wkhtmltox/imageconverter.hh>
#include <wkhtmltox/utilities.hh>
#include <QWebPage>
#include <utility>

struct IchabodSettings : public wkhtmltopdf::settings::ImageGlobal 
{
    int verbosity;
    QString rasterizer;
};

class IchabodConverter: public wkhtmltopdf::ImageConverter 
{
    Q_OBJECT
public:
    IchabodConverter(IchabodSettings & settings, const QString * data=0);
    ~IchabodConverter();

    std::pair<QString,QVector<QString> > convert();
public slots:
    void setTransparent( bool t );
    void setQuality( int q );
    void setScreen( int w, int h );
    void setFormat( const QString& fmt );
    void snapshotPage( int msec_delay = 100);
    void saveToOutput();

private slots:
    void slotJavascriptEnvironment(QWebPage* page);
    void slotJavascriptWarning(QString s);
private:
    void debugSettings(bool success_status);
    IchabodSettings m_settings;
    QWebPage* m_activePage;
    QVector<QImage> m_images;
    QVector<int> m_delays;
    QVector<QString> m_warnings;
};

#endif
