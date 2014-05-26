#include <stdio.h>
#include "agif.h"
#include <gif_lib.h>
#include "quant.h"

#include <iostream>

void makeIndexedImage( const QuantizeMethod method, QImage& img )
{
    //img.save("/tmp/out_orig.png", "png", 50);
    switch (method)
    {
    case QuantizeMethod_DIFFUSE:
        img = img.convertToFormat(QImage::Format_Indexed8, Qt::DiffuseDither);
        break;
    case QuantizeMethod_THRESHOLD:
        img = img.convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither);
        break;
    case QuantizeMethod_MEDIANCUT:
        img = quantize_mediancut(img).convertToFormat( QImage::Format_Indexed8 );
        break;
    }
}

bool gifWrite ( const QuantizeMethod method, const QVector<QImage> & images, const QVector<int>& delays, const QString& filename, bool loop )
{
    if ( !images.size() )
    {
        return false;
    }
    if ( images.size() != delays.size() )
    {
        return false;
    }
    QImage first = images.at(0);
    makeIndexedImage(method, first);

    // color table
    QVector<QRgb> colorTable = first.colorTable();
    ColorMapObject cmap;
    // numColors must be a power of 2
    int numColors = 1 << GifBitSize(first.colorCount());
    cmap.ColorCount = numColors;
    cmap.BitsPerPixel = first.depth();// should always be 8
    if ( cmap.BitsPerPixel != 8 )
    {
        std::cerr << "Incorrect bit depth" << std::endl;
        return false;
    }
    GifColorType* colorValues = (GifColorType*)malloc(cmap.ColorCount * sizeof(GifColorType));
    cmap.Colors = colorValues;
    int c = 0;
    for(; c < first.colorCount(); ++c)
    {
        //qDebug("color %d has %02X%02X%02X", c, qRed(colorTable[c]), qGreen(colorTable[c]), qBlue(colorTable[c]));
        colorValues[c].Red = qRed(colorTable[c]);
        colorValues[c].Green = qGreen(colorTable[c]);
        colorValues[c].Blue = qBlue(colorTable[c]);
    }
    // In case we had an actual number of colors that's not a power of 2,
    // fill the rest with something (black perhaps).
    for (; c < numColors; ++c)
    {
        colorValues[c].Red = 0;
        colorValues[c].Green = 0;
        colorValues[c].Blue = 0;
    }

    int error = 0;
    GifFileType *gif = EGifOpenFileName(filename.toLocal8Bit().constData(), false, &error);
    if (!gif) return false;
    EGifSetGifVersion(gif, true);

    if (EGifPutScreenDesc(gif, first.width(), first.height(), 8, 0, &cmap) == GIF_ERROR)
    {
        std::cerr << "EGifPutScreenDesc returned error" << std::endl;
        return false;
    }
    
    if ( loop )
    {
        unsigned char nsle[12] = "NETSCAPE2.0";
        unsigned char subblock[3];
        int loop_count = 0;
        subblock[0] = 1;
        subblock[1] = loop_count % 256;
        subblock[2] = loop_count / 256;
        if (EGifPutExtensionLeader(gif, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR
            || EGifPutExtensionBlock(gif, 11, nsle) == GIF_ERROR
            || EGifPutExtensionBlock(gif, 3, subblock) == GIF_ERROR
            || EGifPutExtensionTrailer(gif) == GIF_ERROR) {
            std::cerr << "Error writing loop extension" << std::endl;
            return false;
        }
    }
    
    for ( QVector<QImage>::const_iterator it = images.begin(); it != images.end() ; ++it )
    {
        QImage toWrite(*it);

        makeIndexedImage(method, toWrite);

        // db more: transparent GIFs?

        //qDebug("width:%d", toWrite.width() );
        //qDebug("height:%d", toWrite.height() );
        //qDebug("pixel count:%d", toWrite.width() * toWrite.height() );
        //qDebug("line length:%d", toWrite.bytesPerLine() );

        // animation delay
        int msec_delay = delays.at( it-images.begin() );
        static unsigned char ExtStr[4] = { 0x04, 0x00, 0x00, 0xff };
        ExtStr[0] = (false) ? 0x06 : 0x04;
        ExtStr[1] = msec_delay % 256;
        ExtStr[2] = msec_delay / 256;
        EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, 4, ExtStr);

        if (EGifPutImageDesc(gif, 0, 0, toWrite.width(), toWrite.height(), 0, &cmap) == GIF_ERROR)
        {
            std::cerr << "EGifPutImageDesc returned error" << std::endl;
        }
        int lc = toWrite.height();
        //int llen = toWrite.bytesPerLine();
        //qDebug("will write %d lines, %d bytes each", lc, toWrite.width());
        for (int l = 0; l < lc; ++l)
        {
            uchar* line = toWrite.scanLine(l);
            if (EGifPutLine(gif, (GifPixelType*)line, toWrite.width()) == GIF_ERROR)
            {
                std::cerr << "EGifPutLine returned error: " <<  gif->Error << std::endl;
            }
        }
    }

    EGifCloseFile(gif);
    return true;
}

