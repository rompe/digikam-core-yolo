#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libnotificationmanager_SRCS
    dnotificationwrapper.cpp
)

if(APPLE)
    set(libnotificationmanager_SRCS
        ${libnotificationmanager_SRCS}
        macnotification.mm
    )
endif()

add_library(notificationmanager STATIC ${libnotificationmanager_SRCS})

target_link_libraries(notificationmanager

                      Qt5::Core
                      Qt5::Gui
                      Qt5::DBus
                      Qt5::Widgets
                      
                      KF5::Notifications
)