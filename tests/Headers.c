#include <limits.h>
#include "PseudoTbox.h"

#if defined(USE_OPTIONAL) && defined(__clang__)
#if __has_warning("-Wnull-to-nonoptional")
#pragma clang diagnostic error "-Wnull-to-nonoptional"
#endif
#endif

/* The intercepted getter preserves the Toolbox null-buffer size query. */
void null_buffer_size_query(void)
{
  int length;
  const int sizes[] = {0, 1, 4096, INT_MAX, -1};
  for (unsigned int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
    (void)pseudo_button_get_value(0, 0, 0, 0, sizes[i], &length,
                                __FILE__, __LINE__);
  }
}
