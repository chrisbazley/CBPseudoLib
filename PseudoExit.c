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

/* History:
  CJB: 25-May-15: Created this source file.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#undef FORTIFY /* Prevent macro redirection of exit calls to
                  pseudo_exit within this source file. */

/* ISO library headers */
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>
#include <string.h>

/* Local headers */
#include "Debug.h"
#include "PseudoExit.h"

static jmp_buf exit_target;
static bool exit_pending;

void pseudo_exit(int status)
{
  if (exit_pending)
  {
    DEBUGF("Intercepted call to exit with %d\n", status);
    exit_pending = false;
    longjmp(exit_target, status+1);
  }
  else
  {
    DEBUGF("Allowing call to exit with %d\n", status);
    exit(status);
  }
}

void pseudo_exit_set_target(jmp_buf env)
{
  DEBUGF("Prepared to intercept call to exit\n");
  exit_pending = true;
  memcpy(exit_target, env, sizeof(exit_target));
}
