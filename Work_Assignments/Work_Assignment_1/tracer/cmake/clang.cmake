#message("Clang/LLVM compiler selected")

SET(CONFIG_COMPILER__FLAGS__SSSE3 "-msse3")
SET(CONFIG_COMPILER__FLAGS__SSSE3 "-mssse3")
SET(CONFIG_COMPILER__FLAGS__SSE41 "-msse4.1")
SET(CONFIG_COMPILER__FLAGS__SSE42 "-msse4.2")
SET(CONFIG_COMPILER__FLAGS__AVX   "-mavx")
SET(CONFIG_COMPILER__FLAGS__AVX2  "-mf16c -mavx2 -mfma -mlzcnt  -mbmi -mbmi2")
SET(CONFIG_COMPILER__FLAGS__AVX512KNL "-march=native")
SET(CONFIG_COMPILER__FLAGS__AVX512SKX "-march=native")
SET(CONFIG_COMPILER__FLAGS__AVX512CLX "-march=native")
SET(CONFIG_COMPILER__FLAGS__NEON "-march=native")

# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-slp-vectorize -fcolor-diagnostics -fPIC -Wno-c++11-narrowing -Wno-unknown-pragmas -Wno-writable-strings")
# SET(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -g")
# SET(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}")
# SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

#-no-ansi-alias -restrict -fp-model fast -fimf-precision=low -no-prec-div -no-prec-sqrt"
