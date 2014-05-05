#ifndef CONVERTER_H
#define CONVERTER_H

#include "progressfeedback.hh"
#include <wkhtmltox/imagesettings.hh>
#include <wkhtmltox/imageconverter.hh>
#include <wkhtmltox/utilities.hh>
#include <QWebPage>

class IchabodConverter: public wkhtmltopdf::ImageConverter {
    Q_OBJECT
public:
    IchabodConverter(wkhtmltopdf::settings::ImageGlobal & settings, const QString * data=NULL);
    ~IchabodConverter();

    QString convert( int verbosity, const wkhtmltopdf::settings::ImageGlobal& settings );
public slots:
    void setTransparent( bool t );
    void setQuality( int q );
    void setScreen( int w, int h );
    void setFormat( const QString& fmt );
    void snapshotPage();
    void saveToOutput();

private slots:
    void slotJavascriptEnvironment(QWebPage* page);
private:
    wkhtmltopdf::settings::ImageGlobal m_settings;
    QWebPage* m_activePage;
    QVector<QImage> m_images;
};

#endif
