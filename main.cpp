#include <iostream>

#include <QApplication>
#include <QString>
#include <QtCore>
#include <QTemporaryFile>
#include <QTextStream>
#include <QImage>
#include <QFileInfo>

#include <wkhtmltox/utilities.hh>
#include <wkhtmltox/imageconverter.hh>

#include "mongoose.h"
#include "ppm.h"

#include "version.h"
#include "conv.h"

#define ICHABOD_NAME "ichabod"

int g_verbosity = 0;

// output error and send error back to client
static int send_error(struct mg_connection* conn, const char* err)
{
    std::cerr << "ERROR:" << err << std::endl;
    mg_send_status(conn, 500);
    mg_send_header(conn, "X-Error-Message", err);
    mg_printf_data(conn, err);
    return MG_TRUE;
}

// either get the POST variable, or returning empty string
// WARNING: this function is not re-entrant
static QString get_var( struct mg_connection *conn, const char* var_name )
{
    static char data[20971520];
    int r = mg_get_var(conn, var_name, data, sizeof(data));
    switch( r )
    {
    case -1: // not found
        return QString();
    case -2: // too big
        return QString();
    }
    return QString(data);
}

static int handle_001(struct mg_connection *conn, IchabodSettings& settings)
{
    mg_send_header(conn, "Content-Type", "application/json");
    if ( !settings.loadPage.runScript.size() )
    {
        return send_error(conn, "Internal error: no scripts");
    }
    IchabodConverter converter(settings);
    std::pair<QString,QVector<QString> > result = converter.convert();
    QString json = QString("{\"path\": \"%1\", \"result\": \"%2\"}").arg(settings.out, result.first);
    mg_send_data(conn, json.toLocal8Bit().constData(), json.length());
    return MG_TRUE;
}

static void send_headers(struct mg_connection* conn)
{
    mg_send_header(conn, "Content-Type", "application/json");
    static const char kszFormat[] = "ddd, dd MMM yyyy HH:mm:ss 'GMT'";
    QDateTime now = QDateTime::currentDateTime();
    mg_send_header(conn, "Date", now.toString(kszFormat).toLocal8Bit().constData());
    mg_send_header(conn, "Server", QString("%1 %2").arg(ICHABOD_NAME).arg(ICHABOD_VERSION).toLocal8Bit().constData());
}

static int handle_002(struct mg_connection *conn, IchabodSettings& settings)
{
    send_headers(conn);
    if ( !settings.loadPage.runScript.size() )
    {
        return send_error(conn, "Internal error: no scripts");
    }
    IchabodConverter converter(settings);
    std::pair<QString,QVector<QString> > result = converter.convert();
    QString json = QString("{\"path\": \"%1\", \"result\": %2}").arg(settings.out, result.first);
    mg_send_data(conn, json.toLocal8Bit().constData(), json.length());
    return MG_TRUE;
}

static int handle_003(struct mg_connection *conn, IchabodSettings& settings)
{
    send_headers(conn);
    if ( !settings.loadPage.runScript.size() )
    {
        return send_error(conn, "Internal error: no scripts");
    }
    IchabodConverter converter(settings);
    std::pair<QString,QVector<QString> > result = converter.convert();
    QString warn;
    for( QVector<QString>::iterator it = result.second.begin();
         it != result.second.end();
         ++it )
    {
        warn += QString("\"%1\"").arg(*it);
        if ( it != result.second.begin() )
        {
            warn += ",";
        }
    }
    QString json = QString("{\"path\": \"%1\", \"result\": %2, \"warnings\": [%3]}").arg(settings.out, result.first, warn);
    mg_send_data(conn, json.toLocal8Bit().constData(), json.length());
    return MG_TRUE;
}


static int handle_health(struct mg_connection *conn)
{
    send_headers(conn);
    char health[4] = {0xF0, 0x9F, 0x91, 0xBB};
    QString vigor = get_var(conn, "vigor");
    if ( vigor.length() )
    {
        int v = vigor.toInt();
        if ( v > 100 ) { v = 100; }
        if ( v < 0 ) { v = 1; }
        for ( int i = 0; i<v; ++i)
        {
            mg_send_data(conn, health, 4);
        }
    }
    mg_send_data(conn, "", 0);
    return MG_TRUE;
}

static int handle_default(struct mg_connection *conn, IchabodSettings& settings)
{
    return handle_003(conn, settings);
}

// handler for all incoming connections
static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
    if (ev == MG_REQUEST) 
    {
        QString canonical_path = "/";
        QStringList pathParts = QString(conn->uri).split("/", QString::SkipEmptyParts);
        if ( pathParts.size() )
        {
            canonical_path = pathParts.last();
        }
        if ( canonical_path == "health" )
        {
            return handle_health(conn);
        }

        QString format = get_var(conn, "format");
        QString html = get_var(conn, "html");
        QString js = get_var(conn, "js");
        QString rasterizer = get_var(conn, "rasterizer");
        QString output = get_var(conn, "output");
        QString url = get_var(conn, "url");
        int width = get_var(conn, "width").toInt();
        int height = get_var(conn, "height").toInt();

        if ( !rasterizer.length() )
        {
            rasterizer = ICHABOD_NAME;
        }
        if ( !output.length() )
        {
            return send_error(conn, "No output specified");
        }
        if ( width < 1 || height < 1 )
        {
            return send_error(conn, "Bad dimensions");
        }
        if ( !html.length() && !url.length() )
        {
            return send_error(conn, "Empty document and no URL specified");
        }

        QString input;
        QTemporaryFile file(output + QString("_XXXXXX.html"));
        if ( html.length() ) {
            if ( !file.open() )
            {
                return send_error(conn, (QString("Unable to open:") + output + QString("_XXXXXX.html")).toLocal8Bit().constData() );
            }
            QTextStream out(&file);
            out << html;
            out.flush();

            input = file.fileName();
        } else {
            input = url;
        }

        if ( format.startsWith(".") )
        {
            format = format.mid(1);
        }
        if ( format.isEmpty() )
        {
            format = "png";
        }

        IchabodSettings settings;
        settings.verbosity = g_verbosity;
        settings.rasterizer = rasterizer;
        settings.fmt = format;
        settings.in = input;
        settings.quality = 50; // reasonable size/speed tradeoff by default
        settings.out = output;
        settings.screenWidth = width;
        settings.screenHeight = height;
        settings.transparent = true;
        settings.looping = false;
        settings.quantize_method = QuantizeMethod_MEDIANCUT;
        settings.loadPage.debugJavascript = true;
        QList<QString> scripts;
        scripts.append(js);
        settings.loadPage.runScript = scripts;

        if ( g_verbosity )
        {
            std::cout << conn->uri << std::endl;
        }

        if ( canonical_path == "004" || canonical_path == "003" )
        {
            return handle_003(conn, settings);
        }
        if ( canonical_path == "002" )
        {
            return handle_002(conn, settings);
        }
        if ( canonical_path == "evaluate" )
        {
            if ( js.isEmpty() )
            {
                return send_error(conn, "Nothing to evaluate");                
            }
            QList<QString> scripts;
            scripts.append("(function(){ichabod.saveToOutput();})()");
            scripts.append(js);
            settings.loadPage.runScript = scripts;
            return handle_001(conn, settings);
        }
        if ( canonical_path == "rasterize" || canonical_path == "001" )
        {
            QList<QString> scripts;
            scripts.append("(function(){ichabod.saveToOutput();})()");
            settings.loadPage.runScript = scripts;
            return handle_001(conn, settings);            
        }
        return handle_default(conn, settings);
    } 
    else if (ev == MG_AUTH) 
    {
        return MG_TRUE;
    }
    return MG_FALSE;
}

int main(int argc, char *argv[])
{
    ppm_init( &argc, argv );

    bool gui = false;
    QApplication app(argc, argv, gui);
    MyLooksStyle * style = new MyLooksStyle();
    app.setStyle(style);


    int port = 9090;
    QStringList args = app.arguments();
    QRegExp rxPort("--port=([0-9]{1,})");
    QRegExp rxVerbose("--verbosity=([0-9]{1,})");
    QRegExp rxVersion("--version");

    for (int i = 1; i < args.size(); ++i) {
        if (rxPort.indexIn(args.at(i)) != -1 )
        {
            port =  rxPort.cap(1).toInt();
        }
        else if (rxVerbose.indexIn(args.at(i)) != -1 ) 
        {
            g_verbosity = rxVerbose.cap(1).toInt();
        }
        else if (rxVersion.indexIn(args.at(i)) != -1 ) 
        {
            std::cout << ICHABOD_NAME << " version " << ICHABOD_VERSION << std::endl;
            std::cout << "** The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond" << std::endl;
            std::cout << "** ppmquant is Copyright (C) 1989, 1991 by Jef Poskanzer." << std::endl;
            std::cout << "**" << std::endl;
            std::cout << "** Permission to use, copy, modify, and distribute this software and its" << std::endl;
            std::cout << "** documentation for any purpose and without fee is hereby granted, provided" << std::endl;
            std::cout << "** that the above copyright notice appear in all copies and that both that" << std::endl;
            std::cout << "** copyright notice and this permission notice appear in supporting" << std::endl;
            std::cout << "** documentation.  This software is provided \"as is\" without express or" << std::endl;
            std::cout << "** implied warranty." << std::endl;
            return 0;
        }
        else 
        {
            std::cerr << "Uknown arg:" << args.at(i).toLocal8Bit().constData() << std::endl;
            return -1;
        }
    }

    struct mg_server *server = mg_create_server(NULL, ev_handler);

    const char * err = mg_set_option(server, "listening_port", QString::number(port).toLocal8Bit().constData());
    if ( err )
    {
        std::cerr << "Cannot bind to port:" << port << " [" << err << "], exiting." << std::endl;
        return -1;
    }
    if ( g_verbosity )
    {
        std::cout << "Starting on port " << mg_get_option(server, "listening_port") << std::endl;
    }
    for (;;) 
    {
        mg_poll_server(server, 1000);
    }
    
    mg_destroy_server(&server);
    
    return 0;
}
