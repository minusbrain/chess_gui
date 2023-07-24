set (SOURCE_CPP
   main.cpp
   texture.cpp
   sprite.cpp
   assets.cpp
   human_gui_player.cpp
)

add_executable(chess_gui ${SOURCE_CPP})

target_link_libraries(chess_gui PRIVATE chess imgui SDL2::SDL2 SDL2_image::SDL2_image fmt::fmt-header-only)

#add_subdirectory(test)
