/***************************************************************************
**
** Copyright (c) 2016 Jolla Ltd.
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

#ifndef TOUCHEVENTFILTER_P_H
#define TOUCHEVENTFILTER_P_H

#include <displaystate.h>

#include "touchscreen.h"
#include "homeapplication.h"

class QDBusInterface;

class TouchScreenPrivate {
public:
    explicit TouchScreenPrivate(TouchScreen *q);

    void handleInputPolicyChange(bool inputEnabled);
    void handleDisplayStateChange(TouchScreen::DisplayState state);
    bool touchBlocked() const;
    void evaluateTouchBlocked();

    bool eatEvents;
    TouchScreen::DisplayState currentDisplayState;
    bool waitForTouchBegin;
    bool inputEnabled;
    bool touchBlockedState;
    int touchUnblockingDelayTimer;
    DeviceState::DisplayStateMonitor *displayState;
    QDBusInterface *mceRequest;

    TouchScreen *q_ptr;

    Q_DECLARE_PUBLIC(TouchScreen)
};

#endif // TOUCHEVENTFILTER_P_H
