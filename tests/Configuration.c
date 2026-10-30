/* Check interception as seen by a client of the CMake targets. */
#include "PseudoFlex.h"
#include "PseudoIO.h"

#if defined(FORTIFY) != EXPECT_FORTIFY
#error Unexpected Fortify configuration inherited by a CBPseudo client
#endif

#if defined(flex_alloc) != EXPECT_FORTIFY
#error Flex interception does not match the client configuration
#endif
