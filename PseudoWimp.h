/*
 * CBPseudoLib: Error injection veneer to Acorn's Wimp library
 * Copyright (C) 2014 Christopher Bazley
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

/* PseudoWimp.h declares macros to mimic the API provided by Acorn's
   Wimp library in order to redirect function calls to an alternative
   implementation that returns errors if allocations via Simon P. Bullen's
   fortified memory allocation shell fail. This allows stress testing.

Dependencies: Acorn Wimp library, Fortify.
Message tokens: None.
History:
  CJB: 23-Dec-14: Created.
  CJB: 26-Jul-15: Added a new function pseudo_wimp_get_message2 that gives
                  unit tests access to more parameters of wimp_send_message.
  CJB: 07-Dec-15: Added a new function pseudo_wimp_set_pointer_info that
                  lets unit tests specify the information returned by
                  subsequent calls to wimp_get_pointer_info.
  CJB: 31-Jan-16: Intercepted wimp_set_colour to allow error simulation.
  CJB: 02-Aug-26: Explicitly allow output arguments of pseudo_wimp_get_message2
                  to be null.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.

*/

#ifndef PseudoWimp_h
#define PseudoWimp_h

/* Acorn C/C++ library headers */
#include <kernel.h>
#include <wimplib.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

/* Define macros to intercept calls to Acorn's Wimp library and replace
 * them with calls to the equivalent 'pseudo_wimp' functions.
 */

#define wimp_read_sys_info(reason, results) \
          pseudo_wimp_read_sys_info(reason, results, __FILE__, __LINE__)

#define wimp_get_window_state(state) \
          pseudo_wimp_get_window_state(state, __FILE__, __LINE__)

#define wimp_get_caret_position(block) \
          pseudo_wimp_get_caret_position(block, __FILE__, __LINE__)

#define wimp_send_message(code, block, handle, icon, th) \
          pseudo_wimp_send_message(code, block, handle, icon, th, __FILE__, __LINE__)

#define wimp_get_pointer_info(block) \
          pseudo_wimp_get_pointer_info(block, __FILE__, __LINE__)

#define wimp_transfer_block(sh, sbuf, dh, dbuf, size) \
          pseudo_wimp_transfer_block(sh, sbuf, dh, dbuf, size, __FILE__, __LINE__)

#define wimp_drag_box(block) \
          pseudo_wimp_drag_box(block, __FILE__, __LINE__)

#define wimp_redraw_window(block, more) \
          pseudo_wimp_redraw_window(block, more, __FILE__, __LINE__)

#define wimp_get_rectangle(block, more) \
          pseudo_wimp_get_rectangle(block, more, __FILE__, __LINE__)

#define wimp_set_colour(colour) \
          pseudo_wimp_set_colour(colour, __FILE__, __LINE__)

#endif

_Optional _kernel_oserror *pseudo_wimp_read_sys_info(int reason, WimpSysInfo *results, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_get_window_state(WimpGetWindowStateBlock *state, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_get_caret_position(WimpGetCaretPositionBlock *block, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_send_message(int code, void *block, int handle, int icon, _Optional int *th, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_get_pointer_info(WimpGetPointerInfoBlock *block, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_transfer_block(int sh, void *sbuf, int dh, void *dbuf, int size, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_drag_box(_Optional WimpDragBox *block, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_redraw_window(WimpRedrawWindowBlock *block, int *more, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_get_rectangle(WimpRedrawWindowBlock *block, int *more, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_wimp_set_colour(int colour, const char *file, unsigned long line);

/* The following functions are for use in unit tests */

void pseudo_wimp_reset(void);
   /*
    * Resets the outgoing Wimp message counter and enables interception of
    * all messages subsequently sent via wimp_send_message. Also disables
    * block data transfers between tasks by subsequent calls to
    * wimp_transfer_block (all addresses are assumed to lie within the
    * current task).
    */

unsigned int pseudo_wimp_get_message_count(void);
   /*
    * Gets the number of outgoing Wimp messages intercepted by the
    * alternative implementation of wimp_send_message.
    * Returns: the number of messages sent.
    */

void pseudo_wimp_get_message(unsigned int index, WimpMessage *msg);
   /*
    * Gets an outgoing Wimp message previously intercepted by the alternative
    * implementation of wimp_send_message. The given message 'index' must be
    * less than the count returned by pseudo_wimp_get_message_count.
    */


void pseudo_wimp_get_message2(unsigned int index,
                              _Optional int *code,
                              _Optional WimpPollBlock *block,
                              _Optional int *handle,
                              _Optional int *icon);
   /*
    * Gets an outgoing Wimp message previously intercepted by the alternative
    * implementation of wimp_send_message. The given message 'index' must be
    * less than the count returned by pseudo_wimp_get_message_count. The
    * reason code is stored at 'code', the event data at 'block', the window
    * handle at 'handle' and the icon handle at 'icon'. Any of the output
    * parameters can be null.
    */

void pseudo_wimp_set_pointer_info(const WimpGetPointerInfoBlock *block);
   /*
    * Sets the mouse pointer state to be returned by all subsequent calls to
    * the wimp_get_pointer_info function. The state pointed to by 'block'
    * is copied by this function.
    */

#endif
