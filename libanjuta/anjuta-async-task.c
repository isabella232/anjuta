/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta
 * Copyright (C) James Liggett 2007 <jrliggett@cox.net>
 *
 * Portions based on the original Subversion plugin 
 * Copyright (C) Johannes Schmid 2005 
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

#include "anjuta-async-task.h"

/**
 * SECTION: anjuta-async-task
 * @short_description: #AnjutaTask subclass that serves as the base for 
 *					   tasks that need to run in another thread.
 * @include: libanjuta/anjuta-async-task.h
 *
 * AnjutaAsyncTask provides a simple way for plugins to run tasks that 
 * are synchronous and usually take several seconds or longer to execute in 
 * another thread so that such tasks do no block Anjuta's user interface.
 *
 * #AnjutaAsyncTask provides basic notifcation and synchronization services for
 * tasks that use a thread, either one they create or from some other threading
 * mechanism such as a thread pool. 
 * 
 * See also: #AnjutaActiveAsyncTask, #AnjutaThreadPoolTask
 */

struct _AnjutaAsyncTaskPriv
{
	GMutex data_mutex;
	GMutex flags_mutex;
	gint flags;
	gfloat progress;
};

/* Notification flags */
enum
{
	RUNNING = (1 << 0),
	DATA_ARRIVED = (1 << 1),
	FINISHED = (1 << 2),
	PROGRESS = (1 << 3),
};

G_DEFINE_TYPE (AnjutaAsyncTask, anjuta_async_task, ANJUTA_TYPE_TASK);

static void
anjuta_async_task_init (AnjutaAsyncTask *self)
{
	self->priv = g_new0 (AnjutaAsyncTaskPriv, 1);

	g_mutex_init (&self->priv->data_mutex);
	g_mutex_init (&self->priv->flags_mutex);
}

static void
anjuta_async_task_finalize (GObject *object)
{
	AnjutaAsyncTask *self;
	
	self = ANJUTA_ASYNC_TASK (object);
	
	g_mutex_clear (&self->priv->data_mutex);
	g_mutex_clear (&self->priv->flags_mutex);
	g_idle_remove_by_data (self);
	
	g_free (self->priv);

	G_OBJECT_CLASS (anjuta_async_task_parent_class)->finalize (object);
}

static void
anjuta_async_task_set_flag (AnjutaAsyncTask *self, gint flag)
{
	g_mutex_lock (&self->priv->flags_mutex);

	self->priv->flags |= flag;

	g_mutex_unlock (&self->priv->flags_mutex);
}

static void
anjuta_async_task_clear_flag (AnjutaAsyncTask *self, gint flag)
{
	g_mutex_lock (&self->priv->flags_mutex);

	self->priv->flags &= ~flag;

	g_mutex_unlock (&self->priv->flags_mutex);
}


static gboolean
anjuta_async_task_notification_poll (AnjutaAsyncTask *self)
{
	gint flags;
	gfloat progress;

	/* Use a stack-local copy of the flags and progress values to make sure we
	 * always get the opportunity to handle processed data and progress changes
	 * even if the command finish flag is set while the this idle is running. 
	 * If that happens, the idle will run one more time, emitting the signal 
	 * after everything else is handled. */
	g_mutex_lock (&self->priv->flags_mutex);

	flags = self->priv->flags;
	progress = self->priv->progress;

	g_mutex_unlock (&self->priv->flags_mutex);
	
	if ((flags & DATA_ARRIVED) && g_mutex_trylock (&self->priv->data_mutex))
	{
		g_signal_emit_by_name (self, "data-arrived");
		g_mutex_unlock (&self->priv->data_mutex);

		anjuta_async_task_clear_flag (self, DATA_ARRIVED);

		/* Indicate that data arrived was handled */
		flags &= ~DATA_ARRIVED;
	}
	
	if (flags & PROGRESS)
	{
		g_signal_emit_by_name (self, "progress", progress);
		anjuta_async_task_clear_flag (self, PROGRESS);
	}
	
	/* Let the command process any remaining data before finishing in case we
	 * didn't acquire the data lock */
	if (flags & FINISHED && (flags & DATA_ARRIVED) == 0)
	{
		/* Indicate that the idle is no longer running and we won't send any more
		 * notifications. */
		anjuta_async_task_clear_flag (self, RUNNING);		

		g_signal_emit_by_name (self, "finished");

		return FALSE;
	}
	else
		return TRUE;
	
}

static gboolean
notify_started (AnjutaAsyncTask *self)
{
	g_signal_emit_by_name (self, "started");
	return FALSE;
}

static void
start (AnjutaTask *task)
{
	AnjutaAsyncTask *self;

	self = ANJUTA_ASYNC_TASK (task);

	anjuta_async_task_set_flag (self, RUNNING);

	/* Emit the started signal in the main thread */
	g_idle_add ((GSourceFunc) notify_started, self);
	
	g_idle_add ((GSourceFunc) anjuta_async_task_notification_poll, 
				ANJUTA_ASYNC_TASK (task));
}

static void
notify_data_arrived (AnjutaTask *task)
{
	AnjutaAsyncTask *self;
	
	self = ANJUTA_ASYNC_TASK (task);
	
	anjuta_async_task_set_flag (self, DATA_ARRIVED);
}

static void
notify_finished (AnjutaTask *task)
{
	AnjutaAsyncTask *self;
	
	self = ANJUTA_ASYNC_TASK (task);
	
	anjuta_async_task_set_flag (self, FINISHED);
}

static void
notify_progress (AnjutaTask *task, gfloat progress)
{
	AnjutaAsyncTask *self;
	
	self = ANJUTA_ASYNC_TASK (task);

	g_mutex_lock (&self->priv->flags_mutex);

	self->priv->progress = progress;
	self->priv->flags |= PROGRESS;

	g_mutex_unlock (&self->priv->flags_mutex);
}

static gboolean
is_running (AnjutaTask *task)
{
	AnjutaAsyncTask *self;
	gint flags;

	self = ANJUTA_ASYNC_TASK (task);

	g_mutex_lock (&self->priv->flags_mutex);
	flags = self->priv->flags;
	g_mutex_unlock (&self->priv->flags_mutex);

	return (flags & RUNNING);
}

static void
anjuta_async_task_class_init (AnjutaAsyncTaskClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	AnjutaTaskClass* parent_class = ANJUTA_TASK_CLASS (klass);

	object_class->finalize = anjuta_async_task_finalize;
	
	parent_class->start = start;
	parent_class->notify_data_arrived = notify_data_arrived;
	parent_class->notify_finished = notify_finished;
	parent_class->notify_progress = notify_progress;
	parent_class->is_running = is_running;
}

/**
 * anjuta_async_task_lock:
 * @self: AnjutaAsyncTask object.
 *
 * Locks the task's built-in mutex.
 */
void
anjuta_async_task_lock (AnjutaAsyncTask *self)
{
	g_mutex_lock (&self->priv->data_mutex);
}

/**
 * anjuta_async_task_unlock:
 * @self: AnjutaAsyncTask object.
 *
 * Unlocks the task's built-in mutex.
 */
void
anjuta_async_task_unlock (AnjutaAsyncTask *self)
{
	g_mutex_unlock (&self->priv->data_mutex);
}
