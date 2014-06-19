# - Find ClanLib
# ClanLib is a cross platform SDK geared toward making games. It is
# available from http://clanlib.org.
#
# Please define the following before starting this module
#   ClanLib_MAJOR_VERSION
#   ClanLib_MINOR_VERSION
#
# The following are defined by this module:
#   ClanLib_FOUND        - TRUE if ClanLib was found
#   ClanLib_INCLUDE_DIRS - Directory containing the ClanLib headers
#   ClanLib_LIBRARIES    - If invoked via FIND_PACKAGE(ClanLib COMPONENTS ...),
#                          will only contain the libraries matching each component.
#                          Otherwise, it will contain all ClanLib libraries found.
#
# For the components
#   App, Core, Display, GL, GUI, SWRender
#
# the following variables are set:
#   ClanLib_${COMPONENT}_LIBRARY - Full path to the component's library.


IF(ClanLib_INCLUDE_DIRS)
    SET(ClanLib_FIND_QUIETLY TRUE)
ENDIF(ClanLib_INCLUDE_DIRS)



IF(NOT ClanLib_FIND_COMPONENTS)
    SET(ClanLib_FIND_COMPONENTS  App  Display GL SWRender GUI)
ENDIF(NOT ClanLib_FIND_COMPONENTS)

MACRO(ClanLib_MSG MSG)
    IF(NOT ClanLib_FIND_QUIETLY)
        MESSAGE(STATUS ${MSG})
    ENDIF(NOT ClanLib_FIND_QUIETLY)
ENDMACRO(ClanLib_MSG)

MACRO(ClanLib_ERR MSG)
    IF(ClanLib_FIND_REQUIRED)
        MESSAGE(SEND_ERROR ${MSG})
    ELSE(ClanLib_FIND_REQUIRED)
        ClanLib_MSG(${MSG})
    ENDIF(ClanLib_FIND_REQUIRED)
ENDMACRO(ClanLib_ERR)

MACRO(ClanLib_FIND_COMPONENT COMPONENT)
    ClanLib_MSG("Checking for Clan${COMPONENT}")

    FIND_LIBRARY(
        ClanLib_${COMPONENT}_LIBRARY
        NAMES
            clan${ClanLib_MAJOR_VERSION}${ClanLib_MINOR_VERSION}${COMPONENT}
			clan${COMPONENT}-dll-debug          clan${COMPONENT}-dll
			clan${COMPONENT}-static-mtdll-debug clan${COMPONENT}-static-mtdll
            clan${COMPONENT}-static-mt-debug    clan${COMPONENT}-static-mt
			clan${COMPONENT}-dll-debug.lib          clan${COMPONENT}-dll.lib
			clan${COMPONENT}-static-mtdll-debug.lib clan${COMPONENT}-static-mtdll.lib
            clan${COMPONENT}-static-mt-debug.lib    clan${COMPONENT}-static-mt.lib
        HINTS
			${ClanLib_LIBRARY_DIRS}
            ${CLANLIB_ROOT_DIR}/lib
            ${CLANLIB_ROOT_DIR}/lib/Win32
            /lib
            /usr/lib
            /usr/local/lib
        DOC "Library name for clan${COMPONENT}.")
    IF(ClanLib_${COMPONENT}_LIBRARY)
        SET(ClanLib_${COMPONENT}_FOUND TRUE)
        ClanLib_MSG("Checking for Clan${COMPONENT} -- ${ClanLib_${COMPONENT}_LIBRARY}")
    ELSE(ClanLib_${COMPONENT}_LIBRARY)
        SET(ClanLib_${COMPONENT}_FOUND FALSE)
        IF(ClanLib_FIND_REQUIRED_${COMPONENT})
            ClanLib_ERR("Checking for Clan${COMPONENT} -- not found")
        ELSE(ClanLib_FIND_REQUIRED_${COMPONENT})
            ClanLib_MSG("Checking for Clan${COMPONENT} -- not found")
        ENDIF(ClanLib_FIND_REQUIRED_${COMPONENT})
    ENDIF(ClanLib_${COMPONENT}_LIBRARY)
ENDMACRO(ClanLib_FIND_COMPONENT)

ClanLib_MSG("Checking for ClanLib")
FIND_PATH(ClanLib_INCLUDE_DIRS ClanLib/core.h
    ${ClanLib_ROOT_DIR}/include ${ClanLib_ROOT_DIR}/include/ClanLib-${ClanLib_MAJOR_VERSION}.${ClanLib_MINOR_VERSION}
    /usr/local/include          /usr/local/include/ClanLib-${ClanLib_MAJOR_VERSION}.${ClanLib_MINOR_VERSION}
    /usr/include                /usr/include/ClanLib-${ClanLib_MAJOR_VERSION}.${ClanLib_MINOR_VERSION}
    DOC "Where to find the ClanLib includes.")
IF(ClanLib_INCLUDE_DIRS)
    ClanLib_MSG("Checking for ClanLib -- headers")
ELSE(ClanLib_INCLUDE_DIRS)
    ClanLib_ERR("Checking for ClanLib -- headers not found")
ENDIF(ClanLib_INCLUDE_DIRS)

ClanLib_FIND_COMPONENT(Core)
IF(ClanLib_INCLUDE_DIRS AND ClanLib_Core_LIBRARY)
    SET(ClanLib_FOUND TRUE)
    SET(ClanLib_LIBRARIES ${ClanLib_Core_LIBRARY})
ELSE(ClanLib_INCLUDE_DIRS AND ClanLib_Core_LIBRARY)
    SET(ClanLib_FOUND FALSE)
ENDIF(ClanLib_INCLUDE_DIRS AND ClanLib_Core_LIBRARY)

ClanLib_MSG("Checking for other ClanLib components")
FOREACH(COMPONENT ${ClanLib_FIND_COMPONENTS})
    ClanLib_FIND_COMPONENT(${COMPONENT})
    IF(ClanLib_${COMPONENT}_LIBRARY)
        LIST(APPEND ClanLib_LIBRARIES ${ClanLib_${COMPONENT}_LIBRARY})
    ENDIF(ClanLib_${COMPONENT}_LIBRARY)
ENDFOREACH(COMPONENT)

MARK_AS_ADVANCED(
    ClanLib_INCLUDE_DIRS
    ClanLib_App_LIBRARY
    ClanLib_Core_LIBRARY
    ClanLib_Display_LIBRARY
    ClanLib_GL_LIBRARY
    ClanLib_SWRender_LIBRARY
    ClanLib_GUI_LIBRARY
    )

