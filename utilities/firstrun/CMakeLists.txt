#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfirstrun_SRCS
    assistantdlg.cpp
    assistantdlgpage.cpp
    welcomepage.cpp
    collectionpage.cpp
    rawpage.cpp
    tooltipspage.cpp
    previewpage.cpp
    openfilepage.cpp
    metadatapage.cpp
    startscanpage.cpp
)

add_library(firstrun STATIC ${libfirstrun_SRCS})

target_link_libraries(firstrun

                      digikamdatabase

                      Qt5::Widgets
                      
                      KF5::WidgetsAddons
                      KF5::ConfigWidgets
                      KF5::KIOWidgets
                      
                      KF5::KDcraw
)
