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

/* History:
  CJB: 23-Dec-14: Created this source file.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 19-Apr-15: Added the pseudo_event_wait_for_idle function to poll the
                  event library until a null event is received.
                  Lots of extra debugging output, including printing of events
                  and messages received via event_poll or event_poll_idle.
  CJB: 16-Sep-15: Guard against the Toolbox ID block being unknown in
                  print_event (because event_initialise was not intercepted).
  CJB: 12-Oct-15: Use wimp_poll instead of event_poll to avoid messing around
                  with the actual event library mask in wait_for_idle.
  CJB: 05-Sep-20: Fixed a null pointer dereference in event_poll when null is
                  passed instead of an event_code address.
                  pseudo_event_wait_for_idle no longer prints or dispatches
                  the final null event it receives.
  CJB: 29-Nov-20: Fixed a null pointer dereference in event_poll_idle when
                  null is passed instead of an event_code address.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Cast _kernel_oserror pointer to intptr_t instead of int.
  CJB: 25-Aug-26: Initialize dynamically allocated handler records by
                  assigning compound literals.
  CJB: 25-Aug-26: Use CONTAINER_OF to recover handler records from their
                  linked-list items.
  CJB: 25-Aug-26: Initialize temporary objects in their declarations.
  CJB: 27-Aug-26: Removed _Optional qualifier from the return type of
                  pseudo_event_get_client_id_block.
  CJB: 21-Sep-26: Declare variables when they are first assigned.
  CJB: 21-Sep-26: Declare SWI registers with an initialiser.
                  Ensure only void * is converted to intptr_t.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
  CJB: 21-Sep-26: Abort if a handler being deregistered was not registered,
                  even when assertions are disabled.
*/

#undef FORTIFY /* Prevent macro redirection of event_... calls to
                  pseudo_event_... functions within this source file. */

/* ISO library headers */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* Acorn C/C++ library headers */
#include "kernel.h"
#include "event.h"
#include "swis.h"
#include "wimplib.h"

#include "fortify.h"

#include "LinkedList.h"

/* Local headers */
#include "Debug.h"
#include "PseudoEvnt.h"
#include "PseudoKern.h"
#include "PseudoTbox.h"
#include "Internal/CBPsdoMisc.h"

/* These lists of event handlers are currently used only to detect leaks */
static LinkedList wimp_handlers, tb_handlers, msg_handlers;
static IdBlock *client_block;

typedef struct
{
  LinkedListItem list_item;
  ObjectId object_id;
  int event_code;
  WimpEventHandler *handler;
  void *handle;
}
PseudoEvent_Wimp_Handler;

typedef struct
{
  LinkedListItem list_item;
  ObjectId object_id;
  int event_code;
  ToolboxEventHandler *handler;
  void *handle;
}
PseudoEvent_Toolbox_Handler;

typedef struct
{
  LinkedListItem list_item;
  int msg_no;
  WimpMessageHandler *handler;
  void *handle;
}
PseudoEvent_Message_Handler;

static _Optional _kernel_oserror *oom(void)
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
  return _kernel_swi(MessageTrans_ErrorLookup, &regs, &regs);
}

IdBlock *pseudo_event_get_client_id_block(void)
{
  return client_block;
}

_Optional _kernel_oserror *pseudo_event_initialise(IdBlock *block, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  if (e == NULL)
  {
    client_block = block;
    linkedlist_init(&wimp_handlers);
    linkedlist_init(&tb_handlers);
    linkedlist_init(&msg_handlers);
    e = event_initialise(block);
    if (e != NULL)
    {
      DEBUGF("event_initialise error: 0x%x %s\n", e->errnum, e->errmess);
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_event_set_mask(unsigned int mask, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  if (e == NULL)
    e = event_set_mask(mask);

  return e;
}

static void print_event(int event_code, WimpPollBlock *poll_block)
{
  const ToolboxEvent * const tb = (ToolboxEvent *)&poll_block->words;
  switch (event_code)
  {
    case Wimp_EUserMessage:
    case Wimp_EUserMessageAcknowledge:
    case Wimp_EUserMessageRecorded:
      DEBUGF("Wimp message event 0x%x action code 0x%x\n",
             event_code, poll_block->user_message.hdr.action_code);
      break;
    case Wimp_EToolboxEvent:
      DEBUGF("Toolbox event 0x%x\n", tb->hdr.event_code);
      switch (tb->hdr.event_code)
      {
        case Toolbox_ObjectAutoCreated:
          if (client_block != NULL)
            pseudo_toolbox_object_created(client_block->self_id);
          break;
        case Toolbox_ObjectDeleted:
          if (client_block != NULL)
            pseudo_toolbox_object_deleted(client_block->self_id);
          break;
      }
      break;
    default:
      DEBUGF("Wimp event 0x%x\n", event_code);
      break;
  }
}

_Optional _kernel_oserror *pseudo_event_poll(_Optional int *event_code, _Optional WimpPollBlock *poll_block, _Optional void *poll_word, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  if (e == NULL)
  {
    WimpPollBlock pb;
    if (poll_block == NULL)
    {
      poll_block = &pb;
    }
    int ec;
    if (event_code == NULL)
    {
      event_code = &ec;
    }
    e = event_poll(event_code, poll_block, poll_word);
    if (e != NULL)
    {
      DEBUGF("event_poll error: 0x%x %s\n", e->errnum, e->errmess);
    }
    else
    {
      print_event(*event_code, &*poll_block);
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_event_wait_for_idle(void)
{
  unsigned int count = 511;
  int event_code;
  _Optional _kernel_oserror *e;

  do
  {
    unsigned int mask;

    DEBUGF("Waiting for idle (count %u)\n", count);
    e = event_get_mask(&mask);
    if (e != NULL)
    {
      DEBUGF("event_get_mask error: 0x%x %s\n", e->errnum, e->errmess);
    }
    else
    {
      WimpPollBlock poll_block;
      /* The client's event mask may enable poll-word polling, in which case
         wimp_poll requires the pointed-to value to be initialized. */
      int pollword = 0;
      e = wimp_poll(mask & ~Wimp_Poll_NullMask, &poll_block, &pollword, &event_code);
      if (e != NULL)
      {
        DEBUGF("wimp_poll error: 0x%x %s\n", e->errnum, e->errmess);
      }
      else if (event_code != Wimp_ENull)
      {
        print_event(event_code, &poll_block);
        e = event_dispatch(event_code, &poll_block);
        if (e != NULL)
        {
          DEBUGF("event_dispatch error: 0x%x %s\n", e->errnum, e->errmess);
        }
      }
    }
  }
  while (e == NULL && event_code != Wimp_ENull && count-- > 0);
  return e;
}

_Optional _kernel_oserror *pseudo_event_poll_idle(_Optional int *event_code, _Optional WimpPollBlock *poll_block, unsigned int earliest, _Optional void *poll_word, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  if (e == NULL)
  {
    WimpPollBlock pb;
    if (poll_block == NULL)
    {
      poll_block = &pb;
    }
    int ec;
    if (event_code == NULL)
    {
      event_code = &ec;
    }
    e = event_poll_idle(event_code, poll_block, earliest, poll_word);
    if (e != NULL)
    {
      DEBUGF("event_poll_idle error: 0x%x %s\n", e->errnum, e->errmess);
    }
    else
    {
      print_event(*event_code, &*poll_block);
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_event_register_toolbox_handler(ObjectId object_id, int event_code, ToolboxEventHandler *handler, void *handle, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  DEBUGF("event_register_toolbox_handler called for event 0x%x on object 0x%x at %s:%lu\n", event_code, (unsigned)object_id, file, line);

  _Optional PseudoEvent_Toolbox_Handler *record = Fortify_malloc(sizeof(*record), file, line);
  if (record != NULL)
  {
    *record = (PseudoEvent_Toolbox_Handler){
      {NULL, NULL}, object_id, event_code, handler, handle};

    e = event_register_toolbox_handler(object_id, event_code, handler, handle);
    if (e == NULL)
    {
      linkedlist_insert(&tb_handlers, NULL, &record->list_item);
    }
    else
    {
      DEBUGF("event_register_toolbox_handler error: 0x%x %s\n", e->errnum, e->errmess);
      Fortify_free(record, file, line);
    }
  }
  else
  {
    e = oom();
  }

  return e;
}

static bool toolbox_handler_matches(LinkedList *list, LinkedListItem *item, void *arg)
{
  const PseudoEvent_Toolbox_Handler *const record =
    CONTAINER_OF(item, PseudoEvent_Toolbox_Handler, list_item);
  const PseudoEvent_Toolbox_Handler * const to_match = arg;

  assert(list == &tb_handlers);
  assert(record != NULL);
  assert(to_match != NULL);
  return to_match->object_id == record->object_id &&
         to_match->event_code == record->event_code &&
         to_match->handler == record->handler &&
         to_match->handle == record->handle;
}

_Optional _kernel_oserror *pseudo_event_deregister_toolbox_handler(ObjectId object_id, int event_code, ToolboxEventHandler *handler, void *handle, const char *file, unsigned long line)
{
  PseudoEvent_Toolbox_Handler to_match = {
    {NULL, NULL}, object_id, event_code, handler, handle};

  DEBUGF("event_deregister_toolbox_handler called for event 0x%x on object 0x%x at %s:%lu\n", event_code, (unsigned)object_id, file, line);

  _Optional LinkedListItem *item = linkedlist_for_each(&tb_handlers, toolbox_handler_matches, &to_match);
  assert(item != NULL);
  if (item == NULL)
    abort();
  linkedlist_remove(&tb_handlers, &*item);
  Fortify_free(CONTAINER_OF(&*item, PseudoEvent_Toolbox_Handler, list_item),
               file, line);

  return event_deregister_toolbox_handler(object_id, event_code, handler, handle);
}

static bool deregister_toolbox_handlers_for_object(LinkedList *list, LinkedListItem *item, void *arg)
{
  PseudoEvent_Toolbox_Handler *const record =
    CONTAINER_OF(item, PseudoEvent_Toolbox_Handler, list_item);
  const int * const object_id = arg;

  assert(list == &tb_handlers);
  assert(record != NULL);
  assert(object_id != NULL);
  if (*object_id == record->object_id)
  {
    linkedlist_remove(list, item);
    Fortify_free(record, __FILE__, __LINE__);
  }

  return false; /* next list item */
}

_Optional _kernel_oserror *pseudo_event_deregister_toolbox_handlers_for_object(int object_id, const char *file, unsigned long line)
{
  NOT_USED(file);
  NOT_USED(line);

  DEBUGF("event_deregister_toolbox_handlers_for_object called for object 0x%x at %s:%lu\n", (unsigned)object_id, file, line);

  linkedlist_for_each(&tb_handlers, deregister_toolbox_handlers_for_object, &object_id);

  return event_deregister_toolbox_handlers_for_object(object_id);
}

_Optional _kernel_oserror *pseudo_event_register_message_handler(int msg_no, WimpMessageHandler *handler, void *handle, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  DEBUGF("event_register_message_handler called for msg 0x%x at %s:%lu\n", msg_no, file, line);

  _Optional PseudoEvent_Message_Handler *record = Fortify_malloc(sizeof(*record), file, line);
  if (record != NULL)
  {
    *record = (PseudoEvent_Message_Handler){
      {NULL, NULL}, msg_no, handler, handle};

    e = event_register_message_handler(msg_no, handler, handle);
    if (e == NULL)
    {
      linkedlist_insert(&msg_handlers, NULL, &record->list_item);
    }
    else
    {
      DEBUGF("event_register_message_handler error: 0x%x %s\n", e->errnum, e->errmess);
      Fortify_free(record, file, line);
    }
  }
  else
  {
    e = oom();
  }

  return e;
}

static bool message_handler_matches(LinkedList *list, LinkedListItem *item, void *arg)
{
  const PseudoEvent_Message_Handler *const record =
    CONTAINER_OF(item, PseudoEvent_Message_Handler, list_item);
  const PseudoEvent_Message_Handler * const to_match = arg;

  assert(list == &msg_handlers);
  assert(record != NULL);
  assert(to_match != NULL);
  return to_match->msg_no == record->msg_no &&
         to_match->handler == record->handler &&
         to_match->handle == record->handle;
}

_Optional _kernel_oserror *pseudo_event_deregister_message_handler(int msg_no, WimpMessageHandler *handler, void *handle, const char *file, unsigned long line)
{
  PseudoEvent_Message_Handler to_match = {
    {NULL, NULL}, msg_no, handler, handle};

  DEBUGF("event_deregister_message_handler called for msg 0x%x at %s:%lu\n", msg_no, file, line);

  _Optional LinkedListItem *item = linkedlist_for_each(&msg_handlers, message_handler_matches, &to_match);
  assert(item != NULL);
  if (item == NULL)
    abort();
  linkedlist_remove(&msg_handlers, &*item);
  Fortify_free(CONTAINER_OF(&*item, PseudoEvent_Message_Handler, list_item),
               file, line);

  return event_deregister_message_handler(msg_no, handler, handle);
}

_Optional _kernel_oserror *pseudo_event_register_wimp_handler(ObjectId object_id, int event_code, WimpEventHandler *handler, void *handle, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = NULL;

  DEBUGF("event_register_wimp_handler called for event 0x%x on object 0x%x at %s:%lu\n", event_code, object_id, file, line);

  _Optional PseudoEvent_Wimp_Handler *record = Fortify_malloc(sizeof(*record), file, line);
  if (record != NULL)
  {
    *record = (PseudoEvent_Wimp_Handler){
      {NULL, NULL}, object_id, event_code, handler, handle};

    e = event_register_wimp_handler(object_id, event_code, handler, handle);
    if (e == NULL)
    {
      linkedlist_insert(&wimp_handlers, NULL, &record->list_item);
    }
    else
    {
      DEBUGF("event_register_wimp_handler error: 0x%x %s\n", e->errnum, e->errmess);
      Fortify_free(record, file, line);
    }
  }
  else
  {
    e = oom();
  }

  return e;
}

static bool wimp_handler_matches(LinkedList *list, LinkedListItem *item, void *arg)
{
  const PseudoEvent_Wimp_Handler *const record =
    CONTAINER_OF(item, PseudoEvent_Wimp_Handler, list_item);
  const PseudoEvent_Wimp_Handler * const to_match = arg;

  assert(list == &wimp_handlers);
  assert(record != NULL);
  assert(to_match != NULL);
  return to_match->object_id == record->object_id &&
         to_match->event_code == record->event_code &&
         to_match->handler == record->handler &&
         to_match->handle == record->handle;
}

_Optional _kernel_oserror *pseudo_event_deregister_wimp_handler(ObjectId object_id, int event_code, WimpEventHandler *handler, void *handle, const char *file, unsigned long line)
{
  PseudoEvent_Wimp_Handler to_match = {
    {NULL, NULL}, object_id, event_code, handler, handle};

  DEBUGF("event_deregister_wimp_handler called for event 0x%x on object 0x%x at %s:%lu\n", event_code, object_id, file, line);

  _Optional LinkedListItem *item = linkedlist_for_each(&wimp_handlers, wimp_handler_matches, &to_match);
  assert(item != NULL);
  if (item == NULL)
    abort();
  linkedlist_remove(&wimp_handlers, &*item);
  Fortify_free(CONTAINER_OF(&*item, PseudoEvent_Wimp_Handler, list_item),
               file, line);

  return event_deregister_wimp_handler(object_id, event_code, handler, handle);
}

static bool deregister_wimp_handlers_for_object(LinkedList *list, LinkedListItem *item, void *arg)
{
  PseudoEvent_Wimp_Handler *const record =
    CONTAINER_OF(item, PseudoEvent_Wimp_Handler, list_item);
  const int * const object_id = arg;

  assert(list == &wimp_handlers);
  assert(record != NULL);
  assert(object_id != NULL);
  if (*object_id == record->object_id)
  {
    linkedlist_remove(list, item);
    Fortify_free(record, __FILE__, __LINE__);
  }

  return false; /* next list item */
}

_Optional _kernel_oserror *pseudo_event_deregister_wimp_handlers_for_object(int object_id, const char *file, unsigned long line)
{
  NOT_USED(file);
  NOT_USED(line);

  DEBUGF("event_deregister_wimp_handlers_for_object called for object 0x%x at %s:%lu\n", object_id, file, line);

  linkedlist_for_each(&wimp_handlers, deregister_wimp_handlers_for_object, &object_id);

  return event_deregister_wimp_handlers_for_object(object_id);
}
