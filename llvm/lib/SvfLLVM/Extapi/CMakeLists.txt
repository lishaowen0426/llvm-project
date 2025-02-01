
# Define a custom command which generates the "extapi.bc" LLVM bytecode file
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/extapi.bc
  COMMAND clang-19      -w
                        -S
                        -c
                        -emit-llvm
                        -fno-discard-value-names
                        -Xclang -disable-O0-optnone
                        -o ${CMAKE_BINARY_DIR}/lib/extapi.bc
                        ${CMAKE_CURRENT_LIST_DIR}/extapi.c
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/extapi.c
)

# Add a custom target for generating the LLVM bytecode file (and add it to the default build targets)
add_custom_target(gen_extapi_ir ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/extapi.bc)



# Install the generated extapi.bc bytecode file
install(
  FILES ${CMAKE_BINARY_DIR}/lib/extapi.bc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
