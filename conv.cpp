#include "conv.h"
#include "agif.h"
#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QDateTime>
#include <iostream>
#include <algorithm> 
#include <time.h>

// overload to allow QString output
std::ostream& operator<<(std::ostream& str, const QString& string) 
{
    return str << string.toLocal8Bit().constData();
}


IchabodConverter::IchabodConverter(IchabodSettings & s, const QString * data) 
    : wkhtmltopdf::ImageConverter(s,data)
    , m_settings(s)
    , m_activePage(0)
{
    connect(this, SIGNAL(javascriptEnvironment(QWebPage*)), this, SLOT(slotJavascriptEnvironment(QWebPage*)));
    connect(this, SIGNAL(warning(QString)), this, SLOT(slotJavascriptWarning(QString)));
    connect(this, SIGNAL(error(QString)), this, SLOT(slotJavascriptError(QString)));

    QObject::connect(this, SIGNAL(checkboxSvgChanged(const QString &)), qApp->style(), SLOT(setCheckboxSvg(const QString &)));
    QObject::connect(this, SIGNAL(checkboxCheckedSvgChanged(const QString &)), qApp->style(), SLOT(setCheckboxCheckedSvg(const QString &)));
    QObject::connect(this, SIGNAL(radiobuttonSvgChanged(const QString &)), qApp->style(), SLOT(setRadioButtonSvg(const QString &)));
    QObject::connect(this, SIGNAL(radiobuttonCheckedSvgChanged(const QString &)), qApp->style(), SLOT(setRadioButtonCheckedSvg(const QString &)));
}

IchabodConverter::~IchabodConverter()
{
}

void IchabodConverter::setTransparent( bool t )
{
    m_settings.transparent = t;
}

void IchabodConverter::setQuality( int q )
{
    m_settings.quality = q;
}

void IchabodConverter::setScreen( int w, int h )
{
    m_settings.screenWidth = w;
    m_settings.screenHeight = h;
}

void IchabodConverter::setFormat( const QString& fmt )
{
    m_settings.fmt = fmt;
}

void IchabodConverter::setLooping( bool l )
{
    m_settings.looping = l;
}

void IchabodConverter::setQuantizeMethod( const QString& method )
{
    if ( method == "DIFFUSE" )
    {
        m_settings.quantize_method = QuantizeMethod_DIFFUSE;
    }
    if ( method == "THRESHOLD" )
    {
        m_settings.quantize_method = QuantizeMethod_THRESHOLD;
    }
    if ( method == "ORDERED" )
    {
        m_settings.quantize_method = QuantizeMethod_ORDERED;
    }
    if ( method == "MEDIANCUT" )
    {
        m_settings.quantize_method = QuantizeMethod_MEDIANCUT;
    }
    if ( method == "MEDIANCUT_FLOYD" )
    {
        m_settings.quantize_method = QuantizeMethod_MEDIANCUT_FLOYD;
    }
}

void IchabodConverter::slotJavascriptEnvironment(QWebPage* page)
{
    m_activePage = page;
    // install custom css, if present
    if ( m_settings.css.length() )
    {
        QByteArray ba = m_settings.css.toUtf8().toBase64();
        QString data("data:text/css;charset=utf-8;base64,");
        QUrl css_data_url(data + QString(ba.data()));
        m_activePage->settings()->setUserStyleSheetUrl(css_data_url);
    }
    // register the current environment
    if ( m_settings.verbosity > 2 )
    {
        std::cout << "js rasterizer: " << m_settings.rasterizer << std::endl;
    }
    m_activePage->mainFrame()->addToJavaScriptWindowObject(m_settings.rasterizer, 
                                                           this);
}

void IchabodConverter::slotJavascriptWarning(QString s )
{
    m_warnings.push_back( s );
}


void IchabodConverter::slotJavascriptError(QString s )
{
    m_errors.push_back( s );
}

void IchabodConverter::snapshotElements( const QStringList& ids, int msec_delay )
{
    if ( !m_crops.size() )
    {
        snapshotPage( msec_delay );
        return;
    }
    QWebFrame* frame = m_activePage->mainFrame();
    // calculate largest rect encompassing all elements
    QRect crop_rect;
    for( QStringList::const_iterator it = ids.begin();
         it != ids.end();
         ++it )
    {
        QMap<QString,QVariant> crop = frame->evaluateJavaScript( QString("document.getElementById('%1').getBoundingClientRect()").arg(*it) ).toMap();    
        QRect r = QRect( crop["left"].toInt(), crop["top"].toInt(),
                         crop["width"].toInt()+1, crop["height"].toInt() );
        if ( r.isValid() )
        {
            r.adjust( -1 * (std::min(15,r.x())), -1 * (std::min(15,r.y())), 15, 15 ); // padding to account for possible font overflow
            if ( crop_rect.isValid() )
            {
                crop_rect = crop_rect.united( r );
            }
            else
            {
                crop_rect = r;
            }
        }
    }
    internalSnapshot( msec_delay, crop_rect );
}


void IchabodConverter::snapshotPage(int msec_delay)
{
    QRect invalid;
    internalSnapshot( msec_delay, invalid );
}

void IchabodConverter::internalSnapshot( int msec_delay, const QRect& crop )
{
    QWebFrame* frame = m_activePage->mainFrame();
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    // Calculate a good width for the image
    int highWidth=m_settings.screenWidth;
    m_activePage->setViewportSize(QSize(highWidth, 10));
    if (m_settings.smartWidth && frame->scrollBarMaximum(Qt::Horizontal) > 0) 
    {
        if (highWidth < 10)
        {
            highWidth=10;
        }
        int lowWidth=highWidth;
        while (frame->scrollBarMaximum(Qt::Horizontal) > 0 && highWidth < 32000) 
        {
            lowWidth = highWidth;
            highWidth *= 2;
            m_activePage->setViewportSize(QSize(highWidth, 10));
        }
        while (highWidth - lowWidth > 10) 
        {
            int t = lowWidth + (highWidth - lowWidth)/2;
            m_activePage->setViewportSize(QSize(t, 10));
            if (frame->scrollBarMaximum(Qt::Horizontal) > 0)
            {
                lowWidth = t;
            }
            else
            {
                highWidth = t;
            }
        }
        m_activePage->setViewportSize(QSize(highWidth, 10));
    }
    m_activePage->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    //Set the right height
    if (m_settings.screenHeight > 0)
    {
        m_activePage->setViewportSize(QSize(highWidth, m_settings.screenHeight));
    }
    else
    {
        m_activePage->setViewportSize(QSize(highWidth, frame->contentsSize().height()));
    }

    QPainter painter;
    QImage image;

    QRect rect = QRect(QPoint(0,0), m_activePage->viewportSize());
    if (rect.width() == 0 || rect.height() == 0) 
    {
        //?
    }

    image = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied); // draw as fast as possible
    painter.begin(&image);

    if (m_settings.transparent) 
    {
        QWebElement e = frame->findFirstElement("body");
        e.setStyleProperty("background-color", "transparent");
        e.setStyleProperty("background-image", "none");
        QPalette pal = m_activePage->palette();
        pal.setColor(QPalette::Base, QColor(Qt::transparent));
        m_activePage->setPalette(pal);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(QRect(QPoint(0,0),m_activePage->viewportSize()), QColor(0,0,0,0));
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    } 
    else 
    {
        painter.fillRect(QRect(QPoint(0,0),m_activePage->viewportSize()), Qt::white);
    }
    
    painter.translate(-rect.left(), -rect.top());
    frame->render(&painter);
    painter.end();
    
    m_images.push_back(image);
    m_delays.push_back(msec_delay);
    m_crops.push_back( crop );
}

void IchabodConverter::saveToOutput()
{
    if ( !m_images.size() )
    {
        //std::cerr << "WARNING: saveToOutput forcing snapshotPage" << std::endl;
        snapshotPage();
    }
    if ( m_settings.verbosity > 1 )
    {
        std::cout << "       images: " << m_images.size() << std::endl;
    }
    if ( m_settings.fmt == "gif" )
    {
        gifWrite( m_settings.quantize_method, m_images, m_delays, m_crops, m_settings.out, m_settings.looping );
    }
    else
    {
        QImage img = m_images.last();
        // selector, optionally creates initial crop
        if ( m_settings.selector.length() )
        {
            QWebFrame* frame = m_activePage->mainFrame();
            QWebElement el = frame->findFirstElement( m_settings.selector );
            QMap<QString,QVariant> crop = el.evaluateJavaScript( QString("this.getBoundingClientRect()") ).toMap();
            QRect r = QRect( crop["left"].toInt(), crop["top"].toInt(),
                             crop["width"].toInt(), crop["height"].toInt() );
            if ( m_settings.verbosity > 1 )
            {
                std::cout << "     selector: " << m_settings.selector << std::endl;
                std::cout << "selector rect: " << crop["left"].toInt() << "," << crop["top"].toInt() 
                          << " " <<crop["width"].toInt() << "x" <<crop["height"].toInt() << " valid:" << r.isValid() << std::endl;
            }
            if (r.isValid())
            {
                img = img.copy(r);
            }
            else
            {
                img = QImage();
            }
        }
        if ( m_settings.verbosity > 1 )
        {
            std::cout << "    crop rect: " << m_settings.crop_rect.x() << "," << m_settings.crop_rect.y() 
                      << " " << m_settings.crop_rect.width() << "x" << m_settings.crop_rect.height() << std::endl;
        }
        // actual cropping, relative to whatever img is now
        if ( m_settings.crop_rect.isValid() )
        {
            img = img.copy(m_settings.crop_rect);
        }
        QFile file;
        file.setFileName(m_settings.out);
        bool openOk = file.open(QIODevice::WriteOnly);
        if (!openOk) {
            QString err = QString("Failure to open output file: %1").arg(m_settings.out);
            m_errors.push_back(err);
            std::cerr << err.toLocal8Bit().constData() << std::endl;            
        }
        if ( !img.save(&file,m_settings.fmt.toLocal8Bit().constData(), m_settings.quality) )
        {
            QString err = QString("Failure to save output file: %1 as %2 img: %3x%4").arg(m_settings.out).arg(m_settings.fmt).arg(img.width()).arg(img.height());
            m_errors.push_back(err);
            std::cerr << err.toLatin1().constData() << std::endl;
        }
    }
}

void IchabodConverter::debugSettings(bool success_status)
{
    if ( m_settings.verbosity )
    {
        std::cout << "      success: " << success_status << std::endl;
        if ( m_settings.verbosity > 1 )
        {
            std::cout << "           in: " << m_settings.in << std::endl;
            std::cout << "          out: " << m_settings.out << std::endl;
            std::cout << "      quality: " << m_settings.quality << std::endl;
            std::cout << "     quantize: " << m_settings.quantize_method << std::endl;
            std::cout << "          fmt: " << m_settings.fmt << std::endl;
            std::cout << "  transparent: " << m_settings.transparent << std::endl;
            std::cout << "  smart width: " << m_settings.smartWidth << std::endl;
            std::cout << "       screen: " << m_settings.screenWidth << "x" << m_settings.screenHeight << std::endl;
            if ( m_settings.crop_rect.isValid() )
            {
                std::cout << "         crop: " << m_settings.crop_rect.x() << "," << m_settings.crop_rect.y()
                          << " " << m_settings.crop_rect.width() << "x" << m_settings.crop_rect.height() << std::endl;
            }
        }
        if ( m_settings.verbosity > 2 )
        {
            QFileInfo fi(m_settings.out);
            std::cout << "        bytes: " << fi.size() << std::endl;
            QImage img(m_settings.out, m_settings.fmt.toLocal8Bit().constData());
            std::cout << "         size: " << img.size().width() << "x" << img.size().height() << std::endl;
            std::cout << "script result: " << scriptResult() << std::endl;
            for( QVector<QString>::iterator it = m_warnings.begin();
                 it != m_warnings.end();
                 ++it )
            {
                std::cout << "       warning: " << *it << std::endl;
            }
            for( QVector<QString>::iterator it = m_errors.begin();
                 it != m_errors.end();
                 ++it )
            {
                std::cout << "         error: " << *it << std::endl;
            }
        }
        if ( m_settings.verbosity > 3 )
        {
            QFile fil_read(m_settings.in);
            fil_read.open(QIODevice::ReadOnly);
            QByteArray arr = fil_read.readAll();
            std::cout << "         html: " << QString(arr) << std::endl;
            for( QList<QString>::const_iterator it = m_settings.loadPage.runScript.begin();
                 it != m_settings.loadPage.runScript.end();
                 ++it )
            {
                std::cout << "           js: " << *it << std::endl;
            }
            std::cout << "     selector: " << m_settings.selector << std::endl;
            std::cout << "          css: " << m_settings.css << std::endl;
        }
    }
}

#define NANOS 1000000000LL
#define USED_CLOCK CLOCK_MONOTONIC

bool IchabodConverter::convert(QString& result, QVector<QString>& warnings, QVector<QString>& errors, double& elapsedms)
{
    m_images.clear();
    m_delays.clear();
    m_warnings.clear();
    m_errors.clear();
    wkhtmltopdf::ProgressFeedback feedback(true, *this);

    timespec time1, time2;
    clock_gettime(USED_CLOCK, &time1);
    long long start = time1.tv_sec*NANOS + time1.tv_nsec;

    bool success = wkhtmltopdf::ImageConverter::convert();  

    clock_gettime(USED_CLOCK, &time2);
    long long diff = time2.tv_sec*NANOS + time2.tv_nsec - start;
    elapsedms = (diff / 1000 + (diff % 1000 >= 500) /*round up halves*/) / 1000.0; 

    debugSettings(success);
    result = scriptResult();
    warnings = m_warnings;
    errors = m_errors;
    return !m_errors.size();
}

void IchabodConverter::setSelector( const QString& sel )
{
    m_settings.selector = sel;
}

void IchabodConverter::setCss( const QString& css )
{
    m_settings.css = css;
}

void IchabodConverter::setCropRect( int x, int y, int w, int h )
{
    m_settings.crop_rect = QRect(x,y,w,h);
}

void IchabodConverter::setSmartWidth( bool sw )
{
    m_settings.smartWidth = sw;
}
