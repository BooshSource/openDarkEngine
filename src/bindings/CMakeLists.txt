include_directories(
    ${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/compat
    ${OPDE_SOURCE_DIR}/src/base/dyntype
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/base/loaders
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SOURCE_DIR}/src/services
)

add_library(OpdePython OBJECT
    bindings.cpp
    bindings.h
    ConfigServiceBinder.cpp
    ConfigServiceBinder.h
    DatabaseServiceBinder.cpp
    DatabaseServiceBinder.h
    DataFieldDescIteratorBinder.cpp
    DataFieldDescIteratorBinder.h
    DrawServiceBinder.cpp
    DrawServiceBinder.h
    DrawSheetBinder.cpp
    DrawSheetBinder.h
    DrawSourceBinder.cpp
    DrawSourceBinder.h
    FontDrawSourceBinder.cpp
    FontDrawSourceBinder.h
    GUIServiceBinder.cpp
    GUIServiceBinder.h
    InheritQueryResultBinder.cpp
    InheritQueryResultBinder.h
    InheritServiceBinder.cpp
    InheritServiceBinder.h
    InputServiceBinder.cpp
    InputServiceBinder.h
    LinkQueryResultBinder.cpp
    LinkQueryResultBinder.h
    LinkServiceBinder.cpp
    LinkServiceBinder.h
    LoopServiceBinder.cpp
    LoopServiceBinder.h
    ObjectServiceBinder.cpp
    ObjectServiceBinder.h
    PropertyServiceBinder.cpp
    PropertyServiceBinder.h
    PythonCallback.h
    PythonStruct.h
    RelationBinder.cpp
    RelationBinder.h
    RenderedImageBinder.cpp
    RenderedImageBinder.h
    RenderedLabelBinder.cpp
    RenderedLabelBinder.h
    RootBinder.cpp
    RootBinder.h
    ServiceBinder.cpp
    ServiceBinder.h
    StringIteratorBinder.cpp
    StringIteratorBinder.h
    TextureAtlasBinder.cpp
    TextureAtlasBinder.h
)
