#ifndef AGIF_H
#define AGIF_H

#include <QVector>
#include <QImage>

bool gifWrite ( const QVector<QImage> & images, const QVector<int>& delays, const QString& filename, bool loop = false );

#endif
