#include "agif.h"
#include <gif_lib.h>

// 
void makeIndexedImage( QImage& img )
{
    // ensure the most artifact-free conversion possible
    //img = img.convertToFormat(QImage::Format_Indexed8, Qt::ColorOnly | Qt::ThresholdDither );
    img = img.convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither );
    //img = img.convertToFormat(QImage::Format_Indexed8);
    //img.save("/tmp/out_indexed8.png", "png", 100);
}

bool gifWrite ( const QVector<QImage> & images, const QVector<int>& delays, const QString& filename, bool loop )
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
    makeIndexedImage(first);

    // color table
    QVector<QRgb> colorTable = first.colorTable();
    ColorMapObject cmap;
    // numColors must be a power of 2
    int numColors = 1 << GifBitSize(first.colorCount());
    //qDebug("numColors: %d", first.colorCount() );
    //qDebug("numColors po2: %d", numColors );
    cmap.ColorCount = numColors;
    cmap.BitsPerPixel = first.depth();// should always be 8
    if ( cmap.BitsPerPixel != 8 )
    {
        qCritical("Incorrect bit depth");
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
        qCritical("EGifPutScreenDesc returned error");
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
            qCritical("Error writing loop extension");
            return false;
        }
    }
    
    for ( QVector<QImage>::const_iterator it = images.begin(); it != images.end() ; ++it )
    {
        QImage toWrite(*it);

        //toWrite.save("/tmp/out_original.png", "png", 0);

        makeIndexedImage(toWrite);

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
            qCritical("EGifPutImageDesc returned error");
        int lc = toWrite.height();
        //int llen = toWrite.bytesPerLine();
        //qDebug("will write %d lines, %d bytes each", lc, llen);
        for (int l = 0; l < lc; ++l)
        {
            uchar* line = toWrite.scanLine(l);
            if (EGifPutLine(gif, (GifPixelType*)line, toWrite.width()) == GIF_ERROR)
            {
                qCritical("EGifPutLine returned error: %d", gif->Error);
            }
        }
    }

    EGifCloseFile(gif);
    return true;
}

