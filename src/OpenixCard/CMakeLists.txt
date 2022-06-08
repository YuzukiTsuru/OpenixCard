file(GLOB libOpenixCardSource *.cpp)
file(GLOB libOpenixCardPayloads payloads/*.cpp)

add_library(libOpenixCard ${libOpenixCardSource} ${libOpenixCardPayloads})
target_link_libraries(libOpenixCard PRIVATE OpenixIMG inicpp GenIMG)