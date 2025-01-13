


add_llvm_component_library(
    LLVMSVF
    SVFAnalysis.cpp
    SVFTransform.cpp

    ADDITIONAL_HEADER_DIRS
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/SVF
       ${SVF_INSTALL_INCLUDE_DIR} 

    LINK_LIBS
        SvfLLVM
        SvfCore
    LINK_COMPONENTS
        Analysis
        BitReader
        BitWriter
        Core
        FrontendOpenMP
        InstCombine
        IRReader
        Linker
        Object
        ProfileData
        Scalar
        Support
        TargetParser
        TransformUtils
)