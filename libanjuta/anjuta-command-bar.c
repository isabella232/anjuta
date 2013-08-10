/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git-shell-test
 * Copyright (C) James Liggett 2009 <jrliggett@cox.net>
 * 
 * git-shell-test is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * git-shell-test is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "anjuta-command-bar.h"

/* Reasonable default */
#define DEFAULT_MAX_TEXT_WIDTH 25

/**
 * SECTION: anjuta-command-bar
 * @short_description: Widget that lays out commands in a vertical row of 
 *					   buttons and frames.
 * @see_also: #AnjutaDock, #GtkAction
 * @include: libanjuta/anjuta-command-bar.h
 *
 * AnjutaCommandBar provides a convenient way to arrange several sets of 
 * commands into one widget. It separates commands into different groups of 
 * actions, with only one group visible at a time. 
 */

G_DEFINE_TYPE (AnjutaCommandBar, anjuta_command_bar, GTK_TYPE_NOTEBOOK);

struct _AnjutaCommandBarPriv
{
	GHashTable *action_groups;
	GHashTable *widgets;

	gint max_text_width;
};

enum
{
	PROP_0,
	PROP_MAX_TEXT_WIDTH,
	PROP_LAST
};

static void
anjuta_command_bar_init (AnjutaCommandBar *self)
{
	self->priv = g_new0 (AnjutaCommandBarPriv, 1);

	gtk_notebook_set_show_border (GTK_NOTEBOOK (self), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (self), FALSE);
	
	/* The action groups table contains the GtkActionGroup objects that 
	 * correspond to each page of the action bar. The widgets table contain's
	 * each group's set of buttons and frames. */
	self->priv->action_groups = g_hash_table_new_full (g_str_hash, g_str_equal,
	                                                   NULL, g_object_unref);
	self->priv->widgets = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
anjuta_command_bar_finalize (GObject *object)
{
	AnjutaCommandBar *self;

	self = ANJUTA_COMMAND_BAR (object);

	g_hash_table_destroy (self->priv->action_groups);
	g_hash_table_destroy (self->priv->widgets);

	G_OBJECT_CLASS (anjuta_command_bar_parent_class)->finalize (object);
}

static void
anjuta_command_bar_get_property (GObject* object, guint prop_id, GValue* value,
                                 GParamSpec* pspec)
{
    AnjutaCommandBar* self = ANJUTA_COMMAND_BAR (object);

    switch (prop_id)
    {
        case PROP_MAX_TEXT_WIDTH:
            g_value_set_int (value,
                             self->priv->max_text_width);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
anjuta_command_bar_set_property (GObject* object, guint prop_id,
                                const GValue* value, GParamSpec* pspec)
{
    AnjutaCommandBar* self = ANJUTA_COMMAND_BAR (object);

    switch (prop_id)
    {
        case PROP_MAX_TEXT_WIDTH:
            self->priv->max_text_width = g_value_get_int (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
anjuta_command_bar_class_init (AnjutaCommandBarClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = anjuta_command_bar_finalize;
	object_class->set_property = anjuta_command_bar_set_property;
	object_class->get_property = anjuta_command_bar_get_property;
	
	g_object_class_install_property (object_class,
	                                 PROP_MAX_TEXT_WIDTH,
	                                 g_param_spec_int ("max-text-width",
	                                                   "",
	                                                   "",
	                                                   10,
	                                                   1000,
	                                                   DEFAULT_MAX_TEXT_WIDTH,
	                                                   G_PARAM_WRITABLE|G_PARAM_READABLE|G_PARAM_CONSTRUCT_ONLY));
}

/**
 * anjuta_command_bar_new:
 *
 * Creates a new AnjutaCommandBar.
 * Returns: A new AnjutaCommandBar
 */
GtkWidget *
anjuta_command_bar_new (void)
{
	return g_object_new (ANJUTA_TYPE_COMMAND_BAR, NULL);
}

/**
 * anjuta_command_bar_add_action_group:
 * @self: An AnjutaCommandBar
 * @group_name: A unique name for this group of entries
 * @entries: (array length=num_entries): A list of entries to add
 * @num_entries: The number of items pointed to by entries
 * @user_data: User data to pass to the entry callback
 * 
 * Adds a group of entries to an AnjutaCommandBar.
 */
void 
anjuta_command_bar_add_action_group (AnjutaCommandBar *self, 
                                     const gchar *group_name, 
                                     const AnjutaCommandBarEntry *entries,
                                     int num_entries, gpointer user_data)
{
	GtkWidget *vbox;
	GtkWidget *scrolled_window;
	GtkWidget *current_vbox;
	GtkActionGroup *action_group;
	int i;
	
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
	                                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	g_hash_table_insert (self->priv->widgets, (gchar *) group_name, 
	                     scrolled_window);

	action_group = gtk_action_group_new (group_name);

	g_hash_table_insert (self->priv->action_groups, (gchar *) group_name,
	                     action_group);

	/* The current_vbox is the vbox we're currently adding buttons to. As 
	 * frame entries are encountered, the current box changes to the newly 
	 * created frame vbox. But start by adding any other buttons to the top
	 * level vbox. */
	current_vbox = vbox;

	for (i = 0; i < num_entries; i++)
	{
		if (entries[i].type == ANJUTA_COMMAND_BAR_ENTRY_BUTTON)
		{
			GtkAction *action;
			GtkWidget *button;
			GtkWidget *button_label;
			
			action = gtk_action_new (entries[i].action_name, _(entries[i].label), 
			                         _(entries[i].tooltip), entries[i].stock_icon);
			button = gtk_button_new_with_label (_(entries[i].label));
			

			gtk_action_group_add_action (action_group, action);
			
			gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
			button_label = gtk_bin_get_child (GTK_BIN (button));
			gtk_label_set_width_chars (GTK_LABEL (button_label), self->priv->max_text_width);
			gtk_label_set_line_wrap (GTK_LABEL (button_label), TRUE);

			/* Left-align button contents */
			gtk_misc_set_alignment (GTK_MISC (button_label), 0.0, 0.5);
			g_object_set (G_OBJECT (button), "xalign", 0.0, NULL);

			if (entries[i].stock_icon)
			{
				GtkWidget *button_image;

				button_image = gtk_action_create_icon (action, 
				                                       GTK_ICON_SIZE_BUTTON);
				gtk_button_set_image (GTK_BUTTON (button), button_image);
			}

			gtk_activatable_set_related_action (GTK_ACTIVATABLE (button), 
			                                    action);
			gtk_widget_show_all (button);
			

			g_signal_connect (G_OBJECT (action), "activate",
			                  entries[i].callback,
			                  user_data);


			gtk_box_pack_start (GTK_BOX (current_vbox), button, FALSE, FALSE, 
			                    2);
		}
		else
		{
			gchar *frame_label_text;
			GtkWidget *frame_label;
			GtkWidget *frame;
			GtkWidget *frame_vbox;
			
			frame_label_text = g_strdup_printf ("<b>%s</b>", _(entries[i].label));
			frame_label = gtk_label_new (NULL);
			frame = gtk_frame_new (NULL);
			
			gtk_label_set_markup(GTK_LABEL (frame_label), frame_label_text);
			gtk_frame_set_label_widget (GTK_FRAME (frame), frame_label);

			g_free (frame_label_text);
			
			frame_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

			g_object_set (G_OBJECT (frame), "shadow-type", GTK_SHADOW_NONE, 
			              NULL);

			gtk_container_add (GTK_CONTAINER (frame), frame_vbox);
			gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 2);

			current_vbox = frame_vbox;
		}
	}

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
	                                       vbox);
	gtk_widget_show_all (scrolled_window);
	gtk_notebook_append_page (GTK_NOTEBOOK (self), scrolled_window, NULL);
}

/**
 * anjuta_command_bar_remove_action_group:
 * @self: An AnjutaCommandBar
 * @group_name: Name of the action group to remove
 *
 * Removes an action group from an AnjutaCommandBar.
 */
void 
anjuta_command_bar_remove_action_group (AnjutaCommandBar *self,
                                        const gchar *group_name)
{
	GtkWidget *page_widget;
	int page_num;

	page_widget = g_hash_table_lookup (self->priv->action_groups, group_name);

	if (page_widget)
	{
		page_num = gtk_notebook_page_num (GTK_NOTEBOOK (self), page_widget);

		gtk_notebook_remove_page (GTK_NOTEBOOK (self), page_num);

		g_hash_table_remove (self->priv->action_groups, group_name);
		g_hash_table_remove (self->priv->widgets, group_name);
	}
	else
		g_warning ("Action group %s not found.", group_name);
	
}

/**
 * anjuta_command_bar_show_action_group:
 * @self: An AnjutaCommandBar
 * @group_name: The name of the action group to show
 * 
 * Causes the actions in the given group to become visible, replacing the 
 * previously visible group.
 */
void
anjuta_command_bar_show_action_group (AnjutaCommandBar *self,
                                      const gchar *group_name)
{
	GtkWidget *page_widget;
	int page_num;

	page_widget = g_hash_table_lookup (self->priv->widgets, group_name);

	if (page_widget)
	{
		page_num = gtk_notebook_page_num (GTK_NOTEBOOK (self), page_widget);

		gtk_notebook_set_current_page (GTK_NOTEBOOK (self), page_num);
	}
	else
		g_warning ("Action group %s not found.", group_name);
	
}

/**
 * anjuta_command_bar_get_action_group:
 * @self An AnjutaCommandBar
 * @group_name: The name of the action group
 *
 * Returns: The #GtkActionGroup with the given @group_name
 */
GtkActionGroup *
anjuta_command_bar_get_action_group (AnjutaCommandBar *self,
                                     const gchar *group_name)
{
	GtkActionGroup *action_group;

	action_group = g_hash_table_lookup (self->priv->action_groups, group_name);

	if (!action_group)
		g_warning ("Action group %s not found.", group_name);

	return action_group;
}

/**
 * anjuta_command_bar_get_action:
 * @self: An AnjutaCommandBar
 * @group_name: The name of the #GtkActionGroup to look for the action in
 * @action_name: The name of the action
 *
 * Retrieves a #GtkAction object in the given group with the given name
 */
GtkAction *
anjuta_command_bar_get_action (AnjutaCommandBar *self, const gchar *group_name, 
                               const gchar *action_name)
{
	GtkActionGroup *action_group;
	GtkAction *action;

	action = NULL;

	action_group = anjuta_command_bar_get_action_group (self, group_name);

	if (action_group)
		action = gtk_action_group_get_action (action_group, action_name);

	return action;
}
