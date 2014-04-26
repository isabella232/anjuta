/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
    plugin.c
    Copyright (C) 2014 Tristian Celestin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <libanjuta/anjuta-plugin.h>

#define ANJUTA_TYPE_PLUGIN_STARTER         (starter_plugin_get_type (NULL))
#define ANJUTA_PLUGIN_STARTER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ANJUTA_TYPE_PLUGIN_STARTER, StarterPlugin))
#define ANJUTA_PLUGIN_STARTER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), ANJUTA_TYPE_PLUGIN_STARTER, StarterPluginClass))
#define ANJUTA_IS_PLUGIN_STARTER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ANJUTA_TYPE_PLUGIN_STARTER))
#define ANJUTA_IS_PLUGIN_STARTER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), ANJUTA_TYPE_PLUGIN_STARTER))
#define ANJUTA_PLUGIN_STARTER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ANJUTA_TYPE_PLUGIN_STARTER, StarterPluginClass))

typedef struct _StarterPlugin StarterPlugin;
typedef struct _StarterPluginClass StarterPluginClass;
typedef struct _StarterPluginPrivate StarterPluginPrivate;

struct _StarterPlugin{
	AnjutaPlugin parent;
	StarterPluginPrivate *priv;
};

struct _StarterPluginClass{
	AnjutaPluginClass parent_class;
};

extern GType starter_plugin_get_type (GTypeModule *module);

void on_recent_project_activated (GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
void on_row_activated (GtkListBox *box, GtkListBoxRow *row, gpointer user_data);

#endif
