DEFINES += QT_STATIC

MOC_DIR      = build
OBJECTS_DIR  = build
UI_DIR       = build

QT += webkit network xmlpatterns svg

TEMPLATE = app
TARGET = ichabod
INCLUDEPATH += . wkhtmltopdf/src/shared wkhtmltopdf/src/image wkhtmltopdf/include mongoose giflib/lib
#CONFIG += debug

# wkhtmltopdf
HEADERS +=  wkhtmltopdf/src/shared/progressfeedback.hh

SOURCES += wkhtmltopdf/src/shared/outputter.cc wkhtmltopdf/src/shared/manoutputter.cc wkhtmltopdf/src/shared/htmloutputter.cc \
           wkhtmltopdf/src/shared/textoutputter.cc wkhtmltopdf/src/shared/arghandler.cc wkhtmltopdf/src/shared/commondocparts.cc \
 	   wkhtmltopdf/src/shared/commandlineparserbase.cc wkhtmltopdf/src/shared/commonarguments.cc \
	   wkhtmltopdf/src/shared/progressfeedback.cc

HEADERS += wkhtmltopdf/src/lib/multipageloader_p.hh  wkhtmltopdf/src/lib/converter_p.hh wkhtmltopdf/src/lib/multipageloader.hh\
           wkhtmltopdf/src/lib/converter.hh wkhtmltopdf/src/lib/utilities.hh
SOURCES += wkhtmltopdf/src/lib/loadsettings.cc wkhtmltopdf/src/lib/multipageloader.cc wkhtmltopdf/src/lib/tempfile.cc \
	   wkhtmltopdf/src/lib/converter.cc wkhtmltopdf/src/lib/websettings.cc  \
  	   wkhtmltopdf/src/lib/reflect.cc wkhtmltopdf/src/lib/utilities.cc

HEADERS += wkhtmltopdf/src/lib/imageconverter.hh wkhtmltopdf/src/lib/imagesettings.hh
HEADERS += wkhtmltopdf/src/lib/imageconverter_p.hh
SOURCES += wkhtmltopdf/src/lib/imagesettings.cc wkhtmltopdf/src/lib/imageconverter.cc


# mongoose
SOURCES += mongoose/mongoose.c

# giflib
SOURCES += giflib/lib/dgif_lib.c giflib/lib/egif_lib.c giflib/lib/gifalloc.c giflib/lib/gif_err.c giflib/lib/gif_font.c giflib/lib/gif_hash.c giflib/lib/quantize.c

# ichabod
HEADERS += conv.h
SOURCES += agif.cpp conv.cpp main.cpp mediancut.cpp
LIBS += -Wl,-Bstatic -lnetpbm -Wl,-Bdynamic 
#INCLUDEPATH += /tmp/im/include/ImageMagick-6
#LIBS += -L/tmp/im/lib -lMagick++-6.Q16 -lMagickWand-6.Q16 -lMagickCore-6.Q16



