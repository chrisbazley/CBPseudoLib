/*
 * CBPseudoLib: Private miscellaneous definitions
 * Copyright (C) 2018 Christopher Bazley
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
  CJB: 21-Sep-26: Extract the pseudo-interface definitions from CBDebugLib and
                  use CBUtilLib for generic macros.
*/

#ifndef CBPsdoMisc_h
#define CBPsdoMisc_h

#include "MacroUtils.h"

/* I believe that using error number 0 can have unpleasant side-effects. */
enum
{
  DUMMY_ERRNO = 255
};

#endif /* CBPsdoMisc_h */
