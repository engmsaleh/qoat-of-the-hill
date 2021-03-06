/**
 * Copyright (c) 2011 Nokia Corporation.
 *
 * Part of the Qt GameEnabler.
 */

#ifndef GEGLOBAL_H
#define GEGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GE_BUILD_LIB)
    #define Q_GE_EXPORT Q_DECL_EXPORT
#else
    #define Q_GE_EXPORT
#endif

#define Q_GE_BEGIN_NAMESPACE namespace GE {
#define Q_GE_END_NAMESPACE }

#endif // GEGLOBAL_H

