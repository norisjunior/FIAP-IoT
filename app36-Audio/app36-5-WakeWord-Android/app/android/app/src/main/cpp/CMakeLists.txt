cmake_minimum_required(VERSION 3.22)
project(wakeword)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EI_SDK ${CMAKE_CURRENT_SOURCE_DIR}/edge-impulse-sdk)
include(${EI_SDK}/cmake/utils.cmake)

# Fontes do SDK, do modelo e da ponte
RECURSIVE_FIND_FILE(SDK_C   "${EI_SDK}" "*.c")
RECURSIVE_FIND_FILE(SDK_CC  "${EI_SDK}" "*.cc")
RECURSIVE_FIND_FILE(SDK_CPP "${EI_SDK}" "*.cpp")
RECURSIVE_FIND_FILE(SDK_S   "${EI_SDK}" "*.S")
RECURSIVE_FIND_FILE(MODEL   "${CMAKE_CURRENT_SOURCE_DIR}/tflite-model" "*.cpp")

list(APPEND SOURCES ${SDK_C} ${SDK_CC} ${SDK_CPP} ${SDK_S} ${MODEL})

# So o porting do Android; os outros alvos quebram a compilacao
foreach(dir ambiq arduino brickml ceva-npn clib espressif ethos-core-driver
            himax himax-we2 iar infineon-psoc62 mbed mingw32 particle posix
            raspberry renesas-ra seeed-vision-ai silabs sony stm32-cubeai
            synaptics ti zephyr)
  list(FILTER SOURCES EXCLUDE REGEX "/porting/${dir}/")
endforeach()

# CMSIS e para Cortex-M, nao para Android
list(FILTER SOURCES EXCLUDE REGEX "/CMSIS/")

add_library(wakeword SHARED ${SOURCES} wakeword_ffi.cpp)

target_include_directories(wakeword PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${EI_SDK}
  ${EI_SDK}/third_party/flatbuffers/include
  ${EI_SDK}/third_party/gemmlowp
  ${EI_SDK}/third_party/ruy
)

target_compile_definitions(wakeword PRIVATE
  EI_PORTING_ANDROID=1
  TF_LITE_DISABLE_X86_NEON=1
  EI_CLASSIFIER_ENABLE_DETECTION_POSTPROCESS_OP=0
)

target_compile_options(wakeword PRIVATE
  -O3 -ffast-math
  -Wno-unused-parameter -Wno-sign-compare -Wno-strict-aliasing
)

find_library(log-lib log)
target_link_libraries(wakeword ${log-lib})
