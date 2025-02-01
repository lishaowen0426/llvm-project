


add_llvm_component_library(
    LLVMSVF
    SVFAnalysis.cpp
    SVFTransform.cpp

    ADDITIONAL_HEADER_DIRS
        ${LLVM_MAIN_INCLUDE_DIR}/svf
        ${LLVM_MAIN_INCLUDE_DIR}/svf-llvm

    LINK_COMPONENTS
        SvfCore
        SvfLLVM
        Analysis
        BitReader
        BitWriter
        Core
        IRReader
        Scalar
        Support
        TransformUtils
)
