cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set(TRANSPORT_CATALOQUE_PROTO_FILES transport_catalogue.proto)
set(TRANSPORT_CATALOQUE_H_FILES domain.h geo.h graph.h json.h json_builder.h json_reader.h map_renderer.h ranges.h request_handler.h router.h serialization.h svg.h transport_catalogue.h transport_router.h)
set(TRANSPORT_CATALOQUE_CPP_FILES domain.cpp geo.cpp json.cpp json_builder.cpp json_reader.cpp main.cpp map_renderer.cpp request_handler.cpp serialization.cpp svg.cpp transport_catalogue.cpp transport_router.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOQUE_CPP_FILES} ${TRANSPORT_CATALOQUE_H_FILES} ${TRANSPORT_CATALOQUE_PROTO_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)