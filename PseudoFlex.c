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

/* History:
  CJB: 20-Jan-08: Created this source file according to the function
                  descriptions in the flex library header.
  CJB: 27-Jan-08: Now passes the file name and line number of the caller
                  straight through to Fortify (hence it now depends directly
                  upon Fortify functions).
  CJB: 14-Feb-08: Corrected an overzealous assertion 'at < size' in the
                  PseudoFlex_midextend function (prevented extension at end).
                  Changed same function to use memcpy instead of memmove when
                  truncating (new block can't overlap old).
  CJB: 27-Aug-08: Minor modification so that PseudoFlex_alloc always leaves
                  the anchor unchanged when returning zero.
  CJB: 09-Sep-09: Stop using reserved identifier '_PseudoFlexRecord' (starts
                  with an underscore followed by a capital letter).
  CJB: 15-Sep-09: Amended assertions to allow blocks of size 0 to be created
                  or blocks to be truncated to 0. PseudoFlex_init checks that
                  Fortify was compiled with the appropriate user options.
  CJB: 17-Oct-09: Fixed overzealous assertion in Pseudoflex_midextend function
                  to allow removal of the start of a block.
  CJB: 17-Dec-14: Updated to use the generic linked list implementation.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 21-Apr-16: Modified format strings to avoid GNU C compiler warnings.
  CJB: 10-Feb-19: Fixed a bug in PseudoFlex_midextend(), which called free()
                  instead of Fortify_free(). This made it unusable with
                  negative 'by' values if compiled without FORTIFY defined.
  CJB: 05-Sep-20: Reduced the volume of debugging output.
                  More local const declarations with initializers.
  CJB: 03-Apr-21: More data in debugging output.
  CJB: 17-Jun-23: Annotated unused variables to suppress warnings when
                  debug output is disabled at compile time.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Initialize dynamically allocated records by assigning
                  compound literals.
  CJB: 25-Aug-26: Use CONTAINER_OF to recover flex records from their
                  linked-list items.
  CJB: 26-Aug-26: Avoid dereferencing void *.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.

*/

/* ISO library headers */
#include <stdlib.h>
#include <stdio.h>

/* Acorn C/C++ library headers */
#include "flex.h"

#include "fortify.h"

#include "LinkedList.h"

/* Local headers */
#include "PseudoFlex.h"
#include "Debug.h"
#include "Internal/CBPsdoMisc.h"

/* -----------------------------------------------------------------------
                          Internal library data
*/

/* The following structure stores information about a flex block */
typedef struct PseudoFlexRecord
{
  LinkedListItem           list_item;
  int                      size; /* the current size of this block, in bytes */
  flex_ptr                 anchor; /* pointer to anchor of the heap block */
}
PseudoFlexRecord;

static int defer_compact = 0; /* compact on frees */
static int budge_state = 0; /* refuse to budge */
static LinkedList block_list;

/* ----------------------------------------------------------------------- */
/*                       Function prototypes                               */

static _Optional PseudoFlexRecord *find_anchor(flex_ptr anchor);

/* -----------------------------------------------------------------------
                         Public library functions
*/

int PseudoFlex_alloc(flex_ptr anchor, int n, const char *file, unsigned long line)
{
  assert(anchor != NULL);
  assert(n >= 0);

  /* Allocate memory for a private record of a new pseudo-flex block */
  _Optional PseudoFlexRecord *const pfr = malloc(sizeof(*pfr));
  if (pfr == NULL)
  {
    DEBUG("PseudoFlex: Memory allocation failed! (1)");
    return 0; /* failure */
  }

  /* Allocate a heap block of the requested size, and store the returned
     pointer in the specified 'flex anchor'. It is possible to allocate a
     flex block of 0 bytes and therefore Fortify must have been compiled
     without FORTIFY_FAIL_ON_ZERO_MALLOC. */
  _Optional char *const blk = Fortify_malloc(n, file, line);
  if (blk == NULL)
  {
    free(pfr);
    DEBUG("PseudoFlex: Memory allocation failed! (2)");
    return 0; /* failure */
  }

  /* Store the address of the anchor and the size of the block. */
  *pfr = (PseudoFlexRecord){{NULL, NULL}, n, anchor};

  /* Link our record of the new block at the head of a double-linked list */
  linkedlist_insert(&block_list, NULL, &pfr->list_item);

  /* Store the address of the heap block in the caller's anchor */
  *anchor = &*blk;
  DEBUG("PseudoFlex: Allocated block %p of %d bytes anchored at %p",
    *anchor, n, (void *)anchor);

  return 1; /* success */
}

/* ----------------------------------------------------------------------- */

void PseudoFlex_free(flex_ptr anchor, const char *file, unsigned long line)
{
  assert(anchor != NULL);
  DEBUG("PseudoFlex: Free block %p anchored at %p", *anchor, (void *)anchor);

  /* Search our linked list of allocated block records for one which describes
     the specified flex anchor */
  _Optional PseudoFlexRecord *const pfr = find_anchor(anchor);
  assert(pfr != NULL);
  if (pfr != NULL)
  {
    /* Remove our record of the heap block from our double-linked list */
    linkedlist_remove(&block_list, &pfr->list_item);

    /* Destroy our record of the heap block */
    free(pfr);

    /* Free the actual heap block */
    Fortify_free(*anchor, file, line);
    *anchor = 0;
  }
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_size(flex_ptr anchor)
{
  assert(anchor != NULL);
  DEBUG_VERBOSE("PseudoFlex: Get size of block %p anchored at %p", *anchor, (void *)anchor);

  /* Search our linked list of allocated block records for one which describes
     the specified flex anchor */
  _Optional PseudoFlexRecord *const pfr = find_anchor(anchor);
  assert(pfr != NULL);
  if (pfr == NULL)
    return 0; /* size unknown (bad flex anchor) */

  /* Return the size of the block, in bytes. There is no equivalent ANSI
     function to do this for a heap block (hence we have to store the size of
     each one separately). */
  DEBUG_VERBOSE("PseudoFlex: Block %p anchored at %p has size %d",
        *anchor, (void *)anchor, pfr->size);
  return pfr->size;
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_extend(flex_ptr anchor, int newsize, const char *file, unsigned long line)
{
  assert(anchor != NULL);
  assert(newsize >= 0);

  /* Search our linked list of allocated block records for one which describes
     the specified flex anchor */
  _Optional PseudoFlexRecord *const pfr = find_anchor(anchor);
  assert(pfr != NULL);
  if (pfr != NULL)
  {
    /* Attempt to resize the heap block. It is possible to truncate a flex
       block to 0 bytes and therefore Fortify must have been compiled without
       FORTIFY_FAIL_ON_ZERO_MALLOC. */
    _Optional char *const new_addr = Fortify_realloc(*anchor, newsize, file, line);
    if (new_addr != NULL)
    {
      /* Update the anchor to point at the resized heap block */
      DEBUG("PseudoFlex: Resized block %p anchored at %p to %d bytes, new address %p",
            *anchor, (void *)anchor, newsize, (void *)new_addr);
      *anchor = &*new_addr;

      /* Update our record of the current block size */
      pfr->size = newsize;

      return 1; /* success */
    }
    DEBUG("PseudoFlex: Failed to resize heap block!");
  }
  return 0; /* failure */
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_midextend(flex_ptr anchor, int at, int by, const char *file, unsigned long line)
{
  assert(anchor != NULL);
  assert(at >= 0);

  /* Search our linked list of allocated block records for one which describes
     the specified flex anchor */
  _Optional PseudoFlexRecord *const pfr = find_anchor(anchor);
  assert(pfr != NULL);
  if (pfr != NULL)
  {
    _Optional char *new_addr;
    int size = pfr->size,
        newsize = size + by;
    size_t bytes_to_copy = size - at;

    DEBUG_VERBOSE("PseudoFlex: Current size of block is %d, target size is %d",
          size, newsize);

    assert(at <= size);

    if (by < 0)
    {
      assert(-by <= at); /* can't truncate beyond start of block */
      if (-by > at)
      {
        DEBUG("PseudoFlex: Can't truncate beyond start of block!");
        return 0; /* failure */
      }

      /* We can't use realloc to truncate the block because we don't want to
         lose the data at the top prematurely */
      new_addr = Fortify_malloc(newsize, file, line);
      if (new_addr == NULL)
      {
        DEBUG("PseudoFlex: Failed to allocate replacement heap block!");
        return 0; /* failure */
      }
      DEBUG_VERBOSE("PseudoFlex: New address of heap block is %p", new_addr);

      /* Replicate the data below the truncation point, in the new block */
      DEBUG_VERBOSE("PseudoFlex: Copying %d bytes from %p to %p",
            at + by, *anchor, new_addr);
      memcpy(&*new_addr, *anchor, at + by);

      /* Copy data above the truncation point downwards */
      DEBUG_VERBOSE("PseudoFlex: Copying %zu bytes from %p to %p", bytes_to_copy,
            (char *)*anchor + at, new_addr + at + by);
      memcpy(new_addr + at + by, (char *)*anchor + at, bytes_to_copy);

      /* Free the old block */
      Fortify_free(*anchor, file, line);
    }
    else
    {
      /* We can only use realloc when extending the block */
      new_addr = Fortify_realloc(*anchor, newsize, file, line);
      if (new_addr == NULL)
      {
        DEBUG("PseudoFlex: Failed to resize heap block!");
        return 0; /* failure */
      }
      DEBUG_VERBOSE("PseudoFlex: New address of heap block is %p", new_addr);

      /* Copy data above the extension point upwards */
      DEBUG_VERBOSE("PseudoFlex: Moving %zu bytes from %p to %p",
            bytes_to_copy, new_addr + at, new_addr + at + by);
      memmove(new_addr + at + by, new_addr + at, bytes_to_copy);
    }

    DEBUG("PseudoFlex: Extended/truncated block %p anchored at %p, "
          "by %d bytes at offset %d, new address %p", *anchor, (void *)anchor,
          by, at, new_addr);

    /* Update the anchor to point at the resized heap block */
    *anchor = &*new_addr;

    /* Update our record of the current block size */
    pfr->size = newsize;

    return 1; /* success */
  }
  return 0; /* failure */
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_reanchor(flex_ptr to, flex_ptr from)
{
  assert(from != NULL);
  assert(to != NULL);

  /* Search our linked list of allocated block records for one which describes
     the specified flex anchor */
  _Optional PseudoFlexRecord *const pfr = find_anchor(from);
  assert(pfr != NULL);
  if (pfr != NULL) {
    /* Store the address of the new anchor for the flex block, so that we will
       be able to find our record again using only the new anchor. */
    pfr->anchor = to;

    DEBUG("PseudoFlex: Reanchored block %p from %p to %p", *from,
          (void *)from, (void *)to);

    *to = *from; /* copy the heap block pointer from old anchor to new */
    *from = 0; /* prevent reuse of old anchor */

    return 1; /* success */
  } else {
    return 0; /* failure */
  }
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_set_budge(int newstate)
{
  int oldstate = budge_state;

  DEBUG("PseudoFlex: Budge state from %d to %d", oldstate, newstate);
  assert(newstate >= -1 && newstate <= 1);

  if (newstate != -1)
    budge_state = newstate;

  return oldstate;
}

/* ----------------------------------------------------------------------- */

void PseudoFlex_init(char *program_name, _Optional int *error_fd, int dynamic_size)
{
  DEBUG("PseudoFlex: Initialised with program name '%s', messages file %p, "
        "and DA limit %d", program_name, (void *)error_fd, dynamic_size);
  NOT_USED(program_name);
  NOT_USED(error_fd);
  NOT_USED(dynamic_size);

  /* Check that Fortify was compiled without FORTIFY_FAIL_ON_ZERO_MALLOC */
  int const percent = Fortify_SetAllocateFailRate(0);
  _Optional void *const test = Fortify_malloc(0, __FILE__, __LINE__);
  assert(test != NULL);
  Fortify_free(test, __FILE__, __LINE__);
  (void)Fortify_SetAllocateFailRate(percent);
}

/* ----------------------------------------------------------------------- */

void PseudoFlex_save_heap_info(char *filename)
{
  DEBUG("PseudoFlex: Append heap info to file '%s'", filename);
  assert(filename != NULL);

  _Optional FILE *const f = fopen(filename, "a");
  if (f != NULL)
  {
    fputs("PseudoFlex does not support flex_save_heap_info", &*f);
    fclose(&*f);
  }
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_compact(void)
{
  DEBUG("PseudoFlex: Compact heap");
  return 0; /* compaction complete */
}

/* ----------------------------------------------------------------------- */

int PseudoFlex_set_deferred_compaction(int newstate)
{
  int oldstate = defer_compact;

  DEBUG("PseudoFlex: Changing deferred compaction state from %d to %d",
        oldstate, newstate);

  assert(newstate == 0 || newstate == 1);
  defer_compact = newstate;
  return oldstate;
}

/* ----------------------------------------------------------------------- */
/*                         Private functions                               */

static bool block_has_anchor(LinkedList *list, LinkedListItem *item, void *arg)
{
  const flex_ptr anchor = arg;
  const PseudoFlexRecord *const pfr =
    CONTAINER_OF(item, PseudoFlexRecord, list_item);

  assert(pfr != NULL);
  assert(anchor != NULL);
  NOT_USED(list);
  return (pfr->anchor == anchor);
}

/* ----------------------------------------------------------------------- */

static _Optional PseudoFlexRecord *find_anchor(flex_ptr anchor)
{
  _Optional LinkedListItem *const item =
      linkedlist_for_each(&block_list, block_has_anchor, anchor);

  if (item == NULL)
  {
    DEBUG("PseudoFlex: Anchor %p not found!", (void *)anchor);
    return NULL;
  }

  PseudoFlexRecord *const pfr =
      CONTAINER_OF(&*item, PseudoFlexRecord, list_item);
  DEBUG_VERBOSE("PseudoFlex: Anchor %p found in record %p",
                (void *)anchor, (void *)pfr);

  return pfr;
}
