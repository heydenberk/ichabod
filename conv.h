#ifndef CONVERTER_H
#define CONVERTER_H

#include "progressfeedback.hh"
#include <wkhtmltox/imagesettings.hh>
#include <wkhtmltox/imageconverter.hh>
#include <wkhtmltox/utilities.hh>
#include <QWebPage>

struct IchabodSettings : public wkhtmltopdf::settings::ImageGlobal 
{
    int verbosity;
    QString rasterizer;
};

class IchabodConverter: public wkhtmltopdf::ImageConverter 
{
    Q_OBJECT
public:
    IchabodConverter(IchabodSettings & settings, const QString * data=NULL);
    ~IchabodConverter();

    QString convert();
public slots:
    void setTransparent( bool t );
    void setQuality( int q );
    void setScreen( int w, int h );
    void setFormat( const QString& fmt );
    void snapshotPage( int msec_delay = 100);
    void saveToOutput();

private slots:
    void slotJavascriptEnvironment(QWebPage* page);
private:
    void debugSettings(bool success_status);
    IchabodSettings m_settings;
    QWebPage* m_activePage;
    QVector<QImage> m_images;
    QVector<int> m_delays;
};

#endif
