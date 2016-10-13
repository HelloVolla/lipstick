/***************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Matt Vogt <matthew.vogt@jollamobile.com>
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

#include "vpnagent.h"

#include <QGuiApplication>
#include "homewindow.h"
#include <QQuickItem>
#include <QQmlContext>
#include <QScreen>
#include <QTimer>
#include <QDBusArgument>
#include <QDBusConnection>
#include "utilities/closeeventeater.h"
#include "lipstickqmlpath.h"

VpnAgent::VpnAgent(QObject *parent) :
    QObject(parent),
    m_window(0)
{
    QTimer::singleShot(0, this, SLOT(createWindow()));
}

VpnAgent::~VpnAgent()
{
    delete m_window;
}

void VpnAgent::createWindow()
{
    m_window = new HomeWindow();
    m_window->setGeometry(QRect(QPoint(), QGuiApplication::primaryScreen()->size()));
    m_window->setCategory(QLatin1String("dialog"));
    m_window->setWindowTitle("VPN Agent");
    m_window->setContextProperty("vpnAgent", this);
    m_window->setContextProperty("initialSize", QGuiApplication::primaryScreen()->size());
    m_window->setSource(QmlPath::to("connectivity/VpnAgent.qml"));
    m_window->installEventFilter(new CloseEventEater(this));
}

void VpnAgent::setWindowVisible(bool visible)
{
    if (visible) {
        if (!m_window->isVisible()) {
            m_window->showFullScreen();
            emit windowVisibleChanged();
        }
    } else if (m_window != 0 && m_window->isVisible()) {
        m_window->hide();
        emit windowVisibleChanged();
    }
}

bool VpnAgent::windowVisible() const
{
    return m_window != 0 && m_window->isVisible();
}

void VpnAgent::respond(const QString &path, const QVariantMap &details)
{
    // Marshall our response
    QVariantMap response;
    for (QVariantMap::const_iterator it = details.cbegin(), end = details.cend(); it != end; ++it) {
        const QVariantMap &field(it.value().value<QVariantMap>());
        const QVariant fieldValue = field.value(QStringLiteral("Value"));
        const QString fieldRequirement = field.value(QStringLiteral("Requirement")).toString();
        if (fieldRequirement == QStringLiteral("mandatory") ||
            (fieldRequirement != QStringLiteral("informational") && fieldValue.isValid())) {
            response.insert(it.key(), fieldValue);
        }
    }

    QList<Request>::iterator it = m_pending.begin(), end = m_pending.end();
    for ( ; it != end; ++it) {
        Request &request(*it);
        if (request.path == path) {
            request.reply << response;
            if (!QDBusConnection::systemBus().send(request.reply)) {
                qWarning() << "Unable to transmit response for:" << path;
            }
            m_pending.erase(it);

            if (!m_pending.isEmpty()) {
                const Request &nextRequest(m_pending.front());
                emit inputRequested(nextRequest.path, nextRequest.details);
            }
            return;
        }
    }

    qWarning() << "Unable to send response for:" << path;
}

void VpnAgent::decline(const QString &path)
{
    QList<Request>::iterator it = m_pending.begin(), end = m_pending.end();
    for ( ; it != end; ++it) {
        Request &request(*it);
        if (request.path == path) {
            if (!QDBusConnection::systemBus().send(request.cancelReply)) {
                qWarning() << "Unable to transmit cancel for:" << path;
            }
            m_pending.erase(it);

            if (!m_pending.isEmpty()) {
                const Request &nextRequest(m_pending.front());
                emit inputRequested(nextRequest.path, nextRequest.details);
            }
            return;
        }
    }

    qWarning() << "Unable to cancel pending request for:" << path;
}

void VpnAgent::Release()
{
}

void VpnAgent::ReportError(const QDBusObjectPath &path, const QString &message)
{
    emit errorReported(path.path(), message);
}

namespace {

template<typename T>
QVariant extract(const QDBusArgument &arg)
{
    T rv;
    arg >> rv;
    return QVariant::fromValue(rv);
}

}

QVariantMap VpnAgent::RequestInput(const QDBusObjectPath &path, const QVariantMap &details)
{
    // Extract the details from DBus marshalling
    QVariantMap extracted(details);
    for (QVariantMap::iterator it = extracted.begin(), end = extracted.end(); it != end; ++it) {
        *it = extract<QVariantMap>(it.value().value<QDBusArgument>());
    }

    // Inform the caller that the reponse will be asynchronous
    QDBusContext::setDelayedReply(true);

    m_pending.append(Request(path.path(), extracted, QDBusContext::message()));
    Request &newRequest(m_pending.back());

    if (m_pending.size() == 1) {
        // No request was previously in progress
        emit inputRequested(newRequest.path, newRequest.details);
    }

    return QVariantMap();
}

void VpnAgent::Cancel()
{
    if (!m_pending.isEmpty()) {
        Request canceledRequest(m_pending.takeFirst());
        emit requestCanceled(canceledRequest.path);

        if (!m_pending.isEmpty()) {
            const Request &nextRequest(m_pending.front());
            emit inputRequested(nextRequest.path, nextRequest.details);
        }
    }
}

VpnAgent::Request::Request(const QString &path, const QVariantMap &details, const QDBusMessage &request)
    : path(path)
    , details(details)
    , reply(request.createReply())
    , cancelReply(request.createErrorReply(QStringLiteral("net.connman.vpn.Agent.Error.Canceled"), QString()))
{
}

