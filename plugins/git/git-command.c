/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-command.c
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

#include "git-command.h"

struct _GitCommandPrivate
{
};


enum
{
	PROP_0,

	PROP_REPOSITORY
};



G_DEFINE_TYPE (GitCommand, git_command, ANJUTA_TYPE_ASYNC_TASK);

static void
git_command_init (GitCommand *git_command)
{
	git_command->priv = G_TYPE_INSTANCE_GET_PRIVATE (git_command, GIT_TYPE_COMMAND, GitCommandPrivate);

	/* TODO: Add initialization code here */
}

static void
git_command_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (git_command_parent_class)->finalize (object);
}

static void
git_command_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GIT_IS_COMMAND (object));

	switch (prop_id)
	{
	case PROP_REPOSITORY:
		/* TODO: Add setter for "repository" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
git_command_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GIT_IS_COMMAND (object));

	switch (prop_id)
	{
	case PROP_REPOSITORY:
		/* TODO: Add getter for "repository" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
git_command_class_init (GitCommandClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GitCommandPrivate));

	object_class->finalize = git_command_finalize;
	object_class->set_property = git_command_set_property;
	object_class->get_property = git_command_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_REPOSITORY,
	                                 g_param_spec_object ("repository",
	                                                      "repository",
	                                                      "Git repository that this command operates on",
	                                                      GGIT_TYPE_REPOSITORY,
	                                                      G_PARAM_CONSTRUCT_ONLY));
}


