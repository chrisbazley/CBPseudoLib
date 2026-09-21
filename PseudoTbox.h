/*
 * CBPseudoLib: Error injection veneer to Acorn's toolbox library
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

/* PseudoTbox.h declares macros to mimic the API provided by Acorn's
   toolbox library in order to redirect function calls to an alternative
   implementation that returns errors if allocations via Simon P. Bullen's
   fortified memory allocation shell fail. This allows stress testing.

Dependencies: Acorn toolbox library, Fortify.
Message tokens: None.
History:
  CJB: 23-Dec-14: Created.
  CJB: 10-Jan-15: Removed spurious argument to macro optionbutton_get_state.
  CJB: 19-Apr-15: Now intercepts extra functions: iconbar_get_icon_handle,
                  window_force_redraw, menu_set_fade and menu_get_fade.
                  Added hooks for monitoring Toolbox_ObjectAutoCreated and
                  Toolbox_ObjectDeleted events.
                  Added one function to query whether an object has been shown
                  by toolbox_show_object and another to reset the mechanism.
  CJB: 07-Dec-15: The window_set_extent function is now intercepted too.
  CJB: 07-Mar-16: The gadget_get_bbox function is now intercepted too.
  CJB: 10-Oct-21: Added #include commands to ensure that the original header
                  has already been included before defining a macro version
                  of a function (since it's impossible to do so afterwards).
  CJB: 27-Nov-21: Added interception of the gadget_set_focus function.
  CJB: 19-Aug-22: Can now be used as a proxy for textarea.h.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 21-Sep-26: Move this interface from CBDebugLib to CBPseudoLib.
*/

#ifndef PseudoTbox_h
#define PseudoTbox_h

#include <stdbool.h>

/* Acorn C/C++ library headers */
#include <kernel.h>
#include <toolbox.h>
#include <menu.h>
#include <window.h>
#include <saveas.h>
#include <gadgets.h>
#include <fileinfo.h>
#include <proginfo.h>
#include <scale.h>
#include <printdbox.h>
#include <colourdbox.h>
#include <fontdbox.h>
#include <dcs.h>
#include <quit.h>
#include <iconbar.h>
#include <textarea.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

/* Define macros to intercept calls to Acorn's toolbox C library and replace
 * them with calls to the equivalent 'pseudo_toolbox' functions.
 */

#define toolbox_initialise(flags, wimp_version, wimp_messages, toolbox_events, directory, mfd, idb, current_wimp_version, task, sprite_area) \
          pseudo_toolbox_initialise(flags, wimp_version, wimp_messages, toolbox_events, directory, mfd, idb, current_wimp_version, task, sprite_area, __FILE__, __LINE__)

#define toolbox_create_object(flags, name_or_template, id) \
          pseudo_toolbox_create_object(flags, name_or_template, id, __FILE__, __LINE__)

#define toolbox_delete_object(flags, id) \
          pseudo_toolbox_delete_object(flags, id, __FILE__, __LINE__)

#define toolbox_show_object(flags, id, show_type, type, parent, parent_component) \
          pseudo_toolbox_show_object(flags, id, show_type, type, parent, parent_component, __FILE__, __LINE__)

#define toolbox_hide_object(flags, id) \
          pseudo_toolbox_hide_object(flags, id, __FILE__, __LINE__)

#define toolbox_set_client_handle(flags, id, client_handle) \
          pseudo_toolbox_set_client_handle(flags, id, client_handle, __FILE__, __LINE__)

#define toolbox_get_client_handle(flags, id, client_handle) \
          pseudo_toolbox_get_client_handle(flags, id, client_handle, __FILE__, __LINE__)

#define toolbox_get_object_class(flags, id, object_class) \
          pseudo_toolbox_get_object_class(flags, id, object_class, __FILE__, __LINE__)

#define toolbox_get_object_state(flags, id, state) \
          pseudo_toolbox_get_object_state(flags, id, state, __FILE__, __LINE__)

#define iconbar_get_icon_handle(flags, iconbar, icon_handle) \
          pseudo_iconbar_get_icon_handle(flags, iconbar, icon_handle, __FILE__, __LINE__)

#define saveas_set_file_name(flags, saveas, file_name) \
          pseudo_saveas_set_file_name(flags, saveas, file_name, __FILE__, __LINE__)

#define saveas_set_file_type(flags, saveas, file_type) \
          pseudo_saveas_set_file_type(flags, saveas, file_type, __FILE__, __LINE__)

#define saveas_get_file_type(flags, saveas, file_type) \
          pseudo_saveas_get_file_type(flags, saveas, file_type, __FILE__, __LINE__)

#define saveas_set_file_size(flags, saveas, file_size) \
          pseudo_saveas_set_file_size(flags, saveas, file_size, __FILE__, __LINE__)

#define saveas_buffer_filled(flags, saveas, buffer, bytes_written) \
          pseudo_saveas_buffer_filled(flags, saveas, buffer, bytes_written, __FILE__, __LINE__)

#define saveas_file_save_completed(flags, saveas, filename) \
          pseudo_saveas_file_save_completed(flags, saveas, filename, __FILE__, __LINE__)

#define saveas_get_window_id(flags, saveas, window) \
          pseudo_saveas_get_window_id(flags, saveas, window, __FILE__, __LINE__)

#define radiobutton_set_state(flags, window, radio_button, state) \
          pseudo_radiobutton_set_state(flags, window, radio_button, state, __FILE__, __LINE__)

#define radiobutton_get_state(flags, window, radio_button, state, selected) \
          pseudo_radiobutton_get_state(flags, window, radio_button, state, selected, __FILE__, __LINE__)

#define optionbutton_set_state(flags, window, option_button, state) \
          pseudo_optionbutton_set_state(flags, window, option_button, state, __FILE__, __LINE__)

#define optionbutton_get_state(flags, window, option_button, state) \
          pseudo_optionbutton_get_state(flags, window, option_button, state, __FILE__, __LINE__)

#define window_set_title(flags, window, title) \
          pseudo_window_set_title(flags, window, title, __FILE__, __LINE__)

#define window_set_extent(flags, window, extent) \
          pseudo_window_set_extent(flags, window, extent, __FILE__, __LINE__)

#define window_get_extent(flags, window, extent) \
          pseudo_window_get_extent(flags, window, extent, __FILE__, __LINE__)

#define window_set_pointer(flags, window, sprite_name, x_hot_spot, y_hot_spot) \
          pseudo_window_set_pointer(flags, window, sprite_name, x_hot_spot, y_hot_spot, __FILE__, __LINE__)

#define window_get_wimp_handle(flags, window, window_handle) \
          pseudo_window_get_wimp_handle(flags, window, window_handle, __FILE__, __LINE__)

#define window_get_tool_bars(flags, window, ibl, itl, ebl, etl) \
          pseudo_window_get_tool_bars(flags, window, ibl, itl, ebl, etl, __FILE__, __LINE__)

#define window_get_pointer_info(flags, x_pos, y_pos, buttons, window, component) \
          pseudo_window_get_pointer_info(flags, x_pos, y_pos, buttons, window, component, __FILE__, __LINE__)

#define window_force_redraw(flags, window, redraw_box) \
          pseudo_window_force_redraw(flags, window, redraw_box, __FILE__, __LINE__)

#define actionbutton_set_text(flags, window, action_button, text) \
          pseudo_actionbutton_set_text(flags, window, action_button, text, __FILE__, __LINE__)

#define gadget_get_bbox(flags, window, gadget, bbox) \
          pseudo_gadget_get_bbox(flags, window, gadget, bbox, __FILE__, __LINE__)

#define gadget_set_help_message(flags, window, gadget, message_text) \
          pseudo_gadget_set_help_message(flags, window, gadget, message_text, __FILE__, __LINE__)

#define gadget_set_focus(flags, window, component) \
          pseudo_gadget_set_focus(flags, window, component, __FILE__, __LINE__)

#define button_set_value(flags, window, button, value) \
          pseudo_button_set_value(flags, window, button, value, __FILE__, __LINE__)

#define button_get_value(flags, window, button, buffer, buff_size, nbytes) \
          pseudo_button_get_value(flags, window, button, buffer, buff_size, nbytes, __FILE__, __LINE__)

#define button_set_validation(flags, window, button, value) \
          pseudo_button_set_validation(flags, window, button, value, __FILE__, __LINE__)

#define numberrange_set_value(flags, window, number_range, value) \
          pseudo_numberrange_set_value(flags, window, number_range, value, __FILE__, __LINE__)

#define numberrange_get_value(flags, window, number_range, value) \
          pseudo_numberrange_get_value(flags, window, number_range, value, __FILE__, __LINE__)

#define slider_set_value(flags, window, slider, value) \
          pseudo_slider_set_value(flags, window, slider, value, __FILE__, __LINE__)

#define slider_set_colour(flags, window, slider, bar_colour, back_colour) \
          pseudo_slider_set_colour(flags, window, slider, bar_colour, back_colour, __FILE__, __LINE__)

#define menu_set_tick(flags, menu, entry, tick) \
          pseudo_menu_set_tick(flags, menu, entry, tick, __FILE__, __LINE__)

#define menu_get_tick(flags, menu, entry, tick) \
          pseudo_menu_get_tick(flags, menu, entry, tick, __FILE__, __LINE__)

#define menu_set_fade(flags, menu, entry, fade) \
          pseudo_menu_set_fade(flags, menu, entry, fade, __FILE__, __LINE__)

#define menu_get_fade(flags, menu, entry, fade) \
          pseudo_menu_get_fade(flags, menu, entry, fade, __FILE__, __LINE__)

#define menu_add_entry(flags, menu, at_entry, entry_description, new_entry) \
          pseudo_menu_add_entry(flags, menu, at_entry, entry_description, new_entry, __FILE__, __LINE__)

#define menu_set_entry_text(flags, menu, entry, text) \
          pseudo_menu_set_entry_text(flags, menu, entry, text, __FILE__, __LINE__)

#define quit_set_message(flags, quit, message) \
          pseudo_quit_set_message(flags, quit, message, __FILE__, __LINE__)

#define quit_get_window_id(flags, quit, window) \
          pseudo_quit_get_window_id(flags, quit, window, __FILE__, __LINE__)

#define colourdbox_get_wimp_handle(flags, colourdbox, wimp_handle) \
          pseudo_colourdbox_get_wimp_handle(flags, colourdbox, wimp_handle, __FILE__, __LINE__)

#define fileinfo_get_window_id(flags, fileinfo, window) \
          pseudo_fileinfo_get_window_id(flags, fileinfo, window, __FILE__, __LINE__)

#define proginfo_get_window_id(flags, proginfo, window) \
          pseudo_proginfo_get_window_id(flags, proginfo, window, __FILE__, __LINE__)

#define scale_get_window_id(flags, scale, window) \
          pseudo_scale_get_window_id(flags, scale, window, __FILE__, __LINE__)

#define fontdbox_get_window_id(flags, fontdbox, window) \
          pseudo_fontdbox_get_window_id(flags, fontdbox, window, __FILE__, __LINE__)

#define dcs_get_window_id(flags, dcs, window) \
          pseudo_dcs_get_window_id(flags, dcs, window, __FILE__, __LINE__)

#define printdbox_get_window_id(flags, printdbox, window) \
          pseudo_printdbox_get_window_id(flags, printdbox, window, __FILE__, __LINE__)

#endif

_Optional _kernel_oserror *pseudo_toolbox_initialise( unsigned int flags,
                                             int wimp_version,
                                             int *wimp_messages,
                                             int *toolbox_events,
                                             char *directory,
                                             MessagesFD *mfd,
                                             IdBlock *idb,
                                             _Optional int *current_wimp_version,
                                             _Optional int *task,
                                             _Optional void *_Optional *sprite_area,
                                             const char *file,
                                             unsigned long line
                                           );

_Optional _kernel_oserror *pseudo_toolbox_create_object(unsigned int flags, void *name_or_template, _Optional ObjectId *id, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_delete_object(unsigned int flags, ObjectId id, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_show_object(unsigned int flags, ObjectId id, int show_type, _Optional void *type, ObjectId parent, ComponentId parent_component, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_hide_object(unsigned int flags, ObjectId id, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_set_client_handle(unsigned int flags, ObjectId id, void *client_handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_get_client_handle(unsigned int flags, ObjectId id, void *_Optional *client_handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_get_object_class(unsigned int flags, ObjectId id, _Optional ObjectClass *object_class, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_toolbox_get_object_state(unsigned int flags, ObjectId id, _Optional unsigned int *state, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_iconbar_get_icon_handle(unsigned int flags, ObjectId iconbar, _Optional int *icon_handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_set_file_name(unsigned int flags, ObjectId saveas, char *file_name, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_set_file_type(unsigned int flags, ObjectId saveas, int file_type, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_get_file_type(unsigned int flags, ObjectId saveas, _Optional int *file_type, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_set_file_size(unsigned int flags, ObjectId saveas, int file_size, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_buffer_filled(unsigned int flags, ObjectId saveas, void *buffer, int bytes_written, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_file_save_completed(unsigned int flags, ObjectId saveas, char *filename, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_saveas_get_window_id(unsigned int flags, ObjectId saveas, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_radiobutton_set_state(unsigned int flags, ObjectId window, ComponentId radio_button, int state, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_radiobutton_get_state(unsigned int flags, ObjectId window, ComponentId radio_button, _Optional int *state, _Optional ComponentId *selected, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_optionbutton_set_state(unsigned int flags, ObjectId window, ComponentId option_button, int state, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_optionbutton_get_state(unsigned int flags, ObjectId window, ComponentId option_button, _Optional int *state, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_set_title(unsigned int flags, ObjectId window, char *title, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_set_extent(unsigned int flags, ObjectId window, BBox *extent, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_get_extent(unsigned int flags, ObjectId window, BBox *extent, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_set_pointer(unsigned int flags, ObjectId window, _Optional char *sprite_name, int x_hot_spot, int y_hot_spot, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_get_wimp_handle(unsigned int flags, ObjectId window, _Optional int *window_handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_get_tool_bars(unsigned int flags, ObjectId window, _Optional ObjectId *ibl, _Optional ObjectId *itl, _Optional ObjectId *ebl, _Optional ObjectId *etl, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_get_pointer_info(unsigned int flags, _Optional int *x_pos, _Optional int *y_pos, _Optional int *buttons, _Optional ObjectId *window, _Optional ComponentId *component, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_window_force_redraw(unsigned int flags, ObjectId window, BBox *redraw_box, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_actionbutton_set_text(unsigned int flags, ObjectId window, ComponentId action_button, char *text, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_gadget_set_help_message(unsigned int flags, ObjectId window, ComponentId gadget, char *message_text, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_gadget_set_focus (unsigned int flags, ObjectId window, ComponentId component, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_gadget_get_bbox(unsigned int flags, ObjectId window, ComponentId gadget, BBox *bbox, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_button_set_value(unsigned int flags, ObjectId window, ComponentId button, char *value, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_button_get_value(unsigned int flags, ObjectId window, ComponentId button, char *buffer, int buff_size, _Optional int *nbytes, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_button_set_validation(unsigned int flags, ObjectId window, ComponentId button, char *value, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_numberrange_set_value(unsigned int flags, ObjectId window, ComponentId number_range, int value, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_numberrange_get_value(unsigned int flags, ObjectId window, ComponentId number_range, _Optional int *value, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_slider_set_value(unsigned int flags, ObjectId window, ComponentId slider, int value, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_slider_set_colour(unsigned int flags, ObjectId window, ComponentId slider, int bar_colour, int back_colour, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_set_tick(unsigned int flags, ObjectId menu, ComponentId entry, int tick, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_get_tick(unsigned int flags, ObjectId menu, ComponentId entry, _Optional int *tick, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_set_fade(unsigned int flags, ObjectId menu, ComponentId entry, int tick, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_get_fade(unsigned int flags, ObjectId menu, ComponentId entry, _Optional int *tick, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_add_entry(unsigned int flags, ObjectId menu, ComponentId at_entry, char *entry_description, _Optional ComponentId *new_entry, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_menu_set_entry_text(unsigned int flags, ObjectId menu, ComponentId entry, char *text, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_quit_set_message(unsigned int flags, ObjectId quit, char *message, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_quit_get_window_id(unsigned int flags, ObjectId quit, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_colourdbox_get_wimp_handle(unsigned int flags, ObjectId colourdbox, _Optional int *wimp_handle, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_fileinfo_get_window_id(unsigned int flags, ObjectId fileinfo, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_proginfo_get_window_id(unsigned int flags, ObjectId proginfo, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_scale_get_window_id(unsigned int flags, ObjectId scale, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_fontdbox_get_window_id(unsigned int flags, ObjectId fontdbox, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_dcs_get_window_id(unsigned int flags, ObjectId dcs, _Optional ObjectId *window, const char *file, unsigned long line);

_Optional _kernel_oserror *pseudo_printdbox_get_window_id(unsigned int flags, ObjectId printdbox, _Optional ObjectId *window, const char *file, unsigned long line);

/* The following functions are for use in unit tests */

void pseudo_toolbox_object_created(ObjectId id);
   /*
    * Called when a toolbox object is created.
    */

void pseudo_toolbox_object_deleted(ObjectId id);
   /*
    * Called when a toolbox object is deleted.
    */

bool pseudo_toolbox_object_is_showing(ObjectId id);
   /*
    * Find out whether an object has been shown since it was last hidden
    * or since pseudo_toolbox_reset was called, whichever happened more
    * recently.
    * This function doesn't take account of objects shown or hidden unless
    * by calling toolbox_show_object and toolbox_hide_object. Events can't
    * be relied upon to provide this information (unlike object creation
    * and deletion) because they are optional.
    */

ObjectId pseudo_toolbox_find_by_template_name(char *template_name);
   /*
    * Gets the ID of the object most recently created from a template with
    * the given name and not yet destroyed.
    * Returns: the ID of an object created from the given template, or
    *          NULL_ObjectId if none exists.
    */

void pseudo_saveas_reset_file_save_completed(void);
   /*
    * Resets the parameter values stored by saveas_file_save_completed
    * to avoid false matches and enables interception of all notifications
    * subsequently sent via saveas_file_save_completed.
    */

ObjectId pseudo_saveas_get_file_save_completed(_Optional unsigned int *flags,
                                               _Optional char *buffer,
                                               int buff_size,
                                               _Optional int *nbytes);
   /*
    * Gets the parameter values passed by the last caller of
    * saveas_file_save_completed.
    * Returns: the ID of the saveas object, or NULL_ObjectId if
    *          saveas_file_save_completed hasn't been called since
    *          the last reset.
    */

void pseudo_saveas_reset_buffer_filled(void);
   /*
    * Resets the parameter values stored by saveas_buffer_filled
    * to avoid false matches and enables interception of all data
    * subsequently sent via saveas_buffer_filled.
    */

ObjectId pseudo_saveas_get_buffer_filled(_Optional unsigned int *flags,
                                         _Optional void *buffer,
                                         int buff_size,
                                         _Optional int *nbytes);
   /*
    * Gets the parameter values passed by the last caller of
    * saveas_buffer_filled.
    * Returns: the ID of the saveas object, or NULL_ObjectId if
    *          saveas_buffer_filled hasn't been called since
    *          the last reset.
    */

void pseudo_toolbox_reset(void);
   /*
    * Resets the parameter values stored by saveas_file_save_completed and
    * saveas_buffer_filled to avoid false matches and enables interception
    * of all notifications subsequently sent via those functions. Also
    * forgets all objects previously shown.
    */

#endif
