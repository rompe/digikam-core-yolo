#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

kde_enable_exceptions()

set(libdimg_SRCS
    dimg.cpp
    dimg_debug.cpp
    drawdecoding.cpp
    dimgscale.cpp
    dcolor.cpp
    dcolorcomposer.cpp
    imagehistory/dimagehistory.cpp
    imagehistory/filteraction.cpp
    imagehistory/historyimageid.cpp
)

set(libdimgfilters_SRCS
    dklcms/digikam-lcms.cpp
    filters/dimgbuiltinfilter.cpp
    filters/dimgthreadedfilter.cpp
    filters/dimgthreadedanalyser.cpp
    filters/dimgfiltermanager.cpp
    filters/dimgfiltergenerator.cpp
    filters/filteractionfilter.cpp
    filters/randomnumbergenerator.cpp
    filters/rawprocessingfilter.cpp
    filters/decorate/borderfilter.cpp
    filters/decorate/bordersettings.cpp
    filters/decorate/texturefilter.cpp
    filters/film/filmfilter.cpp
    filters/fx/blurfilter.cpp
    filters/fx/blurfxfilter.cpp
    filters/fx/colorfxfilter.cpp
    filters/fx/colorfxsettings.cpp
    filters/fx/distortionfxfilter.cpp
    filters/fx/charcoalfilter.cpp
    filters/fx/embossfilter.cpp
    filters/fx/filmgrainfilter.cpp
    filters/fx/filmgrainsettings.cpp
    filters/fx/invertfilter.cpp
    filters/fx/pixelsaliasfilter.cpp
    filters/fx/oilpaintfilter.cpp
    filters/fx/raindropfilter.cpp
    filters/auto/autolevelsfilter.cpp
    filters/auto/autoexpofilter.cpp
    filters/auto/equalizefilter.cpp
    filters/auto/stretchfilter.cpp
    filters/auto/normalizefilter.cpp
    filters/cb/cbfilter.cpp
    filters/cb/cbsettings.cpp
    filters/bcg/bcgfilter.cpp
    filters/bcg/bcgsettings.cpp
    filters/bw/bwsepiafilter.cpp
    filters/bw/bwsepiasettings.cpp
    filters/bw/tonalityfilter.cpp
    filters/bw/infraredfilter.cpp
    filters/bw/mixerfilter.cpp
    filters/bw/mixersettings.cpp
    filters/hsl/hslfilter.cpp
    filters/hsl/hslsettings.cpp
    filters/hsl/hspreviewwidget.cpp
    filters/icc/iccmanager.cpp
    filters/icc/iccprofile.cpp
    filters/icc/iccprofilesettings.cpp
    filters/icc/icctransform.cpp
    filters/icc/icctransformfilter.cpp
    filters/icc/iccsettingscontainer.cpp
    filters/icc/iccsettings.cpp
    filters/lc/localcontrastfilter.cpp
    filters/lc/localcontrastsettings.cpp
    filters/lc/localcontrastcontainer.cpp
    filters/nr/nrfilter.cpp
    filters/nr/nrestimate.cpp
    filters/nr/nrsettings.cpp
    filters/sharp/sharpenfilter.cpp
    filters/sharp/unsharpmaskfilter.cpp
    filters/sharp/sharpsettings.cpp
    filters/levels/imagelevels.cpp
    filters/levels/levelsfilter.cpp
    filters/levels/imagehistogram.cpp
    filters/levels/histogrambox.cpp
    filters/levels/histogramwidget.cpp
    filters/levels/histogrampainter.cpp
    filters/curves/curvescontainer.cpp
    filters/curves/imagecurves.cpp
    filters/curves/curvesfilter.cpp
    filters/curves/curvessettings.cpp
    filters/curves/curveswidget.cpp
    filters/curves/curvesbox.cpp
    filters/wb/wbcontainer.cpp
    filters/wb/wbfilter.cpp
    filters/wb/wbsettings.cpp
    filters/transform/freerotationfilter.cpp
    filters/transform/freerotationsettings.cpp
    filters/transform/shearfilter.cpp
    filters/transform/autocrop.cpp
    filters/greycstoration/greycstorationfilter.cpp
    filters/greycstoration/greycstorationsettings.cpp
    filters/lens/antivignettingfilter.cpp
    filters/lens/antivignettingsettings.cpp
    filters/lens/lensdistortionfilter.cpp
    filters/lens/pixelaccess.cpp
)

# ==================================================================================================
# get the gcc version

# CImg.h version 1.2.8 do not compile fine with gcc 4.3.x
# See bug #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
# Using -fno-tree-pre is work around this problem.

# TODO is this hack anymore required?
if(CMAKE_COMPILER_IS_GNUCXX)
    exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} --version OUTPUT_VARIABLE VERSION_GCC_INFO)
    string(REGEX MATCH " [34]\\.[0-9]\\.[0-9]" GCC_VERSION "${VERSION_GCC_INFO}")

    # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
    if(NOT GCC_VERSION)
        string(REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" GCC_VERSION "${VERSION_GCC_INFO}")
    endif()

    if (${GCC_VERSION} VERSION_LESS "4.3.0")
        message(STATUS "Adjusting compilation flags for GCC version (${GCC_VERSION} )")
        add_definitions(-fno-tree-pre)
    endif()
endif()

# ugly hack, but better than listing these in main cmakelists or polluting includes
include_directories(imagehistory/ 
                      filters/
                      filters/decorate/
                      filters/film/
                      filters/fx/
                      filters/auto/
                      filters/cb/
                      filters/bcg/
                      filters/bw/
                      filters/hsl/
                      filters/icc/
                      filters/lc/
                      filters/nr/
                      filters/sharp/
                      filters/levels/
                      filters/curves/
                      filters/wb/
                      filters/transform/
                      filters/greycstoration/
                      filters/lens/
                      loaders/
)

if(LQR-1_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/transform/contentawarefilter.cpp
    )
    include_directories(${LQR-1_INCLUDE_DIRS})
endif()

if(LENSFUN_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/lens/lensfunfilter.cpp
        filters/lens/lensfuncameraselector.cpp
        filters/lens/lensfuniface.cpp
        filters/lens/lensfunsettings.cpp
    )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

if(EIGEN3_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/sharp/refocusfilter.cpp
        filters/sharp/matrix.cpp
    )
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

set(libdimgloaders_SRCS
    loaders/dimgloader.cpp
    loaders/pngloader.cpp
    loaders/jpegloader.cpp
    loaders/tiffloader.cpp
    loaders/rawloader.cpp
    loaders/ppmloader.cpp
    loaders/qimageloader.cpp
    loaders/pgfloader.cpp
    loaders/jpegsettings.cpp
    loaders/pngsettings.cpp
    loaders/tiffsettings.cpp
    loaders/pgfsettings.cpp
)

# JPEG2000 support

if(JASPER_FOUND)
    # this should be compiled separately to the lib, cleans if checks
    set(libdimgloaders_SRCS
        ${libdimgloaders_SRCS}
        loaders/jp2kloader.cpp
        loaders/jp2ksettings.cpp
    )
endif()

add_library(dimg STATIC ${libdimgloaders_SRCS} ${libdimgfilters_SRCS} ${libdimg_SRCS})

if(X11_FOUND)
    target_link_libraries(dimg Qt5::X11Extras ${X11_LIBRARIES})
endif()

if(JASPER_FOUND)
    target_link_libraries(dimg ${JASPER_LIBRARIES})
endif()

# LibLqr-1 library rules for content-aware filter
if(LQR-1_FOUND)
    target_link_libraries(dimg ${LQR-1_LIBRARIES})
endif()

add_dependencies(dimg digikam-gitversion) # needed by pngloader. TODO extraction may speed-up compilation of dimg and its dependents

# TODO clear this up. users should be using more complete path than direct one...
# TODO this is just the same as with include_directories, macro?
target_include_directories(dimg
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/loaders>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/film>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/fx>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/auto>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/cb>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/bcg>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/bw>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/hsl>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/icc>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/lc>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/nr>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/levels>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/curves>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/wb>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/transform>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/greycstoration>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filters/lens>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dklcms>
)

# TODO ugly way to expose lensfun include dirs to users of dimg.
if(LENSFUN_FOUND)
    target_include_directories(dimg PUBLIC ${LENSFUN_INCLUDE_DIRS})
    target_link_libraries(dimg ${LENSFUN_LIBRARIES})
endif()

# for nrfilter
if(OpenCV_FOUND)
    target_link_libraries(dimg ${OpenCV_LIBRARIES})
endif()

target_link_libraries(dimg

                      digikamwidgets
                      dmetadata
                      dthread
                      jpegutils
                      pgfutils
                      3rdparty # KMemoryInfo
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Concurrent # filters
                      Qt5::Widgets
                      Qt5::Xml
                      
                      KF5::KExiv2
                      KF5::KDcraw
                      
                      # Required by CImg library which is using pthread internally.
                      ${CMAKE_THREAD_LIBS_INIT}

                      ${LCMS2_LIBRARIES} # filters
                      
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
)
