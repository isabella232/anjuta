/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-thread-pool-task.c
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

#include "anjuta-thread-pool-task.h"

struct _AnjutaThreadPoolTaskPrivate
{
	GMutex waiting_mutex;
	gboolean waiting;
};


G_DEFINE_TYPE (AnjutaThreadPoolTask, anjuta_thread_pool_task, ANJUTA_TYPE_ASYNC_TASK);

static void
anjuta_thread_pool_task_init (AnjutaThreadPoolTask *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, ANJUTA_TYPE_THREAD_POOL_TASK, AnjutaThreadPoolTaskPrivate);

	g_mutex_init (&self->priv->waiting_mutex);
}

static void
anjuta_thread_pool_task_finalize (GObject *object)
{
	AnjutaThreadPoolTask *self;

	self = ANJUTA_THREAD_POOL_TASK (object);

	g_mutex_clear (&self->priv->waiting_mutex);

	G_OBJECT_CLASS (anjuta_thread_pool_task_parent_class)->finalize (object);
}

static void
start (AnjutaTask *task)
{
	AnjutaThreadPoolTask *self;

	self = ANJUTA_THREAD_POOL_TASK (task);

	if (anjuta_thread_pool_task_is_ready (self))
	{
		g_mutex_lock (&self->priv->waiting_mutex);
		self->priv->waiting = FALSE;
		g_mutex_unlock (&self->priv->waiting_mutex);

		ANJUTA_TASK_CLASS (anjuta_thread_pool_task_parent_class)->start (task);
		ANJUTA_TASK_GET_CLASS (self)->run (task);
	}
}

static void
anjuta_thread_pool_task_class_init (AnjutaThreadPoolTaskClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	AnjutaTaskClass *task_class = ANJUTA_TASK_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AnjutaThreadPoolTaskPrivate));

	object_class->finalize = anjuta_thread_pool_task_finalize;
	task_class->start = start;
}


gboolean
anjuta_thread_pool_task_is_ready (AnjutaThreadPoolTask *self)
{
	return !anjuta_task_is_running (ANJUTA_TASK (self)) &&
		   !anjuta_thread_pool_task_is_waiting (self);
}

gboolean
anjuta_thread_pool_task_is_waiting (AnjutaThreadPoolTask *self)
{
	gboolean waiting;

	g_mutex_lock (&self->priv->waiting_mutex);
	waiting = self->priv->waiting;
	g_mutex_unlock (&self->priv->waiting_mutex);

	return waiting;
}

void
anjuta_thread_pool_task_notify_waiting (AnjutaThreadPoolTask *self)
{
	g_mutex_lock (&self->priv->waiting_mutex);
	self->priv->waiting = TRUE;
	g_mutex_unlock (&self->priv->waiting_mutex);
}

