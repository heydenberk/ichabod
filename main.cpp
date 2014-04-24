#include <QApplication>
#include <QtCore>

#include <iostream>
#include <wkhtmltox/imageconverter.hh>

#include <QDateTime>
#include <QHttpRequestHeader>
#include <QUrl>
#include <iostream>
#include "progressfeedback.hh"
#include <QApplication>
#include <QWebFrame>
#include <wkhtmltox/imagesettings.hh>
#include <wkhtmltox/utilities.hh>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTcpSocket>
#include <QImage>
#include <QBuffer>
#include <QFontDatabase>
#include <QFileInfo>

/*
int main2(int argc, char *argv[])
{
    
    bool gui = true;
    
    QApplication app(argc, argv, gui);

    QStringList args = app.arguments();
    QRegExp rxArgPort("--port=([0-9]{1,})");
    QRegExp rxVerbose("--verbosity=([0-2])");

    int verbosity = 0;
    int port = 9090;
    for (int i = 1; i < args.size(); ++i) {
        if (rxArgPort.indexIn(args.at(i)) != -1 )
        {
            port =  rxArgPort.cap(1).toInt();
        }
        else if (rxVerbose.indexIn(args.at(i)) != 1 ) 
        {
            verbosity = rxVerbose.cap(1).toInt();
        }
        else 
        {
            std::cerr << "Uknown arg:" << args.at(i).toLocal8Bit().constData() << std::endl;
            app.exit();
            return -1;
        }
    }
    Server server(port, verbosity);

    return app.exec();
}
*/

#include <stdio.h>
#include <string.h>
#include "mongoose.h"


static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
    if (ev == MG_REQUEST) 
    {
        char formatChar[32], htmlChar[2097152], jsChar[1048576], outputChar[128], widthChar[32], heightChar[32];
        mg_get_var(conn, "format", formatChar, sizeof(formatChar));
        mg_get_var(conn, "html", htmlChar, sizeof(htmlChar));
        mg_get_var(conn, "js", jsChar, sizeof(jsChar));
        mg_get_var(conn, "output", outputChar, sizeof(outputChar));
        mg_get_var(conn, "width", widthChar, sizeof(widthChar));
        mg_get_var(conn, "height", heightChar, sizeof(heightChar));

        QString format = QString(formatChar);
        QString html = QString(htmlChar);
        QString js = QString(jsChar);
        QString output = QString(outputChar);
        int width = QString(widthChar).toInt();
        int height = QString(heightChar).toInt();

        QTemporaryFile file(output + QString("_XXXXXX.html"));
        file.open();
        QTextStream out(&file);
        out << html;
        out.flush();

        if ( format.startsWith(".") )
        {
            format = format.mid(1);
        }
        if ( format.isEmpty() )
        {
            format = "png";
        }

        wkhtmltopdf::settings::ImageGlobal settings;
        settings.fmt = format;
        settings.in = file.fileName();
        settings.quality = 0;
        settings.out = output;
        settings.screenWidth = width;
        settings.screenHeight = height;

        QStringList pathParts = QString(conn->uri).split("/", QString::SkipEmptyParts);
        QString path = pathParts[pathParts.size() - 1];

        QList<QString> scripts;
        if (path.compare(QString("evaluate")) == 0) {
            scripts.append(js.toLocal8Bit().constData());
            settings.loadPage.runScript = scripts;
        }

        wkhtmltopdf::ImageConverter converter(settings);
        QObject::connect(&converter, SIGNAL(checkboxSvgChanged(const QString &)), qApp->style(), SLOT(setCheckboxSvg(const QString &)));
        QObject::connect(&converter, SIGNAL(checkboxCheckedSvgChanged(const QString &)), qApp->style(), SLOT(setCheckboxCheckedSvg(const QString &)));
        QObject::connect(&converter, SIGNAL(radiobuttonSvgChanged(const QString &)), qApp->style(), SLOT(setRadioButtonSvg(const QString &)));
        QObject::connect(&converter, SIGNAL(radiobuttonCheckedSvgChanged(const QString &)), qApp->style(), SLOT(setRadioButtonCheckedSvg(const QString &)));

        wkhtmltopdf::ProgressFeedback feedback(true, converter);
        std::cout << "starting converter." << std::endl;
        bool success = converter.convert();
        if ( true )
        {
            std::cout << "Rasterized html--------------------" << std::endl;
            std::cout << "      success: " << success << std::endl;
            std::cout << "           in: " << settings.in.toLocal8Bit().constData() << std::endl;
            std::cout << "          out: " << settings.out.toLocal8Bit().constData() << std::endl;
            std::cout << "      quality: " << settings.quality<< std::endl;
            std::cout << "          fmt: " << settings.fmt.toLocal8Bit().constData() << std::endl;
            std::cout << "  screenWidth: " << settings.screenWidth << std::endl;
            std::cout << " screenHeight: " << settings.screenHeight << std::endl;
            if ( 2 > 1 )
            {
                QFileInfo fi(settings.out);
                std::cout << "        bytes: " << fi.size() << std::endl;
                QImage img(settings.out, settings.fmt.toLocal8Bit().constData());
                std::cout << "         size: " << img.size().width() << "x" << img.size().height() << std::endl;
                std::cout << " script result: " << converter.scriptResult().toLocal8Bit().constData() << std::endl;
            }
        }
        mg_send_header(conn, "Content-Type", "application/json");
        QString clickzones = converter.scriptResult();
        QString json = QString("{\"path\": \"%1\", \"clickzones\": \"%2\"}").arg(settings.out, clickzones.toLocal8Bit().constData());
        std::cout << "json: " << json.toLocal8Bit().constData() << std::endl;

        mg_send_data(conn, json.toLocal8Bit().constData(), json.length());
        std::cout << "done." << std::endl;
        return MG_TRUE;
    } 
    else if (ev == MG_AUTH) 
    {
        return MG_TRUE;
    }
    return MG_FALSE;
}

int main(int argc, char *argv[])
{

    bool gui = true;
    QApplication app(argc, argv, gui);
    MyLooksStyle * style = new MyLooksStyle();
    app.setStyle(style);

    QStringList args = app.arguments();
    QRegExp rxArgPort("--port=([0-9]{1,})");
    QRegExp rxVerbose("--verbosity=([0-2])");

    int verbosity = 0;
    int port = 9090;
    for (int i = 1; i < args.size(); ++i) {
        if (rxArgPort.indexIn(args.at(i)) != -1 )
        {
            port =  rxArgPort.cap(1).toInt();
        }
        else if (rxVerbose.indexIn(args.at(i)) != 1 ) 
        {
            verbosity = rxVerbose.cap(1).toInt();
        }
        else 
        {
            std::cerr << "Uknown arg:" << args.at(i).toLocal8Bit().constData() << std::endl;
            app.exit();
            return -1;
        }
    }


    struct mg_server *server = mg_create_server(NULL, ev_handler);

    mg_set_option(server, "listening_port", QString::number(port).toLocal8Bit().constData());

    printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
    for (;;) {
        mg_poll_server(server, 1000);
    }
    
    mg_destroy_server(&server);
    
    return 0;
}
