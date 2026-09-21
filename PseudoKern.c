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

/* History:
  CJB: 29-Apr-12: Created this source file.
  CJB: 27-Dec-14: Exported the pseudokern_fail function for internal use.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 13-Jun-20: Use new Fortify_AllowAllocate to avoid accumulating huge
                  numbers of 'freed' dummy memory allocations.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Cast _kernel_oserror pointer to intptr_t instead of int.
  CJB: 21-Sep-26: Declare variables when they are first assigned.
  CJB: 21-Sep-26: Declare SWI registers with an initialiser.
                  Ensure only void * is converted to intptr_t.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#undef FORTIFY /* Prevent macro redirection of _kernel_... calls to
                  pseudokern_... functions within this source file. */

/* ISO library headers */
#include <stdint.h>

/* Acorn C/C++ library headers */
#include "kernel.h"
#include "swis.h"

#include "fortify.h"

/* Local headers */
#include "PseudoKern.h"
#include "Debug.h"
#include "Internal/CBPsdoMisc.h"

_Optional _kernel_oserror *pseudokern_fail(const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  /* CJB's extra Fortify function to avoid accumulating
     huge numbers of 'freed' dummy memory allocations. */
  if (!Fortify_AllowAllocate(file, line))
  {
    /* Look up a generic out-of-memory error. Note that this also takes
       care of setting _kernel_last_oserror. */
    static const _kernel_oserror temp = {DUMMY_ERRNO, "NoMem"};
    _kernel_swi_regs regs = {
      .r = {
        (intptr_t)(void *)&temp,
        0, /* use global messages */
        0, /* use an internal buffer */
        0, /* buffer size */
      }
    };
    e = _kernel_swi(MessageTrans_ErrorLookup, &regs, &regs);
  }

  return e;
}

_Optional _kernel_oserror *pseudokern_swi(int no,
                                _kernel_swi_regs *in,
                                _kernel_swi_regs *out,
                                const char *file,
                                unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  assert(in != NULL);
  assert(out != NULL);

  /* Only calls with the _kernel_NONX bit clear can return an error
     (otherwise SIGOSERROR is raised on error) */
  if (!(_kernel_NONX & no))
    e = pseudokern_fail(file, line);

  if (e == NULL)
    e = _kernel_swi(no, in, out);

  return e;
}

_Optional _kernel_oserror *pseudokern_swi_c(int no,
                                  _kernel_swi_regs *in,
                                  _kernel_swi_regs *out,
                                  int *carry,
                                  const char *file,
                                  unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  assert(in != NULL);
  assert(out != NULL);
  assert(carry != NULL);

  /* Only calls with the _kernel_NONX bit clear can return an error
     (otherwise SIGOSERROR is raised on error) */
  if (!(_kernel_NONX & no))
    e = pseudokern_fail(file, line);

  if (e == NULL)
    e = _kernel_swi_c(no, in, out, carry);

  return e;
}

int pseudokern_osbyte(int op,
                      int x,
                      int y,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osbyte(op, x, y);

  return result;
}

int pseudokern_osrdch(const char *file, unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osrdch();

  return result;
}

int pseudokern_oswrch(int ch, const char *file, unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_oswrch(ch);

  return result;
}

int pseudokern_osbget(unsigned handle, const char *file, unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osbget(handle);

  return result;
}

int pseudokern_osbput(int ch,
                      unsigned handle,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osbput(ch, handle);

  return result;
}

int pseudokern_osgbpb(int op,
                      unsigned handle,
                      _kernel_osgbpb_block *inout,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  assert(inout != NULL);
  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osgbpb(op, handle, inout);

  return result;
}

int pseudokern_osword(int op,
                      int *data,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  assert(data != NULL);
  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osword(op, data);

  return result;
}

int pseudokern_osfind(int op,
                      char *name,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osfind(op, name);

  return result;
}

int pseudokern_osfile(int op,
                      const char *name,
                      _kernel_osfile_block *inout,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  assert(name != NULL);
  assert(inout != NULL);
  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osfile(op, name, inout);

  return result;
}

int pseudokern_osargs(int op,
                      unsigned handle,
                      int arg,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_osargs(op, handle, arg);

  return result;
}

int pseudokern_oscli(const char *s, const char *file, unsigned long line)
{
  int result = _kernel_ERROR;

  assert(s != NULL);
  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_oscli(s);

  return result;
}

_Optional _kernel_oserror *pseudokern_getenv(const char *name,
                                             char *buffer,
                                             unsigned size,
                                             const char *file,
                                             unsigned long line)
{

  assert(name != NULL);
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);
  if (e == NULL)
    e = _kernel_getenv(name, buffer, size);

  return e;
}

_Optional _kernel_oserror *pseudokern_setenv(const char *name,
                                             const char *value,
                                             const char *file,
                                             unsigned long line)
{

  assert(name != NULL);
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);
  if (e == NULL)
    e = _kernel_setenv(name, value);

  return e;
}

int pseudokern_system(const char *string,
                      int chain,
                      const char *file,
                      unsigned long line)
{
  int result = _kernel_ERROR;

  assert(string != NULL);
  if (pseudokern_fail(file, line) == NULL)
    result = _kernel_system(string, chain);

  return result;
}
