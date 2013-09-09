/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta
 * Copyright (C) James Liggett 2010 <jrliggett@cox.net>
 * 
 * anjuta is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * anjuta is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "git-stash-pane.h"

enum
{
	COL_NUMBER,
	COL_MESSAGE,
	COL_ID,
	COL_DIFF
};

struct _GitStashPanePriv
{
	GtkBuilder *builder;
};

G_DEFINE_TYPE (GitStashPane, git_stash_pane, GIT_TYPE_PANE);

static void
on_stash_list_command_started (AnjutaCommand *command, GitStashPane *self)
{
	GtkTreeView *stash_view;
	GtkTreeStore *stash_model;

	stash_view = GTK_TREE_VIEW (gtk_builder_get_object (self->priv->builder,
	                                                    "stash_view"));
	stash_model = GTK_TREE_STORE (gtk_builder_get_object (self->priv->builder,
	                                                      "stash_model"));

	gtk_tree_view_set_model (stash_view, NULL);
	gtk_tree_store_clear (stash_model);
}

static void
on_stash_list_command_finished (AnjutaCommand *command, guint return_code,
                                GitStashPane *self)
{
	GtkTreeView *stash_view;
	GtkTreeModel *stash_model;

	stash_view = GTK_TREE_VIEW (gtk_builder_get_object (self->priv->builder,
	                                                    "stash_view"));
	stash_model = GTK_TREE_MODEL (gtk_builder_get_object (self->priv->builder,
	                                                      "stash_model"));

	gtk_tree_view_set_model (stash_view, stash_model);
}

static void
on_stash_diff_command_finished (AnjutaCommand *command, guint return_code,
                                GtkTreeStore *stash_model)
{
	GtkTreePath *parent_path;
	GtkTreeIter parent_iter;
	GtkTreeIter iter;
	GQueue *output;
	gchar *output_line;

	if (return_code == 0)
	{
		parent_path = g_object_get_data (G_OBJECT (command), "parent-path");
		gtk_tree_model_get_iter (GTK_TREE_MODEL (stash_model), &parent_iter,
		                         parent_path);

		output = git_raw_output_command_get_output (GIT_RAW_OUTPUT_COMMAND (command));

		while (g_queue_peek_head (output))
		{
			output_line = g_queue_pop_head (output);

			gtk_tree_store_append (stash_model, &iter, &parent_iter);
			gtk_tree_store_set (stash_model, &iter,
			                    COL_DIFF, output_line,
			                    -1);

			g_free (output_line);
		}
	}
}

static void
on_stash_list_command_data_arrived (AnjutaCommand *command, 
                                    GtkTreeStore *stash_model)
{
	GQueue *output;
	GtkTreeIter iter;
	GitStash *stash;
	guint number;
	gchar *message;
	gchar *id;
	gchar *working_directory;
	GitStashShowCommand *show_command;
	
	output = git_stash_list_command_get_output (GIT_STASH_LIST_COMMAND (command));

	while (g_queue_peek_head (output))
	{
		stash = g_queue_pop_head (output);
		number = git_stash_get_number (stash);
		message = git_stash_get_message (stash);
		id = git_stash_get_id (stash);


		gtk_tree_store_append (stash_model, &iter, NULL);
		gtk_tree_store_set (stash_model, &iter, 
		                    COL_NUMBER, number,
		                    COL_MESSAGE, message,
		                    COL_ID, id,
		                    -1);

		g_object_get (G_OBJECT (command), "working-directory", 
		              &working_directory, NULL);
		show_command = git_stash_show_command_new (working_directory, id);

		g_free (working_directory);

		g_object_set_data_full (G_OBJECT (show_command), "parent-path", 
		               			gtk_tree_model_get_path (GTK_TREE_MODEL (stash_model),
		                                        		 &iter),
		               			(GDestroyNotify) gtk_tree_path_free);

		g_signal_connect (G_OBJECT (show_command), "command-finished",
		                  G_CALLBACK (on_stash_diff_command_finished),
		                  stash_model);

		g_signal_connect (G_OBJECT (show_command), "command-finished",
		                  G_CALLBACK (g_object_unref),
		                  NULL);

		anjuta_command_start (ANJUTA_COMMAND (show_command));

		g_object_unref (stash);
		g_free (message);
		g_free (id);
	}
}

static gboolean
on_stash_view_button_press_event (GtkWidget *stash_view, GdkEventButton *event,
                                  GitStashPane *self)
{
	gboolean path_valid;
	GtkTreePath *path;
	gboolean ret = FALSE;

	path_valid = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (stash_view), 
	                                            event->x, event->y, &path, NULL, 
	                                            NULL, NULL);

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		if (path_valid && gtk_tree_path_get_depth (path) == 1)
		{
			git_pane_popup_menu (GIT_PANE (self), "GitStashPopup", event->button,
			                     event->time);
		}
	}

	/* Don't forward button presses to diff renderers */
	if (path_valid)
	{
		ret = gtk_tree_path_get_depth (path) == 2;
		gtk_tree_path_free (path);
	}

	return ret;
}

static void
stash_message_renderer_data_func (GtkTreeViewColumn *tree_column,
                                  GtkCellRenderer *renderer,
                                  GtkTreeModel *model,
                                  GtkTreeIter *iter,
                                  gpointer user_data)
{
	gboolean visible;
	gchar *message;

	/* Don't show this column on diffs */
	visible = gtk_tree_store_iter_depth (GTK_TREE_STORE (model), iter) == 0;
	gtk_cell_renderer_set_visible (renderer, visible);

	if (visible)
	{
		gtk_tree_model_get (model, iter, COL_MESSAGE, &message, -1);
		g_object_set (renderer, "text", message, NULL);

		g_free (message);
	}
	else
		g_object_set (renderer, "text", "", NULL);
}

static void
stash_number_renderer_data_func (GtkTreeViewColumn *tree_column,
                                 GtkCellRenderer *renderer,
                                 GtkTreeModel *model,
                                 GtkTreeIter *iter,
                                 gpointer user_data)
{
	gboolean visible;
	guint number;
	gchar *number_string;

	/* Don't show this column on diffs */
	visible = gtk_tree_store_iter_depth (GTK_TREE_STORE (model), iter) == 0;
	gtk_cell_renderer_set_visible (renderer, visible);

	if (visible)
	{
		gtk_tree_model_get (model, iter, COL_NUMBER, &number, -1);
		number_string = g_strdup_printf ("%i", number);

		g_object_set (renderer, "text", number_string, NULL);
		g_free (number_string);
	}
	else
		g_object_set (renderer, "text", "", NULL);
}

static gboolean
on_stash_view_row_selected (GtkTreeSelection *selection,
                            GtkTreeModel *model,
                            GtkTreePath *path,
                            gboolean path_currently_selected,
                            gpointer user_data)
{
	return gtk_tree_path_get_depth (path) == 1;
}

static void
git_stash_pane_init (GitStashPane *self)
{
	gchar *objects[] = {"stash_pane",
						"stash_model",
						NULL};
	GError *error = NULL;
	GtkTreeView *stash_view;
	GtkTreeViewColumn *stash_number_column;
	GtkCellRenderer *stash_number_renderer;
	GtkTreeViewColumn *stash_message_column;
	GtkCellRenderer *stash_message_renderer;
	GtkCellRenderer *diff_renderer;
	GtkTreeSelection *selection;
	
	self->priv = g_new0 (GitStashPanePriv, 1);
	self->priv->builder = gtk_builder_new ();

	if (!gtk_builder_add_objects_from_file (self->priv->builder, BUILDER_FILE, 
	                                        objects, 
	                                        &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	stash_view = GTK_TREE_VIEW (gtk_builder_get_object (self->priv->builder,
	                                             	    "stash_view"));
	stash_number_column = GTK_TREE_VIEW_COLUMN (gtk_builder_get_object (self->priv->builder,
	                                                                    "stash_number_column"));
	stash_number_renderer = GTK_CELL_RENDERER (gtk_builder_get_object (self->priv->builder,
	                                                                   "stash_number_renderer"));
	stash_message_column = GTK_TREE_VIEW_COLUMN (gtk_builder_get_object (self->priv->builder,
	                                                                     "stash_message_column"));
	stash_message_renderer = GTK_CELL_RENDERER (gtk_builder_get_object (self->priv->builder,
	                                                                    "stash_message_renderer"));
	diff_renderer = anjuta_cell_renderer_diff_new ();
	selection = gtk_tree_view_get_selection (stash_view);

	gtk_tree_view_column_set_cell_data_func (stash_number_column, stash_number_renderer,
	                                         stash_number_renderer_data_func, 
	                                         NULL, NULL);
	gtk_tree_view_column_set_cell_data_func (stash_message_column, stash_message_renderer,
	                                         stash_message_renderer_data_func,
	                                         NULL, NULL);

	gtk_tree_view_column_pack_start (stash_message_column, diff_renderer, TRUE);
	gtk_tree_view_column_add_attribute (stash_message_column, diff_renderer,
	                                    "diff", COL_DIFF);

	/* Don't allow diffs to be selected */
	gtk_tree_selection_set_select_function (selection, on_stash_view_row_selected,
	                                        NULL, NULL);

	g_signal_connect (G_OBJECT (stash_view), "button-press-event",
	                  G_CALLBACK (on_stash_view_button_press_event),
	                  self);
}

static void
git_stash_pane_finalize (GObject *object)
{
	GitStashPane *self;

	self = GIT_STASH_PANE (object);

	g_object_unref (self->priv->builder);
	g_free (self->priv);

	G_OBJECT_CLASS (git_stash_pane_parent_class)->finalize (object);
}

static GtkWidget *
git_stash_pane_get_widget (AnjutaDockPane *pane)
{
	GitStashPane *self;

	self = GIT_STASH_PANE (pane);

	return GTK_WIDGET (gtk_builder_get_object (self->priv->builder,
	                                           "stash_pane"));
}

static void
git_stash_pane_class_init (GitStashPaneClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	AnjutaDockPaneClass *pane_class = ANJUTA_DOCK_PANE_CLASS (klass);

	object_class->finalize = git_stash_pane_finalize;
	pane_class->get_widget = git_stash_pane_get_widget;
	pane_class->refresh = NULL;
}


AnjutaDockPane *
git_stash_pane_new (Git *plugin)
{
	GitStashPane *self;
	GtkTreeStore *stash_model;

	self = g_object_new (GIT_TYPE_STASH_PANE, "plugin", plugin, NULL);
	stash_model = GTK_TREE_STORE (gtk_builder_get_object (self->priv->builder,
	                                                      "stash_model"));

	g_signal_connect (G_OBJECT (plugin->stash_list_command), "command-started",
	                  G_CALLBACK (on_stash_list_command_started),
	                  self);

	g_signal_connect (G_OBJECT (plugin->stash_list_command), "command-finished",
	                  G_CALLBACK (on_stash_list_command_finished),
	                  self);

	g_signal_connect (G_OBJECT (plugin->stash_list_command), "data-arrived",
	                  G_CALLBACK (on_stash_list_command_data_arrived),
	                  stash_model);

	return ANJUTA_DOCK_PANE (self);
}

gchar *
git_stash_pane_get_selected_stash_id (GitStashPane *self)
{
	GtkTreeView *stash_view;
	GtkTreeSelection *selection;
	gchar *id;
	GtkTreeModel *stash_model;
	GtkTreeIter iter;
	                                   
	stash_view = GTK_TREE_VIEW (gtk_builder_get_object (self->priv->builder,
	                                                    "stash_view"));
	selection = gtk_tree_view_get_selection (stash_view);
	id = NULL;

	if (gtk_tree_selection_get_selected (selection, &stash_model, &iter))
		gtk_tree_model_get (stash_model, &iter, COL_ID, &id, -1);

	return id;
}

gint
git_stash_pane_get_selected_stash_number (GitStashPane *self)
{
	GtkTreeView *stash_view;
	GtkTreeSelection *selection;
	guint number;
	GtkTreeModel *stash_model;
	GtkTreeIter iter;

	stash_view = GTK_TREE_VIEW (gtk_builder_get_object (self->priv->builder,
	                                                    "stash_view"));
	selection = gtk_tree_view_get_selection (stash_view);
	number = -1;

	if (gtk_tree_selection_get_selected (selection, &stash_model, &iter))
		gtk_tree_model_get (stash_model, &iter, COL_NUMBER, &number, -1);

	return number;
}
