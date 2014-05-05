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


IchabodConverter::IchabodConverter(wkhtmltopdf::settings::ImageGlobal & s, const QString * data) 
    : wkhtmltopdf::ImageConverter(s,data)
    , m_settings(s)
    , m_activePage(0)
{
    connect(this, SIGNAL(javascriptEnvironment(QWebPage*)), this, SLOT(slotJavascriptEnvironment(QWebPage*)));

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

void IchabodConverter::slotJavascriptEnvironment(QWebPage* page)
{
    // register the current environment
    m_activePage = page;
    m_activePage->mainFrame()->addToJavaScriptWindowObject("rasterizer", 
                                                           this);
}

void IchabodConverter::snapshotPage()
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
}

void IchabodConverter::saveToOutput()
{
    std::cout << "IchabodConverter::saveToOutput size:" << m_images.size() << std::endl;
    if ( !m_images.size() )
    {
        snapshotPage();
    }
    if ( m_settings.fmt == "gif" && m_images.size() > 1 )
    {
        gifWrite( m_images, m_settings.out, true );
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

QString IchabodConverter::convert( int verbosity, const wkhtmltopdf::settings::ImageGlobal& settings )
{
    m_images.clear();
    wkhtmltopdf::ProgressFeedback feedback(true, *this);
    bool success = wkhtmltopdf::ImageConverter::convert();  
    if ( verbosity )
    {
        if ( verbosity > 1 )
        {
            std::cout << "      success: " << success << std::endl;
            std::cout << "           in: " << settings.in << std::endl;
            std::cout << "          out: " << settings.out << std::endl;
            std::cout << "      quality: " << settings.quality<< std::endl;
            std::cout << "          fmt: " << settings.fmt << std::endl;
            std::cout << "  transparent: " << settings.transparent << std::endl;
            std::cout << "       screen: " << settings.screenWidth << "x" << settings.screenHeight << std::endl;
        }
        if ( verbosity > 2 )
        {
            QFileInfo fi(settings.out);
            std::cout << "        bytes: " << fi.size() << std::endl;
            QImage img(settings.out, settings.fmt.toLocal8Bit().constData());
            std::cout << "         size: " << img.size().width() << "x" << img.size().height() << std::endl;
            std::cout << " script result: " << scriptResult() << std::endl;
        }
        if ( verbosity > 3 )
        {
            QFile fil_read(settings.in);
            fil_read.open(QIODevice::ReadOnly);
            QByteArray arr = fil_read.readAll();
            std::cout << "      html: " << QString(arr) << std::endl;
            std::cout << "        js: " << settings.loadPage.runScript.at(0) << std::endl;
        }
    }
    return scriptResult();
}
