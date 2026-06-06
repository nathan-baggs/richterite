add_library(winmm SHARED
  winmm.def
  winmm.cpp
)
install(TARGETS winmm DESTINATION bin)
target_compile_options(winmm PRIVATE
  -Wall
  -Wextra
  -pedantic
  -Werror
  -Wconversion-null
  -Wmissing-declarations
  -Woverlength-strings
  -Wpointer-arith
  -Wunused-local-typedefs
  -Wunused-result
  -Wvarargs
  -Wvla
  -Wwrite-strings
  -Wno-cast-function-type
  -Wno-expansion-to-defined
  -Wno-error=attributes
  -Wno-error=missing-declarations
)
target_compile_definitions(winmm PUBLIC -DNOMINMAX)
target_link_libraries(winmm PUBLIC stdc++exp)
set_target_properties(winmm PROPERTIES PREFIX "")

