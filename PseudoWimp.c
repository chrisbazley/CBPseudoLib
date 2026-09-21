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

/* History:
  CJB: 23-Dec-14: Created this source file.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 26-Jul-15: Added a new function pseudo_wimp_get_message2 that gives
                  unit tests access to more parameters of wimp_send_message.
                  Outgoing Wimp messages are no longer recorded until
                  pseudo_wimp_reset has been called. All messages sent
                  after pseudo_wimp_reset has been called are now intercepted
                  instead of merely being recorded.
                  The redraw protocol is now completed before returning a
                  simulated error from the wimp_redraw_window or
                  wimp_get_rectangle function.
  CJB: 07-Dec-15: Added a new function pseudo_wimp_set_pointer_info that
                  lets unit tests specify the information returned by
                  subsequent calls to wimp_get_pointer_info.
  CJB: 01-Jan-16: Fixed the pseudo_wimp_send_message function when called
                  after a pseudo_wimp_reset. It previously attempted to fill
                  in the sender's task handle and message reference even if
                  not being used to send a user message, thereby corrupting
                  the second and third words of the message data.
                  Modified the pseudo_wimp_transfer_block function so that
                  it simply copies data from the source to the destination
                  address, ignoring task handles, after pseudo_wimp_reset.
  CJB: 31-Jan-16: Intercepted wimp_set_colour to allow error simulation.
  CJB: 09-Apr-16: Added missing curly braces and initializers to a
                  WimpMessage declaration to avoid GNU C compiler warnings.
  CJB: 21-Apr-16: Substituted type size_t for the count of messages caught.
  CJB: 30-Oct-21: Handle negative or zero values passed to wimp_drag_box
                  (for cancellation). More debugging output for drags.
  CJB: 17-Jun-23: Annotated unused variables to suppress warnings when
                  debug output is disabled at compile time.
  CJB: 02-Aug-26: Explicitly allow output arguments of pseudo_wimp_get_message2
                  to be null.
  CJB: 24-Aug-26: Fix broken checks for 'more' in pseudo_wimp_redraw_window
                  and pseudo_wimp_get_rectangle (which would always have
                  continued the loop), and assert that the pointer argument
                  is not null.
                  Assert that null is not passed to pseudo_wimp_read_sys_info.
                  Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Cast WimpDragBox pointer to intptr_t instead of int.
                  Change the type of msg_count to match
                  pseudo_wimp_get_message_count.
  CJB: 25-Aug-26: Initialize temporary redraw blocks in their declarations.
  CJB: 21-Sep-26: Ensure only void * is converted to intptr_t.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#undef FORTIFY /* Prevent macro redirection of wimp_... calls to
                  pseudo_wimp_... functions within this source file. */

/* ISO library headers */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Acorn C/C++ library headers */
#include "kernel.h"
#include "wimplib.h"

/* Local headers */
#include "Debug.h"
#include "PseudoWimp.h"
#include "PseudoKern.h"
#include "Internal/CBPsdoMisc.h"

enum
{
   TaskHandleAndVersion = 5
};

static bool capture;
static unsigned int msg_count;
static struct
{
  int code;
  WimpPollBlock block;
  int handle;
  int icon;
}
msgs[32];

_Optional _kernel_oserror *pseudo_wimp_read_sys_info(int reason, WimpSysInfo *results, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(results);
  if (e == NULL)
    e = wimp_read_sys_info(reason, results);

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_get_window_state(WimpGetWindowStateBlock *state, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(state);
  if (e == NULL)
    e = wimp_get_window_state(state);

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_get_caret_position(WimpGetCaretPositionBlock *block, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(block);
  if (e == NULL)
    e = wimp_get_caret_position(block);

  return e;
}

void pseudo_wimp_reset(void)
{
  DEBUGF("Resetting Wimp message trap\n");
  msg_count = 0;
  capture = true;
}

unsigned int pseudo_wimp_get_message_count(void)
{
  DEBUGF("%u Wimp messages have been recorded\n", msg_count);
  return msg_count;
}

void pseudo_wimp_get_message(unsigned int index, WimpMessage *msg)
{
  assert(index < msg_count);

  DEBUGF("Wimp message %u of %u {0x%x,0x%x,0x%x,0x%x,0x%x,0x%x} queried\n",
         index+1, msg_count,
         msgs[index].block.words[0],
         msgs[index].block.words[1],
         msgs[index].block.words[2],
         msgs[index].block.words[3],
         msgs[index].block.words[4],
         msgs[index].block.words[5]);

  assert(msg);
  *msg = msgs[index].block.user_message;
}

void pseudo_wimp_get_message2(unsigned int index, _Optional int *code,
                              _Optional WimpPollBlock *block,
                              _Optional int *handle, _Optional int *icon)
{
  assert(index < msg_count);

  DEBUGF("Wimp message %u of %u {0x%x,0x%x,0x%x,0x%x,0x%x,0x%x} queried\n",
         index+1, msg_count,
         msgs[index].block.words[0],
         msgs[index].block.words[1],
         msgs[index].block.words[2],
         msgs[index].block.words[3],
         msgs[index].block.words[4],
         msgs[index].block.words[5]);

  if (code)
    *code = msgs[index].code;
  if (block)
    *block = msgs[index].block;
  if (handle)
    *handle = msgs[index].handle;
  if (icon)
    *icon = msgs[index].icon;
}

_Optional _kernel_oserror *pseudo_wimp_send_message(int code, void *block, int handle, int icon, _Optional int *th, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(block != NULL);
  /* th can be NULL */
  if (e == NULL)
  {
    int action_code = 0;
    if (code == Wimp_EUserMessage || code == Wimp_EUserMessageRecorded ||
        code == Wimp_EUserMessageAcknowledge)
    {
      action_code = ((WimpMessage *)block)->hdr.action_code;
    }

    DEBUGF("Task sends Wimp message code %d (action %d) to handle %d, icon %d at %s:%lu\n",
           code, action_code, handle, icon, file, line);
    NOT_USED(action_code);
    if (capture)
    {
      DEBUGF("Wimp message captured\n");

      if (code == Wimp_EUserMessage || code == Wimp_EUserMessageRecorded)
      {
        static int my_ref;
        WimpSysInfo info;
        e =  wimp_read_sys_info(TaskHandleAndVersion, &info);
        assert(e == NULL);
        ((WimpMessage *)block)->hdr.sender = (int)info.r0;
        ((WimpMessage *)block)->hdr.my_ref = ++my_ref;
      }

      if (th && handle)
      {
        /* Find the task handle of the given window/icon.
           No message will actually be sent. */
        WimpMessage query =
        {
          .hdr = {
            offsetof(WimpMessage, data),
            0, /* unused */
            0, /* unused */
            0,
            Wimp_MPaletteChange /* action_code */
          },
          .data = {
            .words = { 0 }
          }
        };
        e = wimp_send_message(Wimp_EUserMessageAcknowledge, &query, handle, icon, th);
        assert(e == NULL);
        DEBUGF("Wimp message sent to find task handle %d\n", *th);
      }

      if (msg_count < ARRAY_SIZE(msgs))
      {
        msgs[msg_count].code = code;
        msgs[msg_count].block = *(const WimpPollBlock *)block;
        msgs[msg_count].handle = handle;
        msgs[msg_count].icon = icon;
        DEBUGF("Wimp message %u {0x%x,0x%x,0x%x,0x%x,0x%x,0x%x} recorded\n",
               msg_count+1,
               msgs[msg_count].block.words[0],
               msgs[msg_count].block.words[1],
               msgs[msg_count].block.words[2],
               msgs[msg_count].block.words[3],
               msgs[msg_count].block.words[4],
               msgs[msg_count].block.words[5]);
        msg_count++;
      }
    }
    else
    {
      e = wimp_send_message(code, block, handle, icon, th);
    }
  }

  return e;
}

static bool pointer_info_is_fake = false;
static WimpGetPointerInfoBlock fake_pointer_info;

void pseudo_wimp_set_pointer_info(const WimpGetPointerInfoBlock *block)
{
  pointer_info_is_fake = true;
  fake_pointer_info = *block;
}

_Optional _kernel_oserror *pseudo_wimp_get_pointer_info(WimpGetPointerInfoBlock *block, const char *file, unsigned long line)
{
  DEBUGF("wimp_get_pointer_info called at %s:%lu\n", file, line);
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(block);
  if (e == NULL)
  {
    if (pointer_info_is_fake)
      *block = fake_pointer_info;
    else
      e = wimp_get_pointer_info(block);
  }

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_transfer_block(int sh, void *sbuf, int dh, void *dbuf, int size, const char *file, unsigned long line)
{
  DEBUGF("wimp_transfer_block called at %s:%lu\n", file, line);
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(sbuf);
  assert(dbuf);
  if (e == NULL)
  {
    if (capture)
    {
      memcpy(dbuf, sbuf, size);
    }
    else
    {
      e = wimp_transfer_block(sh, sbuf, dh, dbuf, size);
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_drag_box(_Optional WimpDragBox *block, const char *file, unsigned long line)
{
  DEBUGF("wimp_drag_box called at %s:%lu with %p\n", file, line, (void *)block);
  if (block && (intptr_t)(void *)block != -1) {
    DEBUGF("wimp_window: %d drag_type: %d\n"
           "dragging_box: {%d,%d,%d,%d}\n"
           "parent_box: {%d,%d,%d,%d}\n",
           block->wimp_window, block->drag_type,
           block->dragging_box.xmin, block->dragging_box.ymin, block->dragging_box.xmax, block->dragging_box.ymax,
           block->parent_box.xmin, block->parent_box.ymin, block->parent_box.xmax, block->parent_box.ymax);
  }
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  if (e == NULL)
    e = wimp_drag_box(block);

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_redraw_window(WimpRedrawWindowBlock *block, int *more, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(block);
  assert(more);
  if (e == NULL)
  {
    e = wimp_redraw_window(block, more);
  }
  else
  {
    /* We actually do need to handle redraw, or the window manager gets pissed off. */
    WimpRedrawWindowBlock b = {.window_handle = block->window_handle};
    _Optional _kernel_oserror *e2;

    for (e2 = wimp_redraw_window(&b, more);
         e2 == NULL && *more;
         e2 = wimp_get_rectangle(&b, more))
    {
      /* Do nothing */
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_get_rectangle(WimpRedrawWindowBlock *block, int *more, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);

  assert(block);
  assert(more);
  if (e == NULL)
  {
    e = wimp_get_rectangle(block, more);
  }
  else
  {
    /* We actually do need to handle redraw, or the window manager gets pissed off. */
    WimpRedrawWindowBlock b = {.window_handle = block->window_handle};
    _Optional _kernel_oserror *e2;

    for (e2 = wimp_get_rectangle(&b, more);
         e2 == NULL && *more;
         e2 = wimp_get_rectangle(&b, more))
    {
      /* Do nothing */
    }
  }

  return e;
}

_Optional _kernel_oserror *pseudo_wimp_set_colour(int colour, const char *file, unsigned long line)
{
  _Optional _kernel_oserror *e = pseudokern_fail(file, line);
  if (e == NULL)
  {
    e = wimp_set_colour(colour);
  }
  return e;
}
