add_subdirectory(aelib)
add_subdirectory(json)

add_subdirectory(fmt EXCLUDE_FROM_ALL)

#=================== IMGUI ===================

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui)

target_sources( imgui
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
                )

target_include_directories( imgui
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            PUBLIC ${SDL2_DIR}/include
                            )

target_link_libraries(imgui PUBLIC ${SDL_TTF_LIBRARIES} ${SDL2_LIBRARIES} ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES})
