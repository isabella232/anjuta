/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
    plugin.c
    Copyright (C) 2000 Naba Kumar
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

#include <signal.h>

#include <libanjuta/anjuta-shell.h>
#include <libanjuta/anjuta-debug.h>

#include <libanjuta/interfaces/ianjuta-terminal.h>
#include <libanjuta/interfaces/ianjuta-preferences.h>
#include <libanjuta/interfaces/ianjuta-project-manager.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define UI_FILE PACKAGE_DATA_DIR"/ui/anjuta-terminal-plugin.xml"
#define PREFS_BUILDER PACKAGE_DATA_DIR"/glade/anjuta-terminal-plugin.ui"
#define ICON_FILE "anjuta-terminal-plugin-48.png"

/* Gnome-Terminal GSettings Schemas and Keys */
#define TERM_PROFILE_LIST_SCHEMA		"org.gnome.Terminal.ProfilesList"
#define TERM_LEGACY_SCHEMA				"org.gnome.Terminal.Legacy.Settings"
#define TERM_PROFILE_DEFAULT			"default"

/* Gnome Desktop GSettings Schemas and Keys */
#define GNOME_DESKTOP_INTERFACE_SCHEMA			"org.gnome.desktop.interface"
#define GNOME_MONOSPACE_FONT					"monospace-font-name"

/* Anjuta Terminal Plugin Schema and Keys */
#define PREF_SCHEMA                           "org.gnome.anjuta.terminal"
#define PREFS_TERMINAL_PROFILE_USE_DEFAULT    "use-default-profile"
#define PREFS_TERMINAL_PROFILE                "terminal-profile"

/* Columns in model for combo box */
enum {
	TERM_STORE_UUID_COLUMN,
	TERM_STORE_NAME_COLUMN
};

#include <vte/vte.h>
#include <pwd.h>
#include <gtk/gtk.h>
#include <libanjuta/anjuta-plugin.h>
#include "terminal-schemas.h"

extern GType terminal_plugin_get_type (GTypeModule *module);
#define ANJUTA_PLUGIN_TERMINAL_TYPE         (terminal_plugin_get_type (NULL))
#define ANJUTA_PLUGIN_TERMINAL(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ANJUTA_PLUGIN_TERMINAL_TYPE, TerminalPlugin))
#define ANJUTA_PLUGIN_TERMINAL_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), ANJUTA_PLUGIN_TERMINAL_CLASS, TerminalPluginClass))
#define ANJUTA_IS_PLUGIN_TERMINAL(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ANJUTA_PLUGIN_TERMINAL_TYPE))
#define ANJUTA_IS_PLUGIN_TERMINAL_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), ANJUTA_PLUGIN_TERMINAL_TYPE))
#define ANJUTA_PLUGIN_TERMINAL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ANJUTA_PLUGIN_TERMINAL_TYPE, TerminalPluginClass))

typedef struct _TerminalPlugin TerminalPlugin;
typedef struct _TerminalPluginClass TerminalPluginClass;

struct _TerminalPlugin{
	AnjutaPlugin parent;

	gint uiid;
	GtkActionGroup *action_group;

	GPid child_pid;
	GtkWidget *shell, *term;
	GtkWidget *shell_box, *term_box;
	GtkWidget *frame;
	GtkWidget *pref_profile_combo;
	GtkWidget *pref_default_button;
	gboolean   widget_added_to_shell;
	GSettings *settings;
	guint root_watch_id;
	gboolean pty_flags;
#if OLD_VTE == 1
	gboolean first_time_realization;
#endif
};

struct _TerminalPluginClass{
	AnjutaPluginClass parent_class;
};

static gpointer parent_class;

#if 0
static const gchar*
get_profile_key (const gchar *profile, const gchar *key)
{
	/* A reasonably safe buffer */
	static gchar buffer[1024];

	g_return_val_if_fail (profile != NULL && key != NULL, NULL);

	snprintf (buffer, 1024, "%s/%s/%s", GCONF_PROFILE_PREFIX, profile, key);
	return buffer;
}
#endif

static gboolean
strv_to_rgbav (const gchar **specs, gsize *size, GdkRGBA **colors)
{
	GVariant *var;
	gint i;
	GVariantIter iter;
	const char *str;

	var = g_variant_new_strv (specs, -1);
	g_variant_iter_init (&iter, var);
	*size = g_variant_iter_n_children (&iter);
	*colors = g_new (GdkRGBA, *size);
	i = 0;
	while (g_variant_iter_next (&iter, "&s", &str)) {
		if (!gdk_rgba_parse (&(*colors)[i++], str)) {
			g_free (*colors);
			g_variant_unref (var);
			return FALSE;
		}
	}
	g_variant_unref (var);
	return TRUE;
}

static void
terminal_set_preferences (VteTerminal *term, GSettings* settings, TerminalPlugin *term_plugin)
{
	GSettingsSchemaSource *source;
	GSettingsSchema *schema;
	gchar *uuid;
	gchar *path;
	GSettings *profiles_list;
	GSettings *profile_settings;
	GSettings *interface_settings;
	gboolean bool_val;
	gchar *str_val;
	gchar **str_vals;
	gint int_val;
	gsize size;
	GdkRGBA color[2];
	GdkRGBA *foreground;
	GdkRGBA *background;
	GdkRGBA *palette;

	g_return_if_fail (settings != NULL);

	/* Always autohide mouse */
	vte_terminal_set_mouse_autohide (term, TRUE);

	/* Check that terminal schemas really exist. They could be missing if gnome
	 * terminal is not installed or there is an old version */
	source = g_settings_schema_source_get_default ();
	schema = g_settings_schema_source_lookup (source, TERMINAL_PROFILES_LIST_SCHEMA, TRUE);
	if (schema == NULL) return;

	profiles_list = g_settings_new_full (schema, NULL, NULL);
	g_settings_schema_unref (schema);

	/* Get selected profile */
	bool_val = g_settings_get_boolean (settings, PREFS_TERMINAL_PROFILE_USE_DEFAULT);
	if (bool_val)
		uuid = g_settings_get_string (profiles_list, TERMINAL_SETTINGS_LIST_DEFAULT_KEY);
	else
		uuid = g_settings_get_string (settings, PREFS_TERMINAL_PROFILE);
	path = g_strdup_printf ("%s:%s/", TERMINAL_PROFILES_PATH_PREFIX, uuid);
	profile_settings = g_settings_new_with_path (TERMINAL_PROFILE_SCHEMA, path);
	g_free (uuid);
	g_free (path);

	/* Set terminal font */
	bool_val = g_settings_get_boolean (profile_settings, TERMINAL_PROFILE_USE_SYSTEM_FONT_KEY);
	if (bool_val)
	{
		interface_settings = g_settings_new (GNOME_DESKTOP_INTERFACE_SCHEMA);
		str_val = g_settings_get_string (interface_settings, GNOME_MONOSPACE_FONT);
		g_object_unref (interface_settings);
	} else
	{
		str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_FONT_KEY);
	}
	if (str_val != NULL)
		vte_terminal_set_font_from_string (term, str_val);

	/* Set cursor blink */
	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_CURSOR_BLINK_MODE_KEY);
	if (g_strcmp0 (str_val, "system") == 0)
		vte_terminal_set_cursor_blink_mode (term, VTE_CURSOR_BLINK_SYSTEM);
	else if (g_strcmp0 (str_val, "on") == 0)
		vte_terminal_set_cursor_blink_mode (term, VTE_CURSOR_BLINK_ON);
	else if (g_strcmp0 (str_val, "off") == 0)
		vte_terminal_set_cursor_blink_mode (term, VTE_CURSOR_BLINK_OFF);
	g_free (str_val);

	/* Set audible bell */
	bool_val = g_settings_get_boolean (profile_settings, TERMINAL_PROFILE_AUDIBLE_BELL_KEY);
	vte_terminal_set_audible_bell (term, bool_val);

	/* Set scrollback */
	int_val = g_settings_get_int (profile_settings, TERMINAL_PROFILE_SCROLLBACK_LINES_KEY);
	vte_terminal_set_scrollback_lines (term, (int_val == 0) ? 500 : int_val);

	/* Set scroll on keystroke */
	bool_val = g_settings_get_boolean (profile_settings, TERMINAL_PROFILE_SCROLL_ON_KEYSTROKE_KEY);
	vte_terminal_set_scroll_on_keystroke (term, bool_val);

	/* Scroll on output */
	bool_val = g_settings_get_boolean (profile_settings, TERMINAL_PROFILE_SCROLL_ON_OUTPUT_KEY);
	vte_terminal_set_scroll_on_output (term, bool_val);

	/* Set word characters */
	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_WORD_CHARS_KEY);
	if (str_val != NULL)
		vte_terminal_set_word_chars (term, str_val);
	g_free (str_val);

	/* Set backspace key */
	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_BACKSPACE_BINDING_KEY);
	if (str_val != NULL)
	{
		if (g_strcmp0 (str_val, "ascii-del") == 0)
			vte_terminal_set_backspace_binding (term, VTE_ERASE_ASCII_DELETE);
		else if (g_strcmp0 (str_val, "escape-sequence") == 0)
			vte_terminal_set_backspace_binding (term, VTE_ERASE_DELETE_SEQUENCE);
		else if (g_strcmp0 (str_val, "control-h") == 0)
			vte_terminal_set_backspace_binding (term, VTE_ERASE_ASCII_BACKSPACE);
		else
			vte_terminal_set_backspace_binding (term, VTE_ERASE_AUTO);
	}
	g_free (str_val);

	/* Set delete key */
	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_DELETE_BINDING_KEY);
	if (str_val != NULL)
	{
		if (g_strcmp0 (str_val, "ascii-del") == 0)
			vte_terminal_set_delete_binding (term, VTE_ERASE_ASCII_DELETE);
		else if (g_strcmp0 (str_val, "escape-sequence") == 0)
			vte_terminal_set_delete_binding (term, VTE_ERASE_DELETE_SEQUENCE);
		else if (g_strcmp0 (str_val, "control-h") == 0)
			vte_terminal_set_delete_binding (term, VTE_ERASE_ASCII_BACKSPACE);
		else
			vte_terminal_set_delete_binding (term, VTE_ERASE_AUTO);
	}
	g_free (str_val);

	/* Set colors (foreground, background, and palette) */
	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_BACKGROUND_COLOR_KEY);
	if (str_val != NULL)
		bool_val = gdk_rgba_parse (&color[0], str_val);
	background = bool_val ? &color[0] : NULL;
	g_free (str_val);

	str_val = g_settings_get_string (profile_settings, TERMINAL_PROFILE_FOREGROUND_COLOR_KEY);
	if (str_val != NULL)
		bool_val = gdk_rgba_parse (&color[1], str_val);
	foreground = bool_val ? &color[1] : NULL;
	g_free (str_val);

	str_vals = g_settings_get_strv(profile_settings, TERMINAL_PROFILE_PALETTE_KEY);
	strv_to_rgbav (str_vals, &size, &palette);
	g_free (str_vals);

	/* vte_terminal_set_colors() works even if the terminal widget is not realized,
	 * which is not the case with vte_terminal_set_color_foreground() and
	 * vte_terminal_set_color_background()
	 */
	vte_terminal_set_colors_rgba (term, foreground, background, palette, size);

	g_free (palette);
	g_object_unref (profiles_list);
	g_object_unref (profile_settings);
}

static void
preferences_changed (GSettings* settings, TerminalPlugin *term)
{
	terminal_set_preferences (VTE_TERMINAL (term->shell), settings, term);
	terminal_set_preferences (VTE_TERMINAL (term->term), settings, term);
}

static void
on_notify_prefs_profile(GSettings* settings,
                        const gchar* key,
                        gpointer user_data)
{
	TerminalPlugin *tp = ANJUTA_PLUGIN_TERMINAL (user_data);
	preferences_changed (settings, tp);
}

static void
on_notify_prefs_default (GSettings* settings,
                         const gchar* key,
                         gpointer user_data)
{
	TerminalPlugin *tp = ANJUTA_PLUGIN_TERMINAL (user_data);
	preferences_changed (settings, tp);
}

static void
prefs_init (TerminalPlugin *tp)
{
	g_signal_connect (tp->settings, "changed::" PREFS_TERMINAL_PROFILE,
	                  G_CALLBACK (on_notify_prefs_profile), tp);
	g_signal_connect (tp->settings, "changed::" PREFS_TERMINAL_PROFILE_USE_DEFAULT,
	                  G_CALLBACK (on_notify_prefs_default), tp);
}

static void
use_default_profile_cb (GtkToggleButton *button,
						TerminalPlugin *term)
{
	if (gtk_toggle_button_get_active (button))
		gtk_widget_set_sensitive (term->pref_profile_combo, FALSE);
	else
		gtk_widget_set_sensitive (term->pref_profile_combo, TRUE);
}

static void
terminal_child_exited_cb (VteTerminal *term, gpointer user_data)
{
	TerminalPlugin *term_plugin = ANJUTA_PLUGIN_TERMINAL (user_data);
	GPid pid = term_plugin->child_pid;
	int status;
	
	if (term_plugin->child_pid)
	{
		gboolean focus;

		focus = gtk_widget_is_focus (term_plugin->term);

		gtk_container_remove (GTK_CONTAINER (term_plugin->frame), term_plugin->term_box);
		gtk_container_add (GTK_CONTAINER (term_plugin->frame), term_plugin->shell_box);
		gtk_widget_show_all (term_plugin->shell_box);
		if (focus)
			gtk_widget_grab_focus (term_plugin->shell);
		term_plugin->child_pid = 0;
	}

	status = vte_terminal_get_child_exit_status (term);
	g_signal_emit_by_name(term_plugin, "child-exited", pid, status);
}

static pid_t
terminal_execute (TerminalPlugin *term_plugin, const gchar *directory,
				  const gchar *command, gchar **environment)
{
	char **args, **args_ptr;
	GList *args_list, *args_list_ptr;
	gchar *dir;
	VteTerminal *term;
	GPid pid;

	g_return_val_if_fail (command != NULL, 0);

	/* Prepare command args */
	args_list = anjuta_util_parse_args_from_string (command);
	args = g_new (char*, g_list_length (args_list) + 1);
	args_list_ptr = args_list;
	args_ptr = args;
	while (args_list_ptr)
	{
		*args_ptr = (char*) args_list_ptr->data;
		args_list_ptr = g_list_next (args_list_ptr);
		args_ptr++;
	}
	*args_ptr = NULL;

	if (directory == NULL)
		dir = g_path_get_dirname (args[0]);
	else
		dir = g_strdup (directory);

	term = VTE_TERMINAL (term_plugin->term);

/*
	vte_terminal_reset (term, TRUE, TRUE);
*/

	if (vte_terminal_fork_command_full (term, term_plugin->pty_flags,
	                                    dir, args, environment,
	                                    G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL,
	                                    &pid, NULL))
	{
		gboolean focus;

		term_plugin->child_pid = pid;

		/* Display terminal widget */
		focus = gtk_widget_is_focus (term_plugin->shell);
		gtk_container_remove (GTK_CONTAINER (term_plugin->frame), term_plugin->shell_box);
		gtk_container_add (GTK_CONTAINER (term_plugin->frame), term_plugin->term_box);
		gtk_widget_show_all (term_plugin->term_box);
		if (focus)
			gtk_widget_grab_focus (term_plugin->term);

		if (term_plugin->widget_added_to_shell)
			anjuta_shell_present_widget (ANJUTA_PLUGIN (term_plugin)->shell,
									 term_plugin->frame, NULL);
	}

	g_free (dir);
	g_free (args);
	g_list_foreach (args_list, (GFunc)g_free, NULL);
	g_list_free (args_list);

	return pid;
}

static void
init_shell (TerminalPlugin *term_plugin, const char *path)
{
	gchar *shell[2] = {0, 0};
	static gboolean first_time = TRUE;
	VteTerminal *term = VTE_TERMINAL (term_plugin->shell);

	shell[0] = vte_get_user_shell ();
	if (shell[0] == NULL) shell[0] = g_strdup("/bin/sh");

	if (!first_time)
		vte_terminal_reset (term, FALSE, TRUE);
	else
		first_time = FALSE;

	vte_terminal_fork_command_full (term, term_plugin->pty_flags,
	                                path, shell, NULL,
	                                0, NULL, NULL,
	                                NULL, NULL);
	g_free (shell[0]);
}

static gboolean
terminal_focus_cb (GtkWidget *widget, GdkEvent  *event,
				   TerminalPlugin *term)
{
	gtk_widget_grab_focus (widget);
	return FALSE;
}

static gboolean
terminal_keypress_cb (GtkWidget *widget, GdkEventKey  *event,
						   TerminalPlugin *terminal_plugin)
{
	if (event->type != GDK_KEY_PRESS)
		return FALSE;

	/* ctrl-d */
	if ((event->keyval == GDK_KEY_d || event->keyval == GDK_KEY_D) &&
		(event->state & GDK_CONTROL_MASK))
	{
		if (terminal_plugin->child_pid)
		{
			kill (terminal_plugin->child_pid, SIGINT);
			terminal_plugin->child_pid = 0;
		}
		return TRUE;
	}
	return FALSE;
}

static gboolean
terminal_click_cb (GtkWidget *widget, GdkEventButton *event,
				   TerminalPlugin *term)
{
	if (event->button == 3)
	{
		AnjutaUI *ui;
		GtkMenu *popup;
		GtkAction *action;

		ui = anjuta_shell_get_ui (ANJUTA_PLUGIN(term)->shell, NULL);
		popup =  GTK_MENU (gtk_ui_manager_get_widget (GTK_UI_MANAGER (ui), "/PopupTerminal"));
		action = gtk_action_group_get_action (term->action_group, "ActionCopyFromTerminal");
		gtk_action_set_sensitive (action,vte_terminal_get_has_selection(VTE_TERMINAL(widget)));

		gtk_menu_popup (popup, NULL, NULL, NULL, NULL,
						event->button, event->time);
	}

	return FALSE;
}

#if OLD_VTE == 1
/* VTE has a terrible bug where it could crash when container is changed.
 * The problem has been traced in vte where its style-set handler does not
 * adequately check if the widget is realized, resulting in a crash when
 * style-set occurs in an unrealized vte widget.
 *
 * This work around blocks all style-set signal emissions when the vte
 * widget is unrealized. -Naba
 */
static void
terminal_realize_cb (GtkWidget *term, TerminalPlugin *plugin)
{
	gint count;

	if (plugin->first_time_realization)
	{
		/* First time realize does not have the signals blocked */
		plugin->first_time_realization = FALSE;
		return;
	}
	count = g_signal_handlers_unblock_matched (term,
											   G_SIGNAL_MATCH_DATA,
											   0,
											   g_quark_from_string ("style-set"),
											   NULL,
											   NULL,
											   NULL);
	DEBUG_PRINT ("Unlocked %d terminal signal", count);
}

static void
terminal_unrealize_cb (GtkWidget *term, TerminalPlugin *plugin)
{
	gint count;
	count = g_signal_handlers_block_matched (term,
											 G_SIGNAL_MATCH_DATA,
											 0,
											 g_quark_from_string ("style-set"),
											 NULL,
											 NULL,
											 NULL);
	DEBUG_PRINT ("Blocked %d terminal signal", count);
}
#endif

static void
on_terminal_copy_cb (GtkAction * action, TerminalPlugin *term_plugin)
{
	VteTerminal *term;

	if (term_plugin->child_pid)
		term = VTE_TERMINAL (term_plugin->term);
	else
		term = VTE_TERMINAL (term_plugin->shell);

	if (vte_terminal_get_has_selection(term))
		vte_terminal_copy_clipboard(term);
}

static void
on_terminal_paste_cb (GtkAction * action, TerminalPlugin *term_plugin)
{
	VteTerminal *term;

	if (term_plugin->child_pid)
		term = VTE_TERMINAL (term_plugin->term);
	else
		term = VTE_TERMINAL (term_plugin->shell);

	vte_terminal_paste_clipboard(term);
}

static void
on_terminal_command_cb (GtkAction * action, TerminalPlugin *term_plugin)
{
	VteTerminal *term;
	gchar c;

	if (term_plugin->child_pid)
		term = VTE_TERMINAL (term_plugin->term);
	else
		term = VTE_TERMINAL (term_plugin->shell);

	/* this only works for control + letter */
	c = gtk_action_get_name (action) [strlen (gtk_action_get_name (action)) - 1] - 64;

	vte_terminal_feed_child (term, &c, 1);
}

static GtkActionEntry actions_terminal[] = {
	{
		"ActionCopyFromTerminal", 	              /* Action name */
		GTK_STOCK_COPY,                           /* Stock icon, if any */
		N_("_Copy"),		                      /* Display label */
		NULL,                                     /* short-cut */
		NULL,                                     /* Tooltip */
		G_CALLBACK (on_terminal_copy_cb)          /* action callback */
	},
	{
		"ActionPasteInTerminal",
		GTK_STOCK_PASTE,
		N_("_Paste"),
		NULL,
		NULL,
		G_CALLBACK (on_terminal_paste_cb)
	},
	/* Add other control + letter commands here ending in -CTRL and then letter e.g. -CTRLT */
	{
		"ActionCommandToTerminal-CTRLC",
		NULL,
		N_("Ctrl-C"),
		NULL,
		NULL,
		G_CALLBACK (on_terminal_command_cb)
	},
	{
		"ActionCommandToTerminal-CTRLX",
		NULL,
		N_("Ctrl-X"),
		NULL,
		NULL,
		G_CALLBACK (on_terminal_command_cb)
	},
	{
		"ActionCommandToTerminal-CTRLZ",
		NULL,
		N_("Ctrl-Z"),
		NULL,
		NULL,
		G_CALLBACK (on_terminal_command_cb)
	},
};

static void
on_project_root_added (AnjutaPlugin *plugin, const gchar *name,
					   const GValue *value, gpointer user_data)
{
	TerminalPlugin *term_plugin;
	const gchar *root_uri;

	term_plugin = (TerminalPlugin *)plugin;

	root_uri = g_value_get_string (value);
	if (root_uri)
	{
		GFile *file;
		char *path;

		file = g_file_new_for_uri (root_uri);
		path = g_file_get_path (file);

		init_shell (term_plugin, path);

		g_object_unref (file);
		g_free (path);
	}
}

static GtkWidget *
create_terminal (TerminalPlugin *term_plugin)
{
	GtkWidget *term;

	/* Create new terminal. */
	term = vte_terminal_new ();
	gtk_widget_set_size_request (GTK_WIDGET (term), 10, 10);
	vte_terminal_set_size (VTE_TERMINAL (term), 50, 1);

	g_signal_connect (G_OBJECT (term), "focus-in-event",
					  G_CALLBACK (terminal_focus_cb), term_plugin);

	g_signal_connect (G_OBJECT (term), "button-press-event",
					  G_CALLBACK (terminal_click_cb), term_plugin);

	g_signal_connect (G_OBJECT (term), "child-exited",
	                  G_CALLBACK (terminal_child_exited_cb), term_plugin);

#if OLD_VTE == 1
	g_signal_connect (G_OBJECT (term), "realize",
					  G_CALLBACK (terminal_realize_cb), term_plugin);
	g_signal_connect (G_OBJECT (term), "unrealize",
					  G_CALLBACK (terminal_unrealize_cb), term_plugin);
#endif

	return term;
}

static GtkWidget *
create_box (GtkWidget *term)
{
	GtkWidget *sb, *hbox;

	sb = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT (vte_terminal_get_adjustment (VTE_TERMINAL (term))));
	gtk_widget_set_can_focus (sb, FALSE);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (hbox), term, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), sb, FALSE, TRUE, 0);

	g_object_ref_sink (hbox);

	return hbox;
}

static void
terminal_create (TerminalPlugin *term_plugin)
{
	GtkWidget *frame;

	g_return_if_fail(term_plugin != NULL);

	term_plugin->child_pid = 0;

	/* Create the terminals. */
	term_plugin->shell = create_terminal (term_plugin);
	term_plugin->shell_box = create_box (term_plugin->shell);

	term_plugin->term = create_terminal (term_plugin);
	term_plugin->term_box = create_box (term_plugin->term);

	/* key-press handler for ctrl-d "kill" */
	g_signal_connect (G_OBJECT (term_plugin->term), "key-press-event",
					  G_CALLBACK (terminal_keypress_cb), term_plugin);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	gtk_container_add (GTK_CONTAINER (frame), term_plugin->shell_box);
	gtk_widget_show_all (frame);

	term_plugin->frame = frame;

	init_shell (term_plugin, NULL);
}

static void
register_stock_icons (AnjutaPlugin *plugin)
{
	static gboolean registered = FALSE;

	if (registered)
		return;
	registered = TRUE;

	BEGIN_REGISTER_ICON (plugin);
	REGISTER_ICON (ICON_FILE, "terminal-plugin-icon");
	END_REGISTER_ICON;
}

static gboolean
activate_plugin (AnjutaPlugin *plugin)
{
	TerminalPlugin *term_plugin;
	static gboolean initialized = FALSE;
	AnjutaUI *ui;

	DEBUG_PRINT ("%s", "TerminalPlugin: Activating Terminal plugin ...");

	term_plugin = ANJUTA_PLUGIN_TERMINAL (plugin);
	term_plugin->widget_added_to_shell = FALSE;
	ui = anjuta_shell_get_ui (plugin->shell, NULL);
	term_plugin->action_group = anjuta_ui_add_action_group_entries (ui,
										"ActionGroupTerminal",
										_("terminal operations"),
										actions_terminal,
										G_N_ELEMENTS (actions_terminal),
										GETTEXT_PACKAGE, TRUE, term_plugin);
	term_plugin->uiid = anjuta_ui_merge (ui, UI_FILE);

	terminal_create (term_plugin);

	if (!initialized)
	{
		register_stock_icons (plugin);
	}

	/* Setup prefs callbacks */
	prefs_init (term_plugin);

	/* Added widget in shell */
	anjuta_shell_add_widget (plugin->shell, term_plugin->frame,
							 "AnjutaTerminal", _("Terminal"),
							 "terminal-plugin-icon",
							 ANJUTA_SHELL_PLACEMENT_BOTTOM, NULL);
	/* terminal_focus_cb (term_plugin->term, NULL, term_plugin); */
	term_plugin->widget_added_to_shell = TRUE;
	initialized = TRUE;

	/* Set all terminal preferences, at that time the terminal widget is
	 * not realized, a few vte functions are not working. Another
	 * possibility could be to call this when the widget is realized */
	preferences_changed (term_plugin->settings, term_plugin);

	/* set up project directory watch */
	term_plugin->root_watch_id = anjuta_plugin_add_watch (plugin,
														  IANJUTA_PROJECT_MANAGER_PROJECT_ROOT_URI,
														  on_project_root_added,
														  0, NULL);

	return TRUE;
}

static gboolean
deactivate_plugin (AnjutaPlugin *plugin)
{
	TerminalPlugin *term_plugin;
	AnjutaUI *ui;

	term_plugin = ANJUTA_PLUGIN_TERMINAL (plugin);

	ui = anjuta_shell_get_ui (plugin->shell, NULL);
	anjuta_ui_unmerge (ui, term_plugin->uiid);
	if (term_plugin->action_group)
	{
		anjuta_ui_remove_action_group (ui, term_plugin->action_group);
		term_plugin->action_group = NULL;
	}

	/* terminal plugin widgets are destroyed as soon as it is removed */
	anjuta_shell_remove_widget (plugin->shell, term_plugin->frame, NULL);

	g_object_unref (term_plugin->shell_box);
	g_object_unref (term_plugin->term_box);

	/* remove watch */
	anjuta_plugin_remove_watch (plugin, term_plugin->root_watch_id, FALSE);

	term_plugin->child_pid = 0;

#if OLD_VTE == 1
	g_signal_handlers_disconnect_by_func (G_OBJECT (term_plugin->term),
			 G_CALLBACK (terminal_unrealize_cb), term_plugin);

	term_plugin->first_time_realization = TRUE;
#endif

	return TRUE;
}

static void
terminal_plugin_dispose (GObject *obj)
{
	TerminalPlugin *term_plugin = ANJUTA_PLUGIN_TERMINAL (obj);

	g_object_unref (term_plugin->settings);

	G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
terminal_plugin_finalize (GObject *obj)
{
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
terminal_plugin_instance_init (GObject *obj)
{
	TerminalPlugin *term_plugin = ANJUTA_PLUGIN_TERMINAL (obj);

	term_plugin->settings = g_settings_new (PREF_SCHEMA);
	term_plugin->child_pid = 0;
	term_plugin->pref_profile_combo = NULL;
	term_plugin->uiid = 0;
	term_plugin->action_group = NULL;
	term_plugin->pty_flags = VTE_PTY_DEFAULT;
#if OLD_VTE == 1
	plugin->first_time_realization = TRUE;
#endif
}

static void
terminal_plugin_class_init (GObjectClass *klass)
{
	AnjutaPluginClass *plugin_class = ANJUTA_PLUGIN_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	plugin_class->activate = activate_plugin;
	plugin_class->deactivate = deactivate_plugin;
	klass->dispose = terminal_plugin_dispose;
	klass->finalize = terminal_plugin_finalize;
}

static pid_t
iterminal_execute_command (IAnjutaTerminal *terminal,
						   const gchar *directory,
						   const gchar *command,
						   gchar **environment, GError **error)
{
	TerminalPlugin *plugin;
	pid_t pid;

	plugin = ANJUTA_PLUGIN_TERMINAL (terminal);

	pid = terminal_execute (plugin, directory, command, environment);
	if (pid <= 0)
	{
		g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED, _("Unable to execute command"));
	}

	return pid;
}

static void
iterminal_iface_init(IAnjutaTerminalIface *iface)
{
	iface->execute_command = iterminal_execute_command;
}

static void
add_data_to_store (gchar *uuid, gchar *name, GtkListStore *model)
{
	GtkTreeIter iter;

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter, TERM_STORE_UUID_COLUMN, uuid, TERM_STORE_NAME_COLUMN, name, -1);
}

static void
on_pref_profile_changed (GtkComboBox* combo, TerminalPlugin* term_plugin)
{
	GtkTreeModel *model = gtk_combo_box_get_model (combo);
	GtkTreeIter iter;
	gchar *uuid;

	gtk_combo_box_get_active_iter (combo, &iter);
	gtk_tree_model_get (model, &iter, TERM_STORE_UUID_COLUMN, &uuid, -1);
	g_settings_set_string (term_plugin->settings, PREFS_TERMINAL_PROFILE, uuid);
	g_free (uuid);
}

static void
ipreferences_merge(IAnjutaPreferences* ipref, AnjutaPreferences* prefs, GError** e)
{
	GSettingsSchemaSource *source;
	GSettingsSchema *schema;
	GSettings *terminal_settings;
	GSettings *profile_settings;
	GtkListStore *store;
	GtkTreeIter iter;
	gchar *default_uuid = NULL;
	gchar *saved_uuid;
	gchar *temp;
	gchar **profiles;
	gchar *path;
	gboolean iter_valid;
	gboolean found;
	int i;
	GError *error = NULL;
	GtkCellRenderer *name_renderer;
	GtkCellRenderer *uuid_renderer;

	/* Create the terminal preferences page */
	TerminalPlugin *term_plugin = ANJUTA_PLUGIN_TERMINAL (ipref);
	GtkBuilder *bxml = gtk_builder_new ();

	if (!gtk_builder_add_from_file (bxml, PREFS_BUILDER, &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}


	anjuta_preferences_add_from_builder (prefs, bxml,
	                                     term_plugin->settings,
	                                     "Terminal", _("Terminal"), ICON_FILE);

	term_plugin->pref_profile_combo = GTK_WIDGET (gtk_builder_get_object (bxml, "profile_list_combo"));
	term_plugin->pref_default_button = GTK_WIDGET (gtk_builder_get_object (bxml, "preferences_toggle:bool:1:0:use-default-profile"));

	/* Add profile-name and profile-uuid renderers to combo box */
	name_renderer = gtk_cell_renderer_text_new ();
	uuid_renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_clear (GTK_CELL_LAYOUT(term_plugin->pref_profile_combo));
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(term_plugin->pref_profile_combo), name_renderer, TRUE);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(term_plugin->pref_profile_combo), uuid_renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(term_plugin->pref_profile_combo), name_renderer, "text", TERM_STORE_NAME_COLUMN);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(term_plugin->pref_profile_combo), uuid_renderer, "text", TERM_STORE_UUID_COLUMN);
	g_object_set (uuid_renderer, "style", PANGO_STYLE_ITALIC, NULL);

	/* If at least a default profile exists ... */
	source = g_settings_schema_source_get_default ();
	schema = g_settings_schema_source_lookup (source, TERMINAL_PROFILES_LIST_SCHEMA, TRUE);
	if (schema != NULL)
	{
		terminal_settings = g_settings_new_full (schema, NULL, NULL);
		default_uuid = g_settings_get_string (terminal_settings, "default");
		g_settings_schema_unref (schema);
	}

	if (default_uuid != NULL) {

		/* Populate profiles store */
		profiles = g_settings_get_strv (terminal_settings, "list");
		store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (term_plugin->pref_profile_combo)));
		gtk_list_store_clear (store);
		for (i = 0; profiles[i] != NULL; i ++) {
			path = g_strdup_printf ("%s:%s/", TERMINAL_PROFILES_PATH_PREFIX, profiles[i]);
			profile_settings = g_settings_new_with_path (TERMINAL_PROFILE_SCHEMA, path);
			temp = g_settings_get_string (profile_settings, TERMINAL_PROFILE_VISIBLE_NAME_KEY);
			add_data_to_store (profiles[i], temp, store);
		}

		/* Display saved profile in combo box */
		saved_uuid = g_settings_get_string (term_plugin->settings, PREFS_TERMINAL_PROFILE);
		if (saved_uuid != NULL)
		{
			found = FALSE;
			iter_valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
			while (iter_valid && !found)
			{
				gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, TERM_STORE_UUID_COLUMN, &temp, -1);
				if (g_strcmp0 (saved_uuid, temp) == 0)
				{
					gtk_combo_box_set_active_iter (GTK_COMBO_BOX (term_plugin->pref_profile_combo), &iter);
					found = TRUE;
				}
				iter_valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);
				g_free (temp);
			}
			g_free (saved_uuid);
		}

		/* Save profile selection if changed */
		g_signal_connect (term_plugin->pref_profile_combo, "changed",
		                  G_CALLBACK (on_pref_profile_changed), term_plugin);

		/* Deactivate profile selection if not using default profile */
		use_default_profile_cb (GTK_TOGGLE_BUTTON (term_plugin->pref_default_button), term_plugin);
		g_signal_connect (G_OBJECT(term_plugin->pref_default_button), "toggled",
						  G_CALLBACK (use_default_profile_cb), term_plugin);

		g_object_unref(terminal_settings);
	}
	else
	{
		/* No profile, perhaps GNOME Terminal is not installed,
		 * Remove selection */
		gtk_widget_set_sensitive (term_plugin->pref_profile_combo, FALSE);
		gtk_widget_set_sensitive (term_plugin->pref_default_button, FALSE);
	}

	g_object_unref (bxml);
}

static void
ipreferences_unmerge(IAnjutaPreferences* ipref, AnjutaPreferences* prefs, GError** e)
{
	TerminalPlugin* term_plugin = ANJUTA_PLUGIN_TERMINAL (ipref);
	g_signal_handlers_disconnect_by_func(G_OBJECT(term_plugin->pref_default_button),
		G_CALLBACK (use_default_profile_cb), term_plugin);
	anjuta_preferences_remove_page(prefs, _("Terminal"));
	term_plugin->pref_profile_combo = NULL;
}

static void
ipreferences_iface_init(IAnjutaPreferencesIface* iface)
{
	iface->merge = ipreferences_merge;
	iface->unmerge = ipreferences_unmerge;
}

ANJUTA_PLUGIN_BEGIN (TerminalPlugin, terminal_plugin);
ANJUTA_PLUGIN_ADD_INTERFACE (iterminal, IANJUTA_TYPE_TERMINAL);
ANJUTA_PLUGIN_ADD_INTERFACE (ipreferences, IANJUTA_TYPE_PREFERENCES);
ANJUTA_PLUGIN_END;

ANJUTA_SIMPLE_PLUGIN (TerminalPlugin, terminal_plugin);
