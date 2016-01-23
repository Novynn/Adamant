#ifndef CORE_GLOBAL_H
#define CORE_GLOBAL_H

#include <QtCore/QtGlobal>

#if CORE_INTERNAL
    #define CORE_EXTERN Q_DECL_EXPORT
#else
    #define CORE_EXTERN Q_DECL_IMPORT
#endif

#endif // CORE_GLOBAL_H

