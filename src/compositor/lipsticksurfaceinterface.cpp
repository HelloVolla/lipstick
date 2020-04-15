/***************************************************************************
**
** Copyright (c) 2014 Jolla Ltd.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "lipsticksurfaceinterface.h"

LipstickOomScoreOp::LipstickOomScoreOp(int score)
                  : QWaylandSurfaceOp((QWaylandSurfaceOp::Type)Type)
                  , m_score(score)
{
}
