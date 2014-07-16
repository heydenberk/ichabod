#ifndef CONVERTER_H
#define CONVERTER_H

#include "progressfeedback.hh"
#include <wkhtmltox/imagesettings.hh>
#include <wkhtmltox/imageconverter.hh>
#include <wkhtmltox/utilities.hh>
#include <QWebPage>
#include <QRect>
#include <utility>
#include "quant.h"

struct IchabodSettings : public wkhtmltopdf::settings::ImageGlobal 
{
    int verbosity;
    QString rasterizer;
    bool looping;
    QuantizeMethod quantize_method;
    QRect crop_rect;
    QString css;
    QString selector;
};

class IchabodConverter: public wkhtmltopdf::ImageConverter 
{
    Q_OBJECT
public:
    IchabodConverter(IchabodSettings & settings, const QString * data=0);
    ~IchabodConverter();

    bool convert(QString& result, QVector<QString>& warnings, QVector<QString>& errors);
public slots:
    void setTransparent( bool t );
    void setQuality( int q );
    void setScreen( int w, int h );
    void setFormat( const QString& fmt );
    void setLooping( bool l );
    void snapshotPage( int msec_delay = 100);
    void snapshotElements( const QStringList& ids, int msec_delay = 100 );
    void saveToOutput();
    void setQuantizeMethod( const QString& method ); // see quant.h
    void setSelector( const QString& sel );
    void setCss( const QString& css );
    void setCropRect( int x, int y, int w, int h );
    void setSmartWidth( bool sw );

private slots:
    void slotJavascriptEnvironment(QWebPage* page);
    void slotJavascriptWarning(QString s);
    void slotJavascriptError(QString s);
private:
    void debugSettings(bool success_status);
    IchabodSettings m_settings;
    QWebPage* m_activePage;
    QVector<QImage> m_images;
    QVector<int> m_delays;
    QVector<QRect> m_crops;
    QVector<QString> m_warnings;
    QVector<QString> m_errors;
    void internalSnapshot( int msec_delay, const QRect& crop );
};

#endif
