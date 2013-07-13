/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-thread-pool.c
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

#include "git-thread-pool.h"

struct _GitThreadPoolPrivate
{
	GgitRepository *repository;
};


G_DEFINE_TYPE (GitThreadPool, git_thread_pool, ANJUTA_TYPE_THREAD_POOL);

static void
git_thread_pool_init (GitThreadPool *git_thread_pool)
{
	git_thread_pool->priv = G_TYPE_INSTANCE_GET_PRIVATE (git_thread_pool, GIT_TYPE_THREAD_POOL, GitThreadPoolPrivate);

	/* TODO: Add initialization code here */
}

static void
git_thread_pool_finalize (GObject *object)
{
	GitThreadPool *self;

	self = GIT_THREAD_POOL (object);

	g_object_unref (self->priv->repository);

	G_OBJECT_CLASS (git_thread_pool_parent_class)->finalize (object);
}

static void
git_thread_pool_class_init (GitThreadPoolClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GitThreadPoolPrivate));

	object_class->finalize = git_thread_pool_finalize;
}


void
git_thread_pool_push (GitThreadPool *self, GitCommand *command)
{
	if (self->priv->repository)
	{
		g_object_set (command, "repository", self->priv->repository, NULL);
		anjuta_thread_pool_push (ANJUTA_THREAD_POOL (self), 
		                         ANJUTA_THREAD_POOL_TASK (command));
	}
}

GitThreadPool *
git_thread_pool_new (GgitRepository *repository)
{
	GitThreadPool *self;

	self = g_object_new (GIT_TYPE_THREAD_POOL, NULL);

	self->priv->repository = g_object_ref (repository);

	return self;
	
}
