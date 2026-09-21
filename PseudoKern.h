/*
 * CBPseudoLib: Error injection veneer to Acorn's C library kernel
 * Copyright (C) 2012 Christopher Bazley
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

/* PseudoKern.h declares macros to mimic the API provided by Acorn's C
   library kernel in order to redirect function calls to an alternative
   implementation that returns errors if allocations via Simon P. Bullen's
   fortified memory allocation shell fail. This allows stress testing.

Dependencies: Acorn C library kernel, Fortify.
Message tokens: None.
History:
  CJB: 29-Apr-12: Created.
  CJB: 27-Dec-14: Exported the pseudokern_fail function for internal use.
  CJB: 31-Jan-16: Fixed an error in the definition of macro _kernel_swi_c
                  which referred to a non-existent argument 'carr'.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.

*/

#ifndef PseudoKern_h
#define PseudoKern_h

/* Acorn C/C++ library headers */
#include <kernel.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

/* Define macros to intercept calls to Acorn's C library kernel and replace
 * them with calls to the equivalent 'pseudokern' functions.
 */

#define _kernel_swi(no, in, out) \
          pseudokern_swi(no, in, out, __FILE__, __LINE__)

#define _kernel_swi_c(no, in, out, carry) \
          pseudokern_swi_c(no, in, out, carry, __FILE__, __LINE__)

#define _kernel_osbyte(op, x, y) \
          pseudokern_osbyte(op, x, y, __FILE__, __LINE__)

#define _kernel_osrdch() \
          pseudokern_osrdch(__FILE__, __LINE__)

#define _kernel_oswrch(ch) \
          pseudokern_oswrch(ch, __FILE__, __LINE__)

#define _kernel_osbget(handle) \
          pseudokern_osbget(handle, __FILE__, __LINE__)

#define _kernel_osbput(ch, handle) \
          pseudokern_osbput(ch, handle, __FILE__, __LINE__)

#define _kernel_osgbpb(op, handle, inout) \
          pseudokern_osgbpb(op, handle, inout, __FILE__, __LINE__)

#define _kernel_osword(op, data) \
          pseudokern_osword(op, data, __FILE__, __LINE__)

#define _kernel_osfind(op, name) \
          pseudokern_osfind(op, name, __FILE__, __LINE__)

#define _kernel_osfile(op, name, inout) \
          pseudokern_osfile(op, name, inout, __FILE__, __LINE__)

#define _kernel_osargs(op, handle, arg) \
          pseudokern_osargs(op, handle, arg, __FILE__, __LINE__)

#define _kernel_oscli(s) \
          pseudokern_oscli(s, __FILE__, __LINE__)

#define _kernel_getenv(name, buffer, size) \
          pseudokern_getenv(name, buffer, size, __FILE__, __LINE__)

#define _kernel_setenv(name, value) \
          pseudokern_setenv(name, value, __FILE__, __LINE__)

#define _kernel_system(string, chain) \
          pseudokern_system(string, chain, __FILE__, __LINE__)

#endif

_Optional _kernel_oserror *pseudokern_swi(int no, _kernel_swi_regs *in,
                                _kernel_swi_regs *out, const char *file,
                                unsigned long line);

_Optional _kernel_oserror *pseudokern_swi_c(int no, _kernel_swi_regs *in,
                                  _kernel_swi_regs *out, int *carry,
                                  const char *file, unsigned long line);

int pseudokern_osbyte(int op, int x, int y, const char *file,
                      unsigned long line);

int pseudokern_osrdch(const char *file, unsigned long line);

int pseudokern_oswrch(int ch, const char *file, unsigned long line);

int pseudokern_osbget(unsigned handle, const char *file, unsigned long line);

int pseudokern_osbput(int ch, unsigned handle, const char *file,
                      unsigned long line);

int pseudokern_osgbpb(int op, unsigned handle, _kernel_osgbpb_block *inout,
                      const char *file, unsigned long line);

int pseudokern_osword(int op, int *data, const char *file, unsigned long line);

int pseudokern_osfind(int op, char *name, const char *file,
                      unsigned long line);

int pseudokern_osfile(int op, const char *name, _kernel_osfile_block *inout,
                      const char *file, unsigned long line);

int pseudokern_osargs(int op, unsigned handle, int arg, const char *file,
                      unsigned long line);

int pseudokern_oscli(const char *s, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudokern_getenv(const char *name, char *buffer,
                                             unsigned size, const char *file,
                                             unsigned long line);

_Optional _kernel_oserror *pseudokern_setenv(const char *name, const char *value,
                                             const char *file, unsigned long line);

int pseudokern_system(const char *string, int chain, const char *file,
                      unsigned long line);

/* Internal function for use by other pseudo modules. */
_Optional _kernel_oserror *pseudokern_fail(const char *file, unsigned long line);

#endif
