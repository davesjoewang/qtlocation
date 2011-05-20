TARGET = qml_location_mapviewer
TEMPLATE=app

QT += declarative network
SOURCES += qmlmapviewer.cpp

symbian {
    TARGET.CAPABILITY = NetworkServices Location ReadUserData WriteUserData
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
}

RESOURCES += \
    mapviewer.qrc

target.path = $$[QT_INSTALL_DEMOS]/qtlocation/declarative/mapviewer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro *.qml *.png *.sci
sources.path = $$[QT_INSTALL_DEMOS]/qtlocation/declarative/mapviewer

INSTALLS += target sources
