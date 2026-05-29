cmake_minimum_required(VERSION 3.16)

project(MeshTalk VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appMeshTalk
    main.cpp
)

qt_add_qml_module(appMeshTalk
    URI MeshTalk
    QML_FILES
        Main.qml
    SOURCES
        backend.h backend.cpp
        packet.h packet.cpp
        peermanager.h peermanager.cpp
        peers.h peers.cpp
        packetqueue.h packetqueue.cpp
        udptransport.h udptransport.cpp
        crypto.h crypto.cpp
)

set_target_properties(appMeshTalk PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# OpenSSL — Mac uses system OpenSSL, Android uses prebuilt from KDAB
if(ANDROID)
    set(OPENSSL_ROOT_DIR ~/Qt/android_openssl/ssl_3/${ANDROID_ABI})
    set(OPENSSL_INCLUDE_DIR ~/Qt/android_openssl/ssl_3/include)
    include(~/Qt/android_openssl/android_openssl.cmake)
    target_link_libraries(appMeshTalk
        PRIVATE Qt6::Quick Qt6::Network
    )
    add_android_openssl_libraries(appMeshTalk)
else()
    include_directories("C:/Program Files/OpenSSL-Win64/include")
    target_link_libraries(appMeshTalk
        PRIVATE Qt6::Quick Qt6::Network
        "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
        "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
    )
endif()