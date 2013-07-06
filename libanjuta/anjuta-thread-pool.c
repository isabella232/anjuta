/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-thread-pool.c
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

#include "anjuta-thread-pool.h"

struct _AnjutaThreadPoolPrivate
{
	GThreadPool *thread_pool;
};


G_DEFINE_TYPE (AnjutaThreadPool, anjuta_thread_pool, G_TYPE_OBJECT);

static void
start_task (AnjutaThreadPoolTask *task, gpointer user_data)
{
	anjuta_task_start (ANJUTA_TASK (task));
}

static void
anjuta_thread_pool_init (AnjutaThreadPool *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, ANJUTA_TYPE_THREAD_POOL, AnjutaThreadPoolPrivate);

	self->priv->thread_pool = g_thread_pool_new ((GFunc) start_task,
	                                             NULL, -1, FALSE, NULL);
}

static void
anjuta_thread_pool_finalize (GObject *object)
{
	AnjutaThreadPool *self;

	self = ANJUTA_THREAD_POOL (object);

	g_thread_pool_free (self->priv->thread_pool, TRUE, FALSE);

	G_OBJECT_CLASS (anjuta_thread_pool_parent_class)->finalize (object);
}

static void
anjuta_thread_pool_class_init (AnjutaThreadPoolClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AnjutaThreadPoolPrivate));

	object_class->finalize = anjuta_thread_pool_finalize;
}

void
anjuta_thread_pool_push (AnjutaThreadPool *self, AnjutaThreadPoolTask *task)
{
	anjuta_thread_pool_task_notify_waiting (task);
	g_thread_pool_push (self->priv->thread_pool, task, NULL);
}

AnjutaThreadPool *
anjuta_thread_pool_new (void)
{
	return g_object_new (ANJUTA_TYPE_THREAD_POOL, NULL);
}

