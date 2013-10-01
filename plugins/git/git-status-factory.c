/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-status-factory.c
 * Copyright (C) 2013 James Liggett <jim@jim-dekstop>
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

#include "git-status-factory.h"

#define STATUS_REGEX "((M|A|D|U|\\?|\\s){2}) (.*)"

struct _GitStatusFactoryPrivate
{
	GHashTable *status_codes;
	GHashTable *conflict_codes;
	GRegex *status_regex;
};


G_DEFINE_TYPE (GitStatusFactory, git_status_factory, G_TYPE_OBJECT);

static void
git_status_factory_init (GitStatusFactory *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GIT_TYPE_STATUS_FACTORY, GitStatusFactoryPrivate);

	self->priv->status_regex = g_regex_new (STATUS_REGEX, 0, 0, NULL);
	self->priv->status_codes = g_hash_table_new (g_direct_hash, g_direct_equal);
	self->priv->conflict_codes = g_hash_table_new (g_str_hash, g_str_equal);

	/* Initialize status code hash tables */
	g_hash_table_insert (self->priv->status_codes, 
	                     GINT_TO_POINTER ('M'),
	                     GINT_TO_POINTER (ANJUTA_VCS_STATUS_MODIFIED));

	g_hash_table_insert (self->priv->status_codes, 
	                     GINT_TO_POINTER ('A'),
	                     GINT_TO_POINTER (ANJUTA_VCS_STATUS_ADDED));

	g_hash_table_insert (self->priv->status_codes, 
	                     GINT_TO_POINTER ('D'),
	                     GINT_TO_POINTER (ANJUTA_VCS_STATUS_DELETED));

	g_hash_table_insert (self->priv->status_codes,
	                     GINT_TO_POINTER (' '),
	                     GINT_TO_POINTER (ANJUTA_VCS_STATUS_NONE));

	g_hash_table_insert (self->priv->status_codes,
	                     GINT_TO_POINTER ('?'),
	                     GINT_TO_POINTER (ANJUTA_VCS_STATUS_UNVERSIONED));

	/* TODO: Handle each conflict case individually so that we can eventually
	 * give the user more information about the conflict */
	g_hash_table_insert (self->priv->conflict_codes, "DD", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "AU", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "UD", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "UA", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "DU", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "AA", NULL);
	g_hash_table_insert (self->priv->conflict_codes, "UU", NULL);
}

static void
git_status_factory_finalize (GObject *object)
{
	GitStatusFactory *self;

	self = GIT_STATUS_FACTORY (object);

	g_regex_unref (self->priv->status_regex);
	g_hash_table_destroy (self->priv->status_codes);
	g_hash_table_destroy (self->priv->conflict_codes);

	G_OBJECT_CLASS (git_status_factory_parent_class)->finalize (object);
}

static void
git_status_factory_class_init (GitStatusFactoryClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GitStatusFactoryPrivate));

	object_class->finalize = git_status_factory_finalize;
}


GitStatusFactory *
git_status_factory_new (void)
{
	return g_object_new (GIT_TYPE_STATUS_FACTORY, NULL);
}

GitStatus *
git_status_factory_create_status (GitStatusFactory *self, 
                                  const gchar *status_line)
{
	GMatchInfo *match_info;
	GitStatus *status_object;
	gchar *status;
	gchar *path;

	if (g_regex_match (self->priv->status_regex, status_line, 0, &match_info))
	{
		/* Determine which section this entry goes in */
		status = g_match_info_fetch (match_info, 1);
		path = g_match_info_fetch (match_info, 3);

		if (!g_hash_table_lookup_extended (self->priv->conflict_codes, status, 
		                                   NULL, NULL))
		{
			status_object = git_status_new (path, 
			                                GPOINTER_TO_INT (g_hash_table_lookup (self->priv->status_codes, GINT_TO_POINTER (status[0]))),
			                                GPOINTER_TO_INT (g_hash_table_lookup (self->priv->status_codes, GINT_TO_POINTER (status[1]))));
		}
		else
		{
			/* Show conflicts in the working tree only */
			status_object = git_status_new (path, ANJUTA_VCS_STATUS_NONE, 
			                                ANJUTA_VCS_STATUS_CONFLICTED);
		}

		g_free (status);
		g_free (path);
	}
	else
	{
		status_object = git_status_new ("", ANJUTA_VCS_STATUS_NONE, 
		                                ANJUTA_VCS_STATUS_NONE);
	}

	g_match_info_free (match_info);

	return status_object;
}

