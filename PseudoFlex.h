/*
 * CBPseudoLib: Fortified alternative to Acorn's flex memory library
 * Copyright (C) 2008 Christopher Bazley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* PseudoFlex.h declares macros to mimic the API provided by Acorn's Flex
   library in order to redirect function calls to an alternative
   implementation based upon Simon P. Bullen's fortified memory allocation
   shell. This allows stress testing, detection of memory leaks and heap
   corruption, etc.

Dependencies: ANSI C library, Fortify.
Message tokens: None.
History:
  CJB: 27-Jan-08: Created.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.

*/

#ifndef PseudoFlex_h
#define PseudoFlex_h

/* Acorn C/C++ library headers */
#include <flex.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

/* Define macros to intercept calls to Acorn's 'flex' library and replace
 * them with calls to the equivalent 'PseudoFlex' functions (i.e. so that the
 * fortified memory allocation shell is used instead).
 */

#define flex_alloc(anchor, n) \
          PseudoFlex_alloc(anchor, n, __FILE__, __LINE__)

#define flex_free(anchor) \
          PseudoFlex_free(anchor, __FILE__, __LINE__)

#define flex_size(anchor) \
          PseudoFlex_size(anchor)

#define flex_extend(anchor, newsize) \
          PseudoFlex_extend(anchor, newsize, __FILE__, __LINE__)

#define flex_midextend(anchor, at, by) \
          PseudoFlex_midextend(anchor, at, by, __FILE__, __LINE__)

#define flex_reanchor(to, from) \
          PseudoFlex_reanchor(to, from)

#define flex_set_budge(newstate) \
          PseudoFlex_set_budge(newstate)

#define flex_init(program_name, error_fd, dynamic_size) \
          PseudoFlex_init(program_name, error_fd, dynamic_size)

#define flex_save_heap_info(filename) \
          PseudoFlex_save_heap_info(filename)

#define flex_compact() \
          PseudoFlex_compact()

#define flex_set_deferred_compaction(newstate) \
          PseudoFlex_set_deferred_compaction(newstate)

#endif

int PseudoFlex_alloc(flex_ptr anchor, int n, const char *file, unsigned long line);

void PseudoFlex_free(flex_ptr anchor, const char *file, unsigned long line);

int PseudoFlex_size(flex_ptr anchor);

int PseudoFlex_extend(flex_ptr anchor, int newsize, const char *file, unsigned long line);

int PseudoFlex_midextend(flex_ptr anchor, int at, int by, const char *file, unsigned long line);

int PseudoFlex_reanchor(flex_ptr to, flex_ptr from);

int PseudoFlex_set_budge(int newstate);

void PseudoFlex_init(char *program_name, _Optional int *error_fd, int dynamic_size);

void PseudoFlex_save_heap_info(char *filename);

int PseudoFlex_compact(void);

int PseudoFlex_set_deferred_compaction(int newstate);

#endif
