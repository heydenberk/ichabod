#ifndef QUANT_H
#define QUANT_H

#include <QImage>

enum QuantizeMethod
{
    QuantizeMethod_THRESHOLD,
    QuantizeMethod_DIFFUSE,
    QuantizeMethod_MEDIANCUT
};

QImage quantize_mediancut( const QImage& src );

#endif
