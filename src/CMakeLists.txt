set (SOURCE_CPP
   main.cpp
)

add_executable(chess_gui ${SOURCE_CPP})

target_link_libraries(chess_gui PRIVATE chess imgui)

#add_subdirectory(test)
