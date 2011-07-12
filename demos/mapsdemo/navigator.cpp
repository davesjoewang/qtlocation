/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "navigator.h"
#include "mapswidget.h"
#include "marker.h"

Navigator::Navigator(QGeoRoutingManager *routingManager,
                     QGeoSearchManager *searchManager,
                     MapsWidget *mapsWidget, const QString &address,
                     const QGeoRouteRequest &requestTemplate) :
    address(address),
    request(requestTemplate),
    routingManager(routingManager),
    searchManager(searchManager),
    mapsWidget(mapsWidget),
    routeObject(0),
    endMarker(0),
    startMarker(0)
{
}

Navigator::~Navigator()
{
    if (routeObject) {
        mapsWidget->map()->removeMapObject(routeObject);
        delete routeObject;
    }
    if (endMarker) {
        mapsWidget->map()->removeMapObject(endMarker);
        delete endMarker;
    }
    if (startMarker) {
        mapsWidget->map()->removeMapObject(startMarker);
        delete startMarker;
    }
}

void Navigator::start()
{
    QList<QGeoCoordinate> waypoints = request.waypoints();
    waypoints.append(mapsWidget->markerManager()->myLocation());
    request.setWaypoints(waypoints);

    startMarker = new Marker(Marker::StartMarker);
    startMarker->setCoordinate(mapsWidget->markerManager()->myLocation());
    startMarker->setName("Start point");
    mapsWidget->map()->addMapObject(startMarker);

    addressReply = searchManager->search(address);
    if (addressReply->isFinished()) {
        on_addressSearchFinished();
    } else {
        connect(addressReply, SIGNAL(error(QGeoSearchReply::Error,QString)),
                this, SIGNAL(searchError(QGeoSearchReply::Error,QString)));
        connect(addressReply, SIGNAL(finished()),
                this, SLOT(on_addressSearchFinished()));
    }
}

void Navigator::on_addressSearchFinished()
{
    if (addressReply->locations().isEmpty()) {
        addressReply->deleteLater();
        return;
    }

    QGeoLocation location = addressReply->locations().at(0);

    QList<QGeoCoordinate> waypoints = request.waypoints();
    waypoints.append(location.coordinate());
    request.setWaypoints(waypoints);

    routeReply = routingManager->calculateRoute(request);
    if (routeReply->isFinished()) {
        on_routingFinished();
    } else {
        connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
                this, SIGNAL(routingError(QGeoRouteReply::Error,QString)));
        connect(routeReply, SIGNAL(finished()),
                this, SLOT(on_routingFinished()));
    }

    endMarker = new Marker(Marker::EndMarker);
    endMarker->setCoordinate(location.coordinate());
    endMarker->setAddress(location.address());
    endMarker->setName("Destination");
    mapsWidget->map()->addMapObject(endMarker);

    addressReply->deleteLater();
}

QGeoRoute Navigator::route() const
{
    return firstRoute;
}

void Navigator::on_routingFinished()
{
    if (routeReply->routes().size() <= 0) {
        emit routingError(QGeoRouteReply::NoError, "No valid routes returned");
        routeReply->deleteLater();
        return;
    }

    QGeoRoute route = routeReply->routes().at(0);
    firstRoute = route;

    routeObject = new QGeoMapRouteObject;
    routeObject->setRoute(route);
    routeObject->setPen(QPen(Qt::blue, 2.0));

    mapsWidget->map()->addMapObject(routeObject);

    emit finished();
    routeReply->deleteLater();
}
