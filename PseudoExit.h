/*
 * CBPseudoLib: Unit test veneer to standard library exit function
 * Copyright (C) 2015 Christopher Bazley
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

/* PseudoExit.h declares a macro to mimic the standard library's exit
   function in order to redirect function calls to an alternative
   implementation that doesn't really terminate the program. This is useful
   for testing.

Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 25-May-15: Created.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#ifndef PseudoExit_h
#define PseudoExit_h

/* ISO library headers */
#include <stdlib.h>
#include <setjmp.h>

#ifdef FORTIFY
#define exit(status) pseudo_exit(status)
#endif

void pseudo_exit(int /*status*/);
   /*
    * If pseudo_exit_set_target has been called since the last call to
    * the pseudo_exit function then the saved program state will be
    * restored and execution resume as though the setjmp function had
    * just been called. The return value of setjmp will be one greater
    * than value of 'status' (since 0 indicates return from a direct
    * call). Otherwise, normal program termination will occur.
    */

void pseudo_exit_set_target(jmp_buf /*env*/);
   /*
    * Sets the state that will be restored when the exit function is
    * called. This state must have been saved by a preceding call to
    * the setjmp function.
    */

#endif
