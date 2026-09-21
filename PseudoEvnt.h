/*
 * CBPseudoLib: Error injection veneer to Acorn's event library
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

/* PseudoEvnt.h declares macros to mimic the API provided by Acorn's
   event library in order to redirect function calls to an alternative
   implementation that returns errors if allocations via Simon P. Bullen's
   fortified memory allocation shell fail. This allows stress testing.

Dependencies: Acorn event library, Fortify.
Message tokens: None.
History:
  CJB: 23-Dec-14: Created.
  CJB: 19-Apr-15: Added declaration of pseudo_event_wait_for_idle function.
  CJB: 27-Aug-26: Removed _Optional qualifier from the return type of
                  pseudo_event_get_client_id_block.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#ifndef PseudoEvnt_h
#define PseudoEvnt_h

/* Acorn C/C++ library headers */
#include <kernel.h>
#include <toolbox.h>
#include <event.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

/* Define macros to intercept calls to Acorn's event library and replace
 * them with calls to the equivalent 'pseudo_event' functions.
 */

#define event_initialise(block) \
          pseudo_event_initialise(block, __FILE__, __LINE__)

#define event_set_mask(mask) \
          pseudo_event_set_mask(mask, __FILE__, __LINE__)

#define event_poll(event_code, poll_block, poll_word) \
          pseudo_event_poll(event_code, poll_block, poll_word, __FILE__, __LINE__)

#define event_poll_idle(event_code, poll_block, earliest, poll_word) \
          pseudo_event_poll_idle(event_code, poll_block, earliest, poll_word, __FILE__, __LINE__)

#define event_register_toolbox_handler(object_id, event_code, handler, handle) \
          pseudo_event_register_toolbox_handler(object_id, event_code, handler, handle, __FILE__, __LINE__)

#define event_deregister_toolbox_handler(object_id, event_code, handler, handle) \
          pseudo_event_deregister_toolbox_handler(object_id, event_code, handler, handle, __FILE__, __LINE__)

#define event_deregister_toolbox_handlers_for_object(object_id) \
          pseudo_event_deregister_toolbox_handlers_for_object(object_id, __FILE__, __LINE__)

#define event_register_message_handler(msg_no, handler, handle) \
          pseudo_event_register_message_handler(msg_no, handler, handle, __FILE__, __LINE__)

#define event_deregister_message_handler(msg_no, handler, handle) \
          pseudo_event_deregister_message_handler(msg_no, handler, handle, __FILE__, __LINE__)

#define event_register_wimp_handler(object_id, event_code, handler, handle) \
          pseudo_event_register_wimp_handler(object_id, event_code, handler, handle, __FILE__, __LINE__)

#define event_deregister_wimp_handler(object_id, event_code, handler, handle) \
          pseudo_event_deregister_wimp_handler(object_id, event_code, handler, handle, __FILE__, __LINE__)

#define event_deregister_wimp_handlers_for_object(object_id) \
          pseudo_event_deregister_wimp_handlers_for_object(object_id, __FILE__, __LINE__)

#endif

_Optional _kernel_oserror *pseudo_event_initialise(IdBlock *block, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_set_mask(unsigned int mask, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_poll(_Optional int *event_code, _Optional WimpPollBlock *poll_block, _Optional void *poll_word, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_poll_idle(_Optional int *event_code, _Optional WimpPollBlock *poll_block, unsigned int earliest, _Optional void *poll_word, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_register_toolbox_handler(ObjectId object_id, int event_code, ToolboxEventHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_deregister_toolbox_handler(ObjectId object_id, int event_code, ToolboxEventHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_deregister_toolbox_handlers_for_object (int object_id, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_register_message_handler(int msg_no, WimpMessageHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_deregister_message_handler(int msg_no, WimpMessageHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_register_wimp_handler(ObjectId object_id, int event_code, WimpEventHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_deregister_wimp_handler(ObjectId object_id, int event_code, WimpEventHandler *handler, void *handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_event_deregister_wimp_handlers_for_object(int object_id, const char *file, unsigned long line);

/* The following functions are for use in unit tests */

IdBlock *pseudo_event_get_client_id_block(void);
   /*
    * Gets the Toolbox ID block registered upon initialisation.
    * Returns: a pointer to the client application's ID block.
    */

_Optional _kernel_oserror *pseudo_event_wait_for_idle(void);
   /*
    * Unmask all events and call event_poll until it returns a null event.
    * Used in tests to ensure that all events entailed by a stimulus have
    * been delivered.
    */

#endif
