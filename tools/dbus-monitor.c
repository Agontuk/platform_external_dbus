/* -*- mode: C; c-file-style: "gnu" -*- */
/* dbus-monitor.c  Utility program to monitor messages on the bus
 *
 * Copyright (C) 2003 Philip Blundell <philb@gnu.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <dbus/dbus.h>
 /* Don't copy this, for programs outside the dbus tree it's dbus/dbus-glib.h */
#include <glib/dbus-glib.h>
#include "dbus-print-message.h"

static DBusHandlerResult
handler_func (DBusMessageHandler *handler,
 	      DBusConnection     *connection,
	      DBusMessage        *message,
	      void               *user_data)
{
  print_message (message);
  
  if (dbus_message_has_name (message, DBUS_MESSAGE_LOCAL_DISCONNECT))
    exit (0);
  
  return DBUS_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

static void
usage (char *name, int ecode)
{
  fprintf (stderr, "Usage: %s [--session]\n", name);
  exit (ecode);
}

int
main (int argc, char *argv[])
{
  DBusConnection *connection;
  DBusError error;
  DBusBusType type = DBUS_BUS_SYSTEM;
  DBusMessageHandler *handler;
  GMainLoop *loop;
  int i;

  for (i = 1; i < argc; i++)
    {
      char *arg = argv[i];

      if (!strcmp (arg, "--session"))
	type = DBUS_BUS_SESSION;
      else if (!strcmp (arg, "--help"))
	usage (argv[0], 0);
      else if (!strcmp (arg, "--"))
	break;
      else if (arg[0] == '-')
	usage (argv[0], 1);
    }

  if (argc > 2)
    usage (argv[0], 1);

  loop = g_main_loop_new (NULL, FALSE);

  dbus_error_init (&error);
  connection = dbus_bus_get (type, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "Failed to open connection to %s message bus: %s\n",
	       (type == DBUS_BUS_SYSTEM) ? "system" : "session",
               error.message);
      dbus_error_free (&error);
      exit (1);
    }

  dbus_connection_setup_with_g_main (connection, NULL);

  handler = dbus_message_handler_new (handler_func, NULL, NULL);
  dbus_connection_add_filter (connection, handler);

  g_main_loop_run (loop);

  exit (0);
}
