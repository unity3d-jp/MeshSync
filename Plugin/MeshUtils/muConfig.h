#pragma once

// available options:
//   muEnablePPL
//   muEnableTBB
//   muEnableISPC
//   muEnableAMP

#ifdef _WIN32
    #define muEnablePPL
    #define muEnableISPC
    #define muEnableAMP
#endif

