#ifndef QAPI_GLOBAL_H
#define QAPI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QAPI_LIBRARY)
#define QAPI_EXPORT Q_DECL_EXPORT
#else
#define QAPI_EXPORT Q_DECL_IMPORT
#endif

#endif // QAPI_GLOBAL_H
