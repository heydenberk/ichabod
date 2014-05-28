#ifndef QUANT_H
#define QUANT_H

#include <QImage>

enum QuantizeMethod
{
    QuantizeMethod_THRESHOLD,
    QuantizeMethod_DIFFUSE,
    QuantizeMethod_ORDERED,
    QuantizeMethod_MEDIANCUT
    //QuantizeMethod_MAGICK
};

QImage quantize_mediancut( const QImage& src );

#endif
