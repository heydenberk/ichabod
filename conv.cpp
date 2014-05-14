#include "conv.h"
#include "agif.h"
#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <iostream>
#include <QDateTime>

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

void IchabodConverter::slotJavascriptEnvironment(QWebPage* page)
{
    // register the current environment
    if ( m_settings.verbosity > 2 )
    {
        std::cout << " js rasterizer:" << m_settings.rasterizer << std::endl;
    }
    m_activePage = page;
    m_activePage->mainFrame()->addToJavaScriptWindowObject(m_settings.rasterizer, 
                                                           this);
}

void IchabodConverter::slotJavascriptWarning(QString s )
{
    m_warnings.push_back( s );
}

void IchabodConverter::snapshotPage(int msec_delay)
{
    if ( m_settings.verbosity > 3 )
    {
        std::cout << "     snapshot"<< std::endl;
    }
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

    image = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied);
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
}

void IchabodConverter::saveToOutput()
{
    if ( !m_images.size() )
    {
        snapshotPage();
    }
    if ( m_settings.verbosity > 1 )
    {
        std::cout << "       images: " << m_images.size() << std::endl;
    }
    if ( m_settings.fmt == "gif" )
    {
        gifWrite( m_images, m_delays, m_settings.out, m_settings.looping );
    }
    else
    {
        QFile file;
        file.setFileName(m_settings.out);
        bool openOk = file.open(QIODevice::WriteOnly);
        if (!openOk) {
            std::cerr << "failure to open output file: " << m_settings.out << std::endl;            
        }
        if ( !m_images.last().save(&file,m_settings.fmt.toLocal8Bit().constData(), m_settings.quality) )
        {
            std::cerr << "failure to save output file: " << m_settings.out << " as " << m_settings.fmt << std::endl;            
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
            std::cout << "      quality: " << m_settings.quality<< std::endl;
            std::cout << "          fmt: " << m_settings.fmt << std::endl;
            std::cout << "  transparent: " << m_settings.transparent << std::endl;
            std::cout << "       screen: " << m_settings.screenWidth << "x" << m_settings.screenHeight << std::endl;
        }
        if ( m_settings.verbosity > 2 )
        {
            QFileInfo fi(m_settings.out);
            std::cout << "        bytes: " << fi.size() << std::endl;
            QImage img(m_settings.out, m_settings.fmt.toLocal8Bit().constData());
            std::cout << "         size: " << img.size().width() << "x" << img.size().height() << std::endl;
            std::cout << " script result: " << scriptResult() << std::endl;
            for( QVector<QString>::iterator it = m_warnings.begin();
                 it != m_warnings.end();
                 ++it )
            {
                std::cout << "       warning: " << *it << std::endl;
            }
        }
        if ( m_settings.verbosity > 3 )
        {
            QFile fil_read(m_settings.in);
            fil_read.open(QIODevice::ReadOnly);
            QByteArray arr = fil_read.readAll();
            std::cout << "      html: " << QString(arr) << std::endl;
            for( QList<QString>::const_iterator it = m_settings.loadPage.runScript.begin();
                 it != m_settings.loadPage.runScript.end();
                 ++it )
            {
                std::cout << "        js: " << *it << std::endl;
            }
        }
    }
}

std::pair<QString,QVector<QString> > IchabodConverter::convert()
{
    m_images.clear();
    m_delays.clear();
    m_warnings.clear();
    wkhtmltopdf::ProgressFeedback feedback(true, *this);
    bool success = wkhtmltopdf::ImageConverter::convert();  
    debugSettings(success);
    QString res = scriptResult();
    return std::make_pair(res,m_warnings);
}
