file(GLOB OpenixCardSource *.cpp)
file(GLOB OpenixCardPayloads payloads/*.cpp)

add_executable(OpenixCard ${OpenixCardSource} ${OpenixCardPayloads})
target_link_libraries(OpenixCard PRIVATE OpenixIMG inicpp Threads::Threads)