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

#include "git-diff-command.h"

G_DEFINE_TYPE (GitDiffCommand, git_diff_command, GIT_TYPE_RAW_OUTPUT_COMMAND);

struct _GitDiffCommandPriv
{
	gchar *path;
	GitDiffType type;
};

static void
git_diff_command_init (GitDiffCommand *self)
{
	self->priv = g_new0 (GitDiffCommandPriv, 1);
}

static void
git_diff_command_finalize (GObject *object)
{
	GitDiffCommand *self;

	self = GIT_DIFF_COMMAND (object);

	g_free (self->priv->path);
	g_free (self->priv);

	G_OBJECT_CLASS (git_diff_command_parent_class)->finalize (object);
}

static guint
git_diff_command_run (AnjutaCommand *command)
{	
	GitDiffCommand *self;

	self = GIT_DIFF_COMMAND (command);

	git_command_add_arg (GIT_COMMAND (command), "diff");

	if (self->priv->type == GIT_DIFF_INDEX)
		git_command_add_arg (GIT_COMMAND (command), "--cached");

	if (self->priv->path)
		git_command_add_arg (GIT_COMMAND (command), self->priv->path);
	
	return 0;
}

static void
git_diff_command_class_init (GitDiffCommandClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	AnjutaCommandClass *command_class = ANJUTA_COMMAND_CLASS (klass);

	object_class->finalize = git_diff_command_finalize;
	command_class->run = git_diff_command_run;
}


GitDiffCommand *
git_diff_command_new (const gchar *working_directory, const gchar *path, 
                      GitDiffType type)
{
	GitDiffCommand *self;

	self = g_object_new (GIT_TYPE_DIFF_COMMAND, 
						 "working-directory", working_directory,
						 NULL);

	self->priv->path = g_strdup (path);
	self->priv->type = type;

	return self;
}
