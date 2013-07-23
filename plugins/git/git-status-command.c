/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta
 * Copyright (C) James Liggett 2008 <jrliggett@cox.net>
 * 
 * anjuta is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * anjuta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with anjuta.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "git-status-command.h"

struct _GitStatusCommandPriv
{
	GAsyncQueue *status_queue;
	GitStatusSections sections;
	gchar *path;
};

G_DEFINE_TYPE (GitStatusCommand, git_status_command, GIT_TYPE_COMMAND);

static void
git_status_command_init (GitStatusCommand *self)
{
	self->priv = g_new0 (GitStatusCommandPriv, 1);
	self->priv->status_queue = g_async_queue_new_full (g_object_unref);
}

static void
git_status_command_finalize (GObject *object)
{
	GitStatusCommand *self;
	
	self = GIT_STATUS_COMMAND (object);
	
	g_async_queue_unref (self->priv->status_queue);
	g_free (self->priv->path);
	
	g_free (self->priv);

	G_OBJECT_CLASS (git_status_command_parent_class)->finalize (object);
}

static gint
git_status_command_status_callback (const gchar *file, GgitStatusFlags flags, 
             						GitStatusCommand *self)
{
	GitStatus *status;

	status = git_status_new (file, flags);

	g_async_queue_push (self->priv->status_queue, status);
	anjuta_task_notify_data_arrived (ANJUTA_TASK (self));

	return 0;
}

static void
git_status_command_run (AnjutaTask *task)
{
	GitStatusCommand *self;
	GgitRepository *repository;
	GgitStatusOptions *options;
	GgitStatusOption option_flags;
	const gchar *paths[2];

	self = GIT_STATUS_COMMAND (task);
	repository = git_command_get_repository (GIT_COMMAND (task));

	g_return_if_fail (repository);

	option_flags = 0;

	if (self->priv->sections & GIT_STATUS_SECTION_UNTRACKED)
		option_flags |= GGIT_STATUS_OPTION_INCLUDE_UNTRACKED;

	paths[0] = self->priv->path;
	paths[1] = NULL;

	options = ggit_status_options_new (option_flags, 0, paths);

	ggit_repository_file_status_foreach (repository, options, 
	                                     (GgitStatusCallback) git_status_command_status_callback,
	                                     self, NULL);

	ggit_status_options_free (options);
}

static void
git_status_command_class_init (GitStatusCommandClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	AnjutaTaskClass* task_class = ANJUTA_TASK_CLASS (klass);

	object_class->finalize = git_status_command_finalize;
	task_class->run = git_status_command_run;
}


GitCommand *
git_status_command_new (GitStatusSections sections, const gchar *path)
{
	GitStatusCommand *self;
	
	self = g_object_new (GIT_TYPE_STATUS_COMMAND, NULL);
	
	self->priv->sections = sections;
	self->priv->path = g_strdup (path);
	
	return GIT_COMMAND (self);
}

GAsyncQueue *
git_status_command_get_status_queue (GitStatusCommand *self)
{
	return self->priv->status_queue;
}
