# All the services as a nice includable script containing the list of them

SET(OPDE_SERVICE_INCLUDES
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/services/
    ${OPDE_SOURCE_DIR}/src/services/binary
    ${OPDE_SOURCE_DIR}/src/services/game
    ${OPDE_SOURCE_DIR}/src/services/physics
    ${OPDE_SOURCE_DIR}/src/services/config
    ${OPDE_SOURCE_DIR}/src/services/worldrep
    ${OPDE_SOURCE_DIR}/src/services/link
    ${OPDE_SOURCE_DIR}/src/services/property
    ${OPDE_SOURCE_DIR}/src/services/inherit
    ${OPDE_SOURCE_DIR}/src/services/object
    ${OPDE_SOURCE_DIR}/src/services/render
    ${OPDE_SOURCE_DIR}/src/services/database
    ${OPDE_SOURCE_DIR}/src/services/input
    ${OPDE_SOURCE_DIR}/src/services/loop
    ${OPDE_SOURCE_DIR}/src/services/gui
    ${OPDE_SOURCE_DIR}/src/services/script
    ${OPDE_SOURCE_DIR}/src/services/material
    ${OPDE_SOURCE_DIR}/src/services/light
    ${OPDE_SOURCE_DIR}/src/services/draw
    ${OPDE_SOURCE_DIR}/src/services/room
)

# All the resulting libraries in a nice package as well
SET(OPDE_SERVICE_LIBRARIES
    OpdeWorldRepService
    OpdeBinaryService
    OpdeGameService
    OpdeConfigService
    OpdeLinkService
    OpdePropertyService
    OpdeInheritService
    OpdeObjectService
    OpdeRenderService
    OpdeLightService
    OpdeMaterialService
    OpdeDatabaseService
    OpdeInputService
    OpdeLoopService
    OpdeGUIService
    OpdeScriptService
    OpdeDrawService
    OpdeRoomService
)

# include the configuration from all the services (each config.cmake should append the OPDE_SERVICE_HEADERS and OPDE_SERVICE_SOURCES)
# that is done via: LIST(APPEND OPDE_SERVICE_SOURCES myfile.cpp) and simmilar...
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/binary/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/config/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/database/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/draw/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/game/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/gui/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/inherit/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/input/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/light/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/link/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/loop/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/material/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/object/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/physics/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/room/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/property/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/render/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/script/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/sim/config.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/services/worldrep/config.cmake )

# join the variables
SET(OPDE_SERVICE_FILES ${OPDE_SERVICE_HEADERS})
LIST(APPEND OPDE_SERVICE_FILES ${OPDE_SERVICE_SOURCES})

# To use this, just do INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
