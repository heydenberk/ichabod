#include <iostream>

#include <QApplication>
#include <QtCore>
#include <QTemporaryFile>
#include <QTextStream>
#include <QImage>
#include <QFileInfo>

#include <wkhtmltox/utilities.hh>
#include <wkhtmltox/imageconverter.hh>

#include "mongoose.h"

#include "conv.h"

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
    static char data[2097152];
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

// handler for all incoming connections
static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
    if (ev == MG_REQUEST) 
    {
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
            rasterizer = "ichabod";
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
        settings.quality = 0;
        settings.out = output;
        settings.screenWidth = width;
        settings.screenHeight = height;
        settings.transparent = true;
        // check endpoint and scripts
        QStringList pathParts = QString(conn->uri).split("/", QString::SkipEmptyParts);
        if ( pathParts.isEmpty() )
        {
            return send_error(conn, "Bad endpoint");
        }
        QString path = pathParts.last();
        if (path == "evaluate") 
        {
            if ( js.isEmpty() )
            {
                return send_error(conn, "Nothing to evaluate");                
            }
        }
        else if (path == "rasterize")
        {
            if ( js.isEmpty() )
            {
                // backwards compatibility
                js = QString("(function(){%1.saveToOutput();})()").arg(settings.rasterizer);
            }
        }
        else
        {
            return send_error(conn, "Bad endpoint");
        }
        QList<QString> scripts;
        scripts.append(js);
        if ( !scripts.size() )
        {
            return send_error(conn, "Internal error: no scripts");
        }
        settings.loadPage.runScript = scripts;

        mg_send_header(conn, "Content-Type", "application/json");
        if ( g_verbosity )
        {
            std::cout << conn->uri << std::endl;
        }

        IchabodConverter converter(settings);
        QDateTime now = QDateTime::currentDateTime();
        QString result = converter.convert();
        if ( g_verbosity )
        {
            std::cout << "elapsed (msec):" << now.msecsTo(QDateTime::currentDateTime()) << std::endl;
        }
        QString json = QString("{\"path\": \"%1\", \"result\": %2}").arg(settings.out, result);
        mg_send_data(conn, json.toLocal8Bit().constData(), json.length());
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

    bool gui = false;
    QApplication app(argc, argv, gui);
    MyLooksStyle * style = new MyLooksStyle();
    app.setStyle(style);

    int port = 9090;
    QStringList args = app.arguments();
    QRegExp rxPort("--port=([0-9]{1,})");
    QRegExp rxVerbose("--verbosity=([0-9]{1,})");

    for (int i = 1; i < args.size(); ++i) {
        if (rxPort.indexIn(args.at(i)) != -1 )
        {
            port =  rxPort.cap(1).toInt();
        }
        else if (rxVerbose.indexIn(args.at(i)) != -1 ) 
        {
            g_verbosity = rxVerbose.cap(1).toInt();
        }
        else 
        {
            std::cerr << "Uknown arg:" << args.at(i).toLocal8Bit().constData() << std::endl;
            return -1;
        }
    }

    struct mg_server *server = mg_create_server(NULL, ev_handler);

    mg_set_option(server, "listening_port", QString::number(port).toLocal8Bit().constData());

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
