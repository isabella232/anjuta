/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-refresh-index-command.c
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

#include "git-refresh-index-command.h"

G_DEFINE_TYPE (GitRefreshIndexCommand, git_refresh_index_command, GIT_TYPE_COMMAND);

static void
git_refresh_index_command_init (GitRefreshIndexCommand *self)
{

}

static void
git_refresh_index_command_finalize (GObject *object)
{

	G_OBJECT_CLASS (git_refresh_index_command_parent_class)->finalize (object);
}

static void
git_refresh_index_command_run (AnjutaTask *task)
{
	GgitRepository *repository;

	repository = git_command_get_repository (GIT_COMMAND (task));

	g_return_if_fail (repository);
	ggit_index_read (ggit_repository_get_index (repository, NULL), NULL);
}

static void
git_refresh_index_command_class_init (GitRefreshIndexCommandClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	AnjutaTaskClass *task_class = ANJUTA_TASK_CLASS (klass);

	object_class->finalize = git_refresh_index_command_finalize;
	task_class->run = git_refresh_index_command_run;
}


GitCommand *
git_refresh_index_command_new (void)
{
	return g_object_new (GIT_TYPE_REFRESH_INDEX_COMMAND, NULL);
}

