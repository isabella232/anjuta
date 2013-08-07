/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-index-refreshable.c
 * Copyright (C) 2013 James Liggett <jrliggett@cox.net>
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

#include "git-index-refreshable.h"

struct _GitIndexRefreshablePrivate
{
	Git *plugin;
	GFileMonitor *index_monitor;
	GFileMonitor *head_monitor;
	GitCommand *refresh_command;
	gboolean index_refreshed;
};

static void
git_index_refreshable_notify_refreshed (GitIndexRefreshable *self)
{
	g_signal_emit_by_name (self, "refreshed", NULL);
}

static void
on_refresh_command_finished (AnjutaTask *task, GitIndexRefreshable *self)
{
	g_clear_object (&self->priv->refresh_command);
	git_index_refreshable_notify_refreshed (self);
	self->priv->index_refreshed = TRUE;
}

static void
on_index_monitor_changed (GFileMonitor *monitor, GFile *file, GFile *other_file,
                          GFileMonitorEvent event, GitIndexRefreshable *self)
{
	/* Handle created and modified events just to cover all possible cases. 
	 * Sometimes git does some odd things... */
	if (event == G_FILE_MONITOR_EVENT_CHANGED ||
	    event == G_FILE_MONITOR_EVENT_CREATED)
	{
		if (!self->priv->refresh_command)
		{
			self->priv->refresh_command = git_refresh_index_command_new ();

			g_signal_connect (G_OBJECT (self->priv->refresh_command), "finished",
			                  G_CALLBACK (on_refresh_command_finished),
			                  self);

			git_thread_pool_push (self->priv->plugin->thread_pool, 
			                      self->priv->refresh_command);
		}
	}
}

static void
on_head_monitor_changed (GFileMonitor *monitor, GFile *file, GFile *other_file,
                         GFileMonitorEvent event, GitIndexRefreshable *self)
{
	if (event == G_FILE_MONITOR_EVENT_CHANGED ||
	    event == G_FILE_MONITOR_EVENT_CREATED)
	{
		/* Don't refresh the status again if we've already refreshed due to inedex
		 * changes. */
		if (!self->priv->index_refreshed)
			git_index_refreshable_notify_refreshed (self);

		self->priv->index_refreshed = FALSE;
	}
}

static void 
git_index_refreshable_start_monitor (IAnjutaRefreshable *obj, GError **err)
{
	GitIndexRefreshable *self;
	const gchar *working_directory;
	gchar *git_index_path;
	gchar *git_head_path;
	GFile *git_index_file;
	GFile *git_head_file;

	self = GIT_INDEX_REFRESHABLE (obj);
	working_directory = self->priv->plugin->project_root_directory;

	/* Watch for changes to the HEAD file and the index file, so that we can
	 * at least detect commits and index changes. */
	git_index_path = g_strjoin (G_DIR_SEPARATOR_S,
	                            working_directory,
	                            ".git",
	                            "index",
	                            NULL);
	git_head_path = g_strjoin (G_DIR_SEPARATOR_S,
	                           working_directory,
	                           ".git",
	                           "HEAD",
	                           NULL);
	git_index_file = g_file_new_for_path (git_index_path);
	git_head_file = g_file_new_for_path (git_head_path);

	self->priv->index_monitor = g_file_monitor_file (git_index_file, 0, NULL,
	                                                 err);

	if (self->priv->index_monitor)
	{
		self->priv->head_monitor = g_file_monitor_file (git_head_file, 0, NULL, 
		                                                err);

		if (self->priv->head_monitor)
		{
			g_signal_connect (G_OBJECT (self->priv->index_monitor), "changed",
			                  G_CALLBACK (on_index_monitor_changed),
			                  obj);

			g_signal_connect (G_OBJECT (self->priv->head_monitor), "changed",
			                  G_CALLBACK (on_head_monitor_changed),
			                  obj);

		}
	}

	g_free (git_index_path);
	g_free (git_head_path);
	g_object_unref (git_index_file);
	g_object_unref (git_head_file);
}

static void
git_index_refreshable_stop_monitor (IAnjutaRefreshable *obj, GError **err)
{
	GitIndexRefreshable *self;

	self = GIT_INDEX_REFRESHABLE (obj);

	g_clear_object (&self->priv->index_monitor);
	g_clear_object (&self->priv->head_monitor);
}

static void
ianjuta_refreshable_init (IAnjutaRefreshableIface *iface)
{
	iface->start_monitor = git_index_refreshable_start_monitor;
	iface->stop_monitor = git_index_refreshable_stop_monitor;
}

G_DEFINE_TYPE_WITH_CODE (GitIndexRefreshable, git_index_refreshable, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (IANJUTA_TYPE_REFRESHABLE, 
                                                ianjuta_refreshable_init));

static void
git_index_refreshable_init (GitIndexRefreshable *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GIT_TYPE_INDEX_REFRESHABLE, GitIndexRefreshablePrivate);

}

static void
git_index_refreshable_finalize (GObject *object)
{
	GitIndexRefreshable *self;

	self = GIT_INDEX_REFRESHABLE (object);

	g_clear_object (&self->priv->index_monitor);
	g_clear_object (&self->priv->head_monitor);

	G_OBJECT_CLASS (git_index_refreshable_parent_class)->finalize (object);
}

static void
git_index_refreshable_class_init (GitIndexRefreshableClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GitIndexRefreshablePrivate));

	object_class->finalize = git_index_refreshable_finalize;
}

IAnjutaRefreshable *
git_index_refreshable_new (Git *plugin)
{
	GitIndexRefreshable *self;

	self = g_object_new (GIT_TYPE_INDEX_REFRESHABLE, NULL);
	self->priv->plugin = plugin;

	return IANJUTA_REFRESHABLE (self);
}
