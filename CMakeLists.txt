cmake_minimum_required(VERSION 3.15)

# static runtime for MSVC (no vc redist needed)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

project(CtrlDtoBackspace LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# small size by default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

# exe always go to build/ (installer read from here)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
foreach(cfg RELEASE DEBUG RELWITHDEBINFO MINSIZEREL)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR})
endforeach()

add_executable(CtrlDtoBackspace WIN32
  src/main.cpp
  src/application.cpp
  src/keyboard_hook.cpp
  src/tray.cpp
  src/settings.cpp
  src/startup.cpp
  resources/resource.rc
)

target_include_directories(CtrlDtoBackspace PRIVATE src resources)

target_compile_definitions(CtrlDtoBackspace PRIVATE
  UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
  WINVER=0x0A00 _WIN32_WINNT=0x0A00
)

target_link_libraries(CtrlDtoBackspace PRIVATE user32 shell32 advapi32)

if(MSVC)
  target_compile_options(CtrlDtoBackspace PRIVATE /W3 /GL /GS- /GR- /EHs-c-)
  # /MANIFEST:NO because we have our own manifest in resource.rc
  target_link_options(CtrlDtoBackspace PRIVATE /LTCG /OPT:REF /OPT:ICF /MANIFEST:NO)
else()
  target_compile_options(CtrlDtoBackspace PRIVATE
    -Wall -Wextra -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)
  target_link_options(CtrlDtoBackspace PRIVATE
    -static -s -Wl,--gc-sections)
endif()
