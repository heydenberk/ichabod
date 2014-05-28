#include <stdio.h>
#include "agif.h"
#include <gif_lib.h>
#include "quant.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

//#include "Magick++.h"
#include <QColor>
#include <QRgb>


/*
std::map<QRgb,int> g_nearest;
 int nearestColor( int r, int g, int b, const QColor *palette, int size )
 {
     if (palette == 0)
       return 0;

     int dr = palette[0].red() - r;
     int dg = palette[0].green() - g;
     int db = palette[0].blue() - b;
 
     int minDist =  dr*dr + dg*dg + db*db;
     int nearest = 0;
 
     for (int i = 1; i < size; i++ )
     {
         dr = palette[i].red() - r;
         dg = palette[i].green() - g;
         db = palette[i].blue() - b;
 
         int dist = dr*dr + dg*dg + db*db;
 
         if ( dist < minDist )
         {
             minDist = dist;
             nearest = i;
         }
     }
 
     //g_nearest[c.rgb()] = nearest;
     return nearest;
 }

 QImage& dither(QImage &img, const QColor *palette, int size)
 {
     if (img.width() == 0 || img.height() == 0 ||
         palette == 0 || img.depth() <= 8)
       return img;
 
     QImage dImage( img.width(), img.height(), QImage::Format_Indexed8 );
     int i;
 
     dImage.setNumColors( size );
     for ( i = 0; i < size; i++ )
         dImage.setColor( i, palette[ i ].rgb() );
 
     int *rerr1 = new int [ img.width() * 2 ];
     int *gerr1 = new int [ img.width() * 2 ];
     int *berr1 = new int [ img.width() * 2 ];
 
     memset( rerr1, 0, sizeof( int ) * img.width() * 2 );
     memset( gerr1, 0, sizeof( int ) * img.width() * 2 );
     memset( berr1, 0, sizeof( int ) * img.width() * 2 );
 
     int *rerr2 = rerr1 + img.width();
     int *gerr2 = gerr1 + img.width();
     int *berr2 = berr1 + img.width();
 
     for ( int j = 0; j < img.height(); j++ )
     {
         uint *ip = (uint * )img.scanLine( j );
         uchar *dp = dImage.scanLine( j );
 
         for ( i = 0; i < img.width(); i++ )
         {
             rerr1[i] = rerr2[i] + qRed( *ip );
             rerr2[i] = 0;
             gerr1[i] = gerr2[i] + qGreen( *ip );
             gerr2[i] = 0;
             berr1[i] = berr2[i] + qBlue( *ip );
             berr2[i] = 0;
             ip++;
         }
 
         *dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0], palette, size );
 
         for ( i = 1; i < img.width()-1; i++ )
         {
             int indx = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
             *dp = indx;
 
             int rerr = rerr1[i];
             rerr -= palette[indx].red();
             int gerr = gerr1[i];
             gerr -= palette[indx].green();
             int berr = berr1[i];
             berr -= palette[indx].blue();
 
             // diffuse red error
             rerr1[ i+1 ] += ( rerr * 7 ) >> 4;
             rerr2[ i-1 ] += ( rerr * 3 ) >> 4;
             rerr2[  i  ] += ( rerr * 5 ) >> 4;
             rerr2[ i+1 ] += ( rerr ) >> 4;
 
             // diffuse green error
             gerr1[ i+1 ] += ( gerr * 7 ) >> 4;
             gerr2[ i-1 ] += ( gerr * 3 ) >> 4;
             gerr2[  i  ] += ( gerr * 5 ) >> 4;
             gerr2[ i+1 ] += ( gerr ) >> 4;
 
             // diffuse red error
             berr1[ i+1 ] += ( berr * 7 ) >> 4;
             berr2[ i-1 ] += ( berr * 3 ) >> 4;
             berr2[  i  ] += ( berr * 5 ) >> 4;
             berr2[ i+1 ] += ( berr ) >> 4;
 
             dp++;
         }
 
         *dp = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
     }
 
     delete [] rerr1;
     delete [] gerr1;
     delete [] berr1;
 
     img = dImage;
     return img;
 }
*/

void makeIndexedImage( const QuantizeMethod method, QImage& img )
{
    //img.save("/tmp/out_orig.png", "png", 50);
    switch (method)
    {
    case QuantizeMethod_DIFFUSE:
    {
        /*
        QImage ct = img.convertToFormat(QImage::Format_Indexed8);
        QVector<QRgb> color_table = ct.colorTable();
        qSort(color_table);
        int rca, gca, bca, rcb, gcb, bcb;
        const QColor ca = color_table.first();
        const QColor cb = color_table.last();;
        int rDiff, gDiff, bDiff;
        rDiff = (rcb = cb.red())   - (rca = ca.red());
        gDiff = (gcb = cb.green()) - (gca = ca.green());
        bDiff = (bcb = cb.blue())  - (bca = ca.blue());
        int ncols = 256;
        QColor *dPal = new QColor[ncols];
//         for (int i=0; i<ncols; i++) {
//             dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
//                              gca + gDiff * i / ( ncols - 1 ),
//                              bca + bDiff * i / ( ncols - 1 ) );
//         }
        for ( QVector<QRgb>::iterator it = color_table.begin();
              it != color_table.end();
              ++it )
        {
            dPal[(it-color_table.begin())] = QColor(*it);
        }
        dither(img, dPal, ncols);
        delete [] dPal;
        */
        //img.setNumColors(256);
        img = img.convertToFormat(QImage::Format_Indexed8, Qt::DiffuseDither);
        break;
    }
    case QuantizeMethod_THRESHOLD:
        img = img.convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither);
        break;
    case QuantizeMethod_ORDERED:
        img = img.convertToFormat(QImage::Format_Indexed8, Qt::OrderedDither);
        break;        
    case QuantizeMethod_MEDIANCUT:
        img = quantize_mediancut(img);
        img = img.convertToFormat( QImage::Format_Indexed8 );
        break;
//     case QuantizeMethod_MAGICK:
//         assert(false); // handled elsewhere, not here
//         break;
    default:
        assert(false); // everything must be handled
        break;
    }
}

ColorMapObject makeColorMapObject( const QImage& idx8 )
{
    // color table
    QVector<QRgb> colorTable = idx8.colorTable();
    ColorMapObject cmap;
    // numColors must be a power of 2
    int numColors = 1 << GifBitSize(idx8.colorCount());
    cmap.ColorCount = numColors;
    //std::cout << "numColors:" << numColors << std::endl;
    cmap.BitsPerPixel = idx8.depth();// should always be 8
    if ( cmap.BitsPerPixel != 8 )
    {
        std::cerr << "Incorrect bit depth" << std::endl;
        return cmap;
    }
    GifColorType* colorValues = (GifColorType*)malloc(cmap.ColorCount * sizeof(GifColorType));
    cmap.Colors = colorValues;
    int c = 0;
    for(; c < idx8.colorCount(); ++c)
    {
        //std::cout << "color " << c << " has " << qRed(colorTable[c]) << "," <<  qGreen(colorTable[c]) << "," << qBlue(colorTable[c]) << std::endl;
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
    return cmap;
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

    /*
    if ( method == QuantizeMethod_MAGICK )
    {
        char  arg0[] = "gifWrite";
        char* argv[] = { &arg0[0], NULL };
        int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
        Magick::InitializeMagick( *argv );

        // shortcut everything below, use magick
        std::vector<Magick::Image> frames;
        for ( QVector<QImage>::const_iterator cit = images.begin(); cit != images.end() ; ++cit )
        {
            int f = cit-images.begin();

            QImage qimg = cit->convertToFormat( QImage::Format_RGB888 );            
            Magick::Image x( qimg.width(), qimg.height(), "RGB", Magick::CharPixel, qimg.constBits() );

            int msec_delay = delays.at( f );
            x.animationDelay(msec_delay);
            x.quantizeColors( 256 ); 
            x.quantizeDither( true );
             if ( f < 3 ) 
             {
                 x.quantizeTreeDepth( 3 );
             }
             else
             {
                 x.quantizeTreeDepth( 2 );
             }
            x.quantize( );
            frames.push_back( x );
        }
        
        //std::vector<Magick::Image> optimized_frames;
        //Magick::optimizeImageLayers( &optimized_frames, frames.begin(), frames.end() );
        Magick::writeImages(frames.begin(), frames.end(), filename.toLocal8Bit().constData());      
        return true;
    }
    */

    QImage first = images.at(0);
    makeIndexedImage(method, first);
    ColorMapObject cmap = makeColorMapObject(first);

    int error = 0;
    GifFileType *gif = EGifOpenFileName(filename.toLocal8Bit().constData(), false, &error);
    if (!gif) return false;
    EGifSetGifVersion(gif, true);

    if (EGifPutScreenDesc(gif, first.width(), first.height(), 8, 0, &cmap) == GIF_ERROR)
    {
        std::cerr << "EGifPutScreenDesc returned error" << std::endl;
        return false;
    }
    free(cmap.Colors);
    
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
        //toWrite.save( QString("/tmp/out_%1.png").arg(it-images.begin()), "PNG", 50 );
        //int nc = 1 << GifBitSize(toWrite.colorCount());
        //std::cout << "numColors:" << nc << std::endl;


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

        ColorMapObject lcmap = makeColorMapObject(toWrite);
        if (EGifPutImageDesc(gif, 0, 0, toWrite.width(), toWrite.height(), 0, &lcmap) == GIF_ERROR)
        {
            std::cerr << "EGifPutImageDesc returned error" << std::endl;
        }
        free(lcmap.Colors);

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

