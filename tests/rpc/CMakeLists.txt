
set(name unittests-opentxs-rpc)

set(cxx-sources
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Test_Rpc.cpp
  ${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp
)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/tests
        ${GTEST_INCLUDE_DIRS}
)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs ${GTEST_LIBRARY})

if(NOT OT_BUNDLED_PROTOBUF)
  target_link_libraries(${name} ${PROTOBUF_LITE_LIBRARIES})
endif()

if(NOT OT_BUNDLED_OPENTXS_PROTO)
  target_link_libraries(${name} opentxs-proto)
endif()

set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
