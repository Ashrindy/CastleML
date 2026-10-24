project("castle-ml")

add_library(castle-ml SHARED
    src/dllmain.cpp
    src/mod-loader.cpp
    src/stub-functions.cpp
)

target_compile_features(castle-ml PUBLIC cxx_std_20)
target_include_directories(castle-ml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(castle-ml PRIVATE -static-libstdc++ -static-libgcc)
target_link_libraries(castle-ml PRIVATE detours inih)
