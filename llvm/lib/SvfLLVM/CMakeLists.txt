
add_subdirectory(Extapi)

file(GLOB_RECURSE SVF_LLVM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  *.cpp)




add_llvm_component_library(LLVMSvfLLVM
    ${SVF_LLVM_SOURCES}

    DEPENDS
    intrinsics_gen
    gen_extapi_ir

    LINK_COMPONENTS
    Analysis
    BitWriter
    Core
    InstCombine
    Instrumentation
    IPO
    IRReader
    Linker
    ScalarOpts
    Support
    Target
    TransformUtils
    Passes
    SvfCore

)



target_include_directories(LLVMSvfLLVM
    PUBLIC
    $<BUILD_INTERFACE:${LLVM_MAIN_INCLUDE_DIR}/svf>
    $<BUILD_INTERFACE:${LLVM_MAIN_INCLUDE_DIR}/svf-llvm>
    $<BUILD_INTERFACE:${LLVM_INCLUDE_DIR}/svf>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/svf>
)








