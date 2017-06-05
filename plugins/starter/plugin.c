/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
    plugin.c
    Copyright (C) 2008 Ignacio Casal Quinteiro
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

#include <config.h>
#include <libanjuta/anjuta-shell.h>
#include <libanjuta/anjuta-debug.h>
#include <libanjuta/anjuta-encodings.h>
#include <libanjuta/interfaces/ianjuta-document-manager.h>
#include <libanjuta/interfaces/ianjuta-project-manager.h>
#include <libanjuta/interfaces/ianjuta-file-loader.h>
#include <libanjuta/interfaces/ianjuta-wizard.h>
#include <gtk/gtk.h>
#include "plugin.h"

#define PREF_SCHEMA "org.gnome.anjuta.starter"
#define RECENT_LIMIT "recent-limit"
#define STARTER_BOX "starter_box"
#define SWITCHER_BOX "switcher_box"
#define ACTIONS_LISTBOX "actions_listbox"
#define ACTIONS_FRAME "actions_frame"
#define RECENT_PROJECTS_BOX "recent_projects_box"
#define RECENT_LISTBOX "recent_listbox"
#define IMPORT_ROW "import_row"
#define ANJUTA_DOC_ROW "anjuta_doc_row"
#define ANJUTA_FAQ_ROW "anjuta_faq_row"
#define ONLINE_DOC_ROW "gtk_doc_row"
#define RECENT_ROW "recent_row"
#define CREATE_ROW "create_row"
#define CODE_ROW "code_row"
#define PROJECT_LABEL "project_label"
#define PATH_LABEL "path_label"
#define TRANSITION_TIME 90
#define URI_KEY "uri"
#define REMOVE_PROJECT_BUTTON "remove_project_button"
#define ACTIONS_ID "actions"
#define RECENT_PROJECTS_ID "recent_projects"
#define PROJECT_WIZARD_ID "anjuta-project-wizard:NPWPlugin"
#define PROJECT_IMPORT_ID "anjuta-project-import:AnjutaProjectImportPlugin"

struct _StarterPluginPrivate {
	GtkWidget *starter;
	gint editor_watch_id;
	gint project_watch_id;
};

void
on_recent_project_activated (GtkListBox *box, GtkListBoxRow *row, gpointer user_data)
{
	GFile *file;
	IAnjutaFileLoader *loader;

	loader = anjuta_shell_get_interface (anjuta_plugin_get_shell (ANJUTA_PLUGIN (user_data)), IAnjutaFileLoader, NULL);
	file = g_file_new_for_uri ((const char*) g_object_get_data (G_OBJECT (row), URI_KEY));
	ianjuta_file_loader_load (IANJUTA_FILE_LOADER (loader), file, FALSE, NULL);
}

static void
on_new_project_activated (GtkListBoxRow *row, gpointer user_data)
{
	AnjutaPlugin *plugin = ANJUTA_PLUGIN (user_data);
	AnjutaPluginManager *plugin_manager =
		anjuta_shell_get_plugin_manager (anjuta_plugin_get_shell (plugin),
										 NULL);
	GList *plugin_handles = NULL;


	plugin_handles = anjuta_plugin_manager_query (plugin_manager,
												  "Anjuta Plugin",
												  "Location",
												  PROJECT_WIZARD_ID,
												  NULL);
	if (plugin_handles != NULL)
	{
		GObject* wizard =
			anjuta_plugin_manager_get_plugin_by_handle (plugin_manager, (AnjutaPluginHandle *)plugin_handles->data);
		if (wizard)
			ianjuta_wizard_activate (IANJUTA_WIZARD (wizard), NULL);
	}
	g_list_free (plugin_handles);
}

static void
on_local_doc_activated (GtkListBoxRow *row, gpointer user_data)
{
	gtk_show_uri (NULL, "help:anjuta-manual", GDK_CURRENT_TIME, NULL);
}

static void
on_local_faq_activated (GtkListBoxRow *row, gpointer user_data)
{
	gtk_show_uri (NULL, "help:anjuta-faqs", GDK_CURRENT_TIME, NULL);
}

static void
on_online_doc_activated (GtkListBoxRow *row, gpointer user_data)
{
	gtk_show_uri (NULL, "https://developer.gnome.org/references", GDK_CURRENT_TIME, NULL);
}

static void
on_search_example_code_activated (GtkListBoxRow *row, gpointer user_data)
{
	gtk_show_uri (NULL, "http://www.softwareheritage.org/archive", GDK_CURRENT_TIME, NULL);
}

static void
on_import_project_activated (GtkListBoxRow *row, gpointer user_data)
{
	AnjutaPlugin* plugin = ANJUTA_PLUGIN (user_data);
	AnjutaPluginManager* plugin_manager = 
		anjuta_shell_get_plugin_manager (anjuta_plugin_get_shell (plugin),
										 NULL);
	GList *plugin_handles = NULL;

	plugin_handles = anjuta_plugin_manager_query (plugin_manager,
												  "Anjuta Plugin",
												  "Location",
												  PROJECT_IMPORT_ID,
												  NULL);

	if (plugin_handles != NULL)
	{
		GObject* wizard =
			anjuta_plugin_manager_get_plugin_by_handle (plugin_manager, (AnjutaPluginHandle *)plugin_handles->data);
		if (wizard)
			ianjuta_wizard_activate (IANJUTA_WIZARD (wizard), NULL);
	}
	g_list_free (plugin_handles);
}

void
on_row_activated (GtkListBox *box, GtkListBoxRow *row, gpointer user_data)
{
	gchar *name;

	if (row != NULL)
	{
		name = gtk_widget_get_name (row);
		if (name != NULL) {
			if (g_strcmp0 (name, CREATE_ROW) == 0)
				on_new_project_activated (row, user_data);
			else if (g_strcmp0 (name, IMPORT_ROW) == 0)
				on_import_project_activated (row, user_data);
			else if (g_strcmp0 (name, ANJUTA_DOC_ROW) == 0)
				on_local_doc_activated (row, user_data);
			else if (g_strcmp0 (name, ANJUTA_FAQ_ROW) == 0)
				on_local_faq_activated (row, user_data);
			else if (g_strcmp0 (name, ONLINE_DOC_ROW) == 0)
				on_online_doc_activated (row, user_data);
			else if (g_strcmp0 (name, CODE_ROW) == 0)
				on_search_example_code_activated (row, user_data);
		}
	}
}

static void
add_recent_project_row (GtkListBox *recent_project_box, GtkRecentData *recent_project)
{
	GtkBuilder *builder;
	GFile *file;
	GtkWidget *recent_row;
	GtkLabel *project_label, *path_label;
	GError *error;

	error = NULL;
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_resource (builder, "/org/gnome/anjuta/ui/starter.ui", &error))
	{
		DEBUG_PRINT ("Could not load starter.ui! %s", error->message);
		g_error_free (error);
	}
	else
	{
		file = g_file_new_for_uri (gtk_recent_info_get_uri (recent_project));
		if (g_file_query_exists (file, NULL))
		{
			recent_row = GTK_WIDGET (gtk_builder_get_object (builder, RECENT_ROW));
			project_label = GTK_WIDGET (gtk_builder_get_object (builder, PROJECT_LABEL));
			path_label = GTK_WIDGET (gtk_builder_get_object (builder, PATH_LABEL));
			gtk_label_set_text (project_label, gtk_recent_info_get_display_name(recent_project));
			gtk_label_set_text (path_label, g_file_get_path(file));
			g_object_set_data_full (G_OBJECT (recent_row), URI_KEY, g_file_get_uri(file), g_free);
			gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (recent_row)), recent_row);
			gtk_list_box_insert (recent_project_box, recent_row, -1);
		}
		g_object_unref (file);

	}
	g_object_unref (builder);
}

static void
refresh_recent_project_view (GtkListBox *box)
{
	GSettings *settings;
	GtkRecentManager *manager;
	GtkRecentData *recent_project;
	GList *items, *list;
	gint i;
	guint recent_limit;

	manager = gtk_recent_manager_get_default ();
	items = gtk_recent_manager_get_items (manager);
	items = g_list_reverse (items);
	list = items;
	settings = g_settings_new (PREF_SCHEMA);
	i = 0;
	g_settings_get (settings, RECENT_LIMIT, "i", &recent_limit);
	while (i < recent_limit && list != NULL)
	{
		if (strcmp (gtk_recent_info_get_mime_type (list->data), "application/x-anjuta") == 0)
		{
			recent_project = list->data;
			add_recent_project_row (box, recent_project);
			i++;
		}
		list = list->next;
	}
	g_list_free_full(items, (GDestroyNotify)gtk_recent_info_unref);
	g_object_unref (settings);
}

static void
on_remove_project_clicked (GtkButton *button, gpointer user_data)
{
	GtkRecentManager *manager;
	GtkListBox *recent_list_box;
	GtkListBoxRow *row;
	GError *error;

	manager = gtk_recent_manager_get_default ();
	recent_list_box = GTK_LIST_BOX (user_data);
	row = gtk_list_box_get_selected_row (recent_list_box);
	error = NULL;
	if (row != NULL)
	{
		if (gtk_recent_manager_remove_item (manager, g_object_get_data (row, URI_KEY), &error))
		{
			gtk_container_remove (GTK_CONTAINER (recent_list_box), row);
		}
		else
		{
			DEBUG_PRINT ("Could not remove recent item. %s", error->message);
			g_error_free (error);
		}

	}
}

static void
add_action_separators (GtkListBoxRow *row, GtkListBoxRow *before, gpointer user_data)
{
	GtkWidget *current;

	if (before == NULL)
		return;

	current = gtk_list_box_row_get_header (row);
	if (current == NULL)
	{
		current = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_widget_show (current);
		gtk_list_box_row_set_header (row, current);
	}
}

static void
show_starter_page ()
{
	return;
}

static GtkWidget*
create_starter_widget (StarterPlugin* plugin)
{
	GError *error;
	GtkWidget *stack;
	GtkWidget *switcher;
	GtkWidget *starter_box;
	GtkWidget *switcher_box;
	GtkWidget *actions_frame;
	GtkWidget *actions_listbox;
	GtkWidget *recent_projects_box;
	GtkWidget *recent_listbox;
	GtkBuilder *builder;
	GtkWidget *button;
	
	error = NULL;
	builder = gtk_builder_new ();

	if (!gtk_builder_add_from_resource (builder, "/org/gnome/anjuta/ui/starter.ui", &error))
	{
		DEBUG_PRINT ("Could not load starter.ui! %s", error->message);
		g_error_free (error);
	}
	else
	{
		/* Manually assembling stack and switcher because they are not available in glade yet */
		switcher = gtk_stack_switcher_new ();
		stack = gtk_stack_new ();
		gtk_stack_switcher_set_stack (switcher, stack);
		gtk_stack_set_transition_type (stack, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
		gtk_stack_set_transition_duration (stack, TRANSITION_TIME);

		starter_box = GTK_WIDGET (gtk_builder_get_object (builder, STARTER_BOX));
		switcher_box = GTK_WIDGET (gtk_builder_get_object (builder, SWITCHER_BOX));
		gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (starter_box)), starter_box);
		g_object_ref (starter_box);
		gtk_box_pack_start (switcher_box, switcher, FALSE, FALSE, 0);
		gtk_box_pack_start (starter_box, stack, FALSE, FALSE, 0);
		gtk_widget_show_all (starter_box);

		actions_listbox = GTK_WIDGET (gtk_builder_get_object (builder, ACTIONS_LISTBOX));
		gtk_list_box_set_header_func (GTK_LIST_BOX (actions_listbox), add_action_separators, NULL, NULL);
		actions_frame = GTK_WIDGET (gtk_builder_get_object (builder, ACTIONS_FRAME));
		gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (actions_frame)), actions_frame);
		g_object_ref (actions_frame);
		gtk_stack_add_titled (stack, actions_frame, ACTIONS_ID, "Actions");

		recent_projects_box = GTK_WIDGET (gtk_builder_get_object (builder, RECENT_PROJECTS_BOX));
		gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (recent_projects_box)), recent_projects_box);
		g_object_ref (recent_projects_box);

		recent_listbox = GTK_WIDGET (gtk_builder_get_object (builder, RECENT_LISTBOX));
		refresh_recent_project_view (GTK_LIST_BOX (recent_listbox));

		gtk_stack_add_titled (stack, recent_projects_box, RECENT_PROJECTS_ID, "Recent Projects");

		button = GTK_WIDGET (gtk_builder_get_object (builder, REMOVE_PROJECT_BUTTON));
		g_signal_connect_object (G_OBJECT (button), "clicked",
			G_CALLBACK (on_remove_project_clicked), recent_listbox, G_CONNECT_AFTER);

		gtk_builder_connect_signals (builder, plugin);
	}
	g_object_unref (builder);
	return starter_box;
}

/* Remove the starter plugin once a document was opened */
static void
on_value_added_current_project (AnjutaPlugin *plugin, const gchar *name,
							   const GValue *value, gpointer data)
{
	GObject *project;
	AnjutaShell *shell;
	StarterPlugin *splugin;
	
	project = g_value_get_object (value);
	shell = ANJUTA_PLUGIN(plugin)->shell;
	splugin = ANJUTA_PLUGIN_STARTER (plugin);
	if (project)
	{
		if (splugin->priv->starter)
		{
			DEBUG_PRINT ("Hiding starter");
			anjuta_shell_remove_widget (shell, splugin->priv->starter, NULL);
		}
		splugin->priv->starter = NULL;
	}
}


/* Remove the starter plugin once a document was opened */
static void
on_value_added_current_editor (AnjutaPlugin *plugin, const gchar *name,
							   const GValue *value, gpointer data)
{
	GObject *doc;
	AnjutaShell *shell;
	StarterPlugin *splugin;

	doc = g_value_get_object (value);
	shell = ANJUTA_PLUGIN(plugin)->shell;
	splugin = ANJUTA_PLUGIN_STARTER (plugin);
	if (doc)
	{
		if (splugin->priv->starter)
		{
			DEBUG_PRINT ("Hiding starter");
			anjuta_shell_remove_widget (shell, splugin->priv->starter, NULL);
		}
		splugin->priv->starter = NULL;
	}
}

static void
on_value_removed (AnjutaPlugin *plugin,
				  const gchar *name,
				  gpointer data)
{
	AnjutaShell* shell = anjuta_plugin_get_shell (plugin);
	StarterPlugin* splugin = ANJUTA_PLUGIN_STARTER (plugin);
	IAnjutaDocumentManager* docman = anjuta_shell_get_interface (shell,
																 IAnjutaDocumentManager,
																 NULL);
	IAnjutaProjectManager* pm = anjuta_shell_get_interface (shell,
															IAnjutaProjectManager,
															NULL);
	
	if (!(docman && ianjuta_document_manager_get_doc_widgets (docman, NULL)) &&
		!(pm && ianjuta_project_manager_get_current_project (pm, NULL)))
	{
		DEBUG_PRINT ("Showing starter");
		splugin->priv->starter = create_starter_widget (splugin);
		anjuta_shell_add_widget (shell, splugin->priv->starter,
								 "AnjutaStarter",
								 _("Start"),
								 GTK_STOCK_ABOUT,
								 ANJUTA_SHELL_PLACEMENT_CENTER,
								 NULL);
		anjuta_shell_present_widget (shell, splugin->priv->starter, NULL);
		g_object_unref (splugin->priv->starter);
	}
}

static void
on_session_load (AnjutaShell *shell,
				 AnjutaSessionPhase phase,
				 AnjutaSession *session,
				 StarterPlugin *plugin)
{
	if (phase == ANJUTA_SESSION_PHASE_END)
	{
		if (!plugin->priv->starter)
			on_value_removed (ANJUTA_PLUGIN (plugin), NULL, plugin);
		if (plugin->priv->starter)
		{
			anjuta_shell_maximize_widget (shell,
										  "AnjutaStarter",
										  NULL);
		}
	}
}

static gboolean
activate_plugin (AnjutaPlugin *plugin)
{
	StarterPlugin *splugin = ANJUTA_PLUGIN_STARTER (plugin);

	DEBUG_PRINT ("StarterPlugin: Activating document manager plugin...");
	
	splugin->priv->editor_watch_id =
		anjuta_plugin_add_watch (plugin,
								 IANJUTA_DOCUMENT_MANAGER_CURRENT_DOCUMENT,
								 on_value_added_current_editor,
								 on_value_removed,
								 NULL);
	splugin->priv->project_watch_id =
		anjuta_plugin_add_watch (plugin,
								 IANJUTA_PROJECT_MANAGER_CURRENT_PROJECT,
								 on_value_added_current_project,
								 on_value_removed,
								 NULL);
	on_value_removed (plugin, NULL, splugin);

	g_signal_connect (anjuta_plugin_get_shell (plugin),
					  "load-session",
					  G_CALLBACK (on_session_load),
					  plugin);
	
	return TRUE;
}

static gboolean
deactivate_plugin (AnjutaPlugin *plugin)
{
	StarterPlugin* splugin = ANJUTA_PLUGIN_STARTER (plugin);
	DEBUG_PRINT ("StarterPlugin: Deactivating starter plugin...");
	if (splugin->priv->starter)
		anjuta_shell_remove_widget (anjuta_plugin_get_shell (plugin),
									splugin->priv->starter, NULL);

	anjuta_plugin_remove_watch (plugin, splugin->priv->editor_watch_id, FALSE);
	anjuta_plugin_remove_watch (plugin, splugin->priv->project_watch_id, FALSE);
	
	return TRUE;
}

static void
dispose (GObject *obj)
{
	AnjutaPluginClass *klass;
	gpointer parent_class;

	klass =  ANJUTA_PLUGIN_GET_CLASS(obj);
	parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
finalize (GObject *obj)
{
	AnjutaPluginClass *klass;
	gpointer parent_class;

	/* Finalization codes here */
	klass =  ANJUTA_PLUGIN_GET_CLASS(obj);
	parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
starter_plugin_instance_init (GObject *obj)
{
	StarterPlugin *splugin;

	splugin = ANJUTA_PLUGIN_STARTER (obj);
	splugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (obj, ANJUTA_TYPE_PLUGIN_STARTER, StarterPluginPrivate);
}

static void
starter_plugin_class_init (GObjectClass *klass) 
{
	AnjutaPluginClass *plugin_class;

	g_type_class_add_private (klass, sizeof (StarterPluginPrivate));
	plugin_class = ANJUTA_PLUGIN_CLASS (klass);
	plugin_class->activate = activate_plugin;
	plugin_class->deactivate = deactivate_plugin;
	klass->dispose = dispose;
	klass->finalize = finalize;
}

ANJUTA_PLUGIN_BOILERPLATE (StarterPlugin, starter_plugin);
ANJUTA_SIMPLE_PLUGIN (StarterPlugin, starter_plugin);
