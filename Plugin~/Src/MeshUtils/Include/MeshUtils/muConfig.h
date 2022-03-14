#pragma once

//Set by CMake:
//   muEnableISPC

// available options:
//   muEnablePPL
//   muEnableTBB
//   muEnableAMP
//   muEnableSymbol

#ifdef _WIN32
    #define muEnableAMP
    #define muEnableSymbol
#endif

