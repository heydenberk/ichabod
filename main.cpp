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

static int rasterize(struct mg_connection* conn )
{
    char var1[500], var2[500];
    mg_get_var(conn, "input_1", var1, sizeof(var1));
    mg_get_var(conn, "input_2", var2, sizeof(var2));


      wkhtmltopdf::settings::ImageGlobal settings;
      settings.in = "/tmp/test.html";
      settings.quality = 0;
      settings.fmt = "png";
      settings.out = "/tmp/out.png";
      settings.screenWidth = 800;
      settings.screenHeight = 600;
      settings.evalJs = "1+1";

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
          std::cout << "Rasterized html--------------------"<< std::endl;
//std::cout << "         peer: " << socket->peerAddress().toString().toLocal8Bit().constData() << std::endl;        
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
//std::cout << "         html: " << m_postMap["html"].toLocal8Bit().constData() << std::endl;
          }
      }
      std::cout << "done." << std::endl;
      return MG_TRUE;
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
    if (ev == MG_REQUEST) 
    {
        if (strcmp(conn->uri, "/rasterize") == 0) 
        {
            return rasterize(conn);
        }
        
        return MG_FALSE;
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
