/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta
 * Copyright (C) James Liggett 2007 <jrliggett@cox.net>
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

#include "anjuta-task.h"

/**
 * SECTION: anjuta-task
 * @short_description: System for creating objects that provide a standard 
 *					   interface to external components (libraries, processes,
 *					   etc.) 
 * @see_also: #AnjutaAsyncTask
 * @include libanjuta/anjuta-task.h
 *
 * #AnjutaTask is the base class for objects that are designed to provide 
 * a layer of abstraction between UI code and some other component, like a 
 * library or child process. AnjutaTask provides a simple and consistent
 * interface for plugins to interact with these components without needing 
 * to concern themselves with the exact details of how these components work.
 * 
 * To create task objects, plugins derive them from an #AnjutaTask 
 * subclass like #AnjutaAsyncTask, which runs tasks in another thread or 
 * #AnjutaSyncTask, which runs tasks synchronously.
 *
 * These classes determine how ::run is called and how signals are emitted.
 * ::run is responsible for actually doing the work of the task. It is the 
 * responsiblity of the task object that does a certain task to implement 
 * ::run to do its job. Everything else is normally implemented by its parent
 * classes at this point
 *
 * For an example of how to use #AnjutaTask, see the Git plugin.
 */

enum
{
	DATA_ARRIVED,
	STARTED,
	FINISHED,
	PROGRESS,

	LAST_SIGNAL
};


static guint anjuta_task_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AnjutaTask, anjuta_task, G_TYPE_OBJECT);

static void
anjuta_task_init (AnjutaTask *self)
{
}

static void
anjuta_task_finalize (GObject *object)
{
	G_OBJECT_CLASS (anjuta_task_parent_class)->finalize (object);
}

static gboolean
start_automatic_monitor (AnjutaTask *self)
{
	return FALSE;
}

static void
stop_automatic_monitor (AnjutaTask *self)
{
}

static void
data_arrived (AnjutaTask *task)
{
}

static void
started (AnjutaTask *task)
{
}

static void
finished (AnjutaTask *task)
{
}

static void
progress (AnjutaTask *task, gfloat progress)
{
}

static void
anjuta_task_class_init (AnjutaTaskClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = anjuta_task_finalize;
	
	klass->run = NULL;
	klass->start = NULL;
	klass->cancel = NULL;
	klass->notify_data_arrived = NULL;
	klass->notify_finished = NULL;
	klass->notify_progress = NULL;
	klass->is_running = NULL;
	klass->start_automatic_monitor = start_automatic_monitor;
	klass->stop_automatic_monitor = stop_automatic_monitor;
	klass->data_arrived = data_arrived;
	klass->started = started;
	klass->finished = finished;
	klass->progress = progress;

	/**
	 * AnjutaTask::data-arrived:
	 * @task: Task
	 * 
	 * Notifies clients that the task has processed data that is ready to 
	 * be used.
	 */
	anjuta_task_signals[DATA_ARRIVED] =
		g_signal_new ("data-arrived",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (AnjutaTaskClass, data_arrived),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 
					  0);

	/**
	 * AnjuaTask::started:
	 * @task: Task
	 *
	 * Indicates that a task has begun executing. This signal is intended to 
	 * be used for tasks that start themselves automatically.
	 */
	anjuta_task_signals[STARTED] =
		g_signal_new ("started",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (AnjutaTaskClass, started),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 
		              0);

	/**
	 * AnjutaTask::finished:
	 * @task: Task
	 * @return_code: The return code of the finished commmand
	 *
	 * Indicates that the task has completed. Clients should at least handle
	 * this signal to unref the task object.
	 */
	anjuta_task_signals[FINISHED] =
		g_signal_new ("finished",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (AnjutaTaskClass, finished),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID ,
		              G_TYPE_NONE,
		              0);
	
	
	/**
	 * AnjutaTask::progress:
	 * @task: Task
	 * @progress: Fraction of the task's task that is complete, between 0.0
	 *			  and 1.0, inclusive.
	 *
	 * Notifies clients of changes in progress during task execution. 
	 */
	anjuta_task_signals[PROGRESS] =
		g_signal_new ("progress",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (AnjutaTaskClass, progress),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__FLOAT,
		              G_TYPE_NONE, 1,
		              G_TYPE_FLOAT);
}

/**
 * anjuta_task_start:
 * @self: Task object to start
 *
 * Starts a task. Client code can handle data from the task by connecting
 * to the ::data-arrived signal. 
 *
 * #AnjutaTask subclasses should override this method to determine how they
 * call ::run, which actually does the task's legwork. 
 */
void
anjuta_task_start (AnjutaTask *self)
{
	ANJUTA_TASK_GET_CLASS (self)->start (self);
}

/**
 * anjuta_task_cancel:
 * @self: Task object.
 *
 * Cancels a running task.
 */
void
anjuta_task_cancel (AnjutaTask *self)
{
	ANJUTA_TASK_GET_CLASS (self)->cancel (self);
}

/**
 * anjuta_task_notify_data_arrived:
 * @self: Task object.
 * 
 * Used by base classes derived from #AnjutaTask to emit the ::data-arrived
 * signal. This method should be used by both base task classes and
 * non-base classes as appropriate. 
 */
void
anjuta_task_notify_data_arrived (AnjutaTask *self)
{
	ANJUTA_TASK_GET_CLASS (self)->notify_data_arrived (self);
}

/**
 * anjuta_task_notify_finished:
 * @self: Task object
 * 
 * Used by base classes derived from #AnjutaTask to emit the 
 * ::task-finished signal. This method should not be used by client code or  
 * #AnjutaTask objects that are not base classes. 
 */
void
anjuta_task_notify_finished (AnjutaTask *self)
{
	ANJUTA_TASK_GET_CLASS (self)->notify_finished (self);
}

/**
 * anjuta_task_notify_progress:
 * @self: Task object.
 * @progress: The of the task that is passed to the notify callback
 * 
 * Emits the ::progress signal. Can be used by both base classes and 
 * tasks as needed. 
 */
void 
anjuta_task_notify_progress (AnjutaTask *self, gfloat progress)
{
	ANJUTA_TASK_GET_CLASS (self)->notify_progress (self, progress);
}

/**
 * anjuta_task_is_running:
 * @self: Task object.
 *
 * Return value: %TRUE if the task is currently running; %FALSE otherwise.
 */
gboolean
anjuta_task_is_running (AnjutaTask *self)
{
	return ANJUTA_TASK_GET_CLASS (self)->is_running (self);
}

/**
 * anjuta_task_start_automatic_monitor:
 * @self: Task object.
 *
 * Sets up any monitoring needed for tasks that should start themselves 
 * automatically in response to some event. 
 *
 * Return value: %TRUE if automatic starting is supported by the task and 
 * no errors were encountered; %FALSE if automatic starting is unsupported or on
 * error.
 */
gboolean
anjuta_task_start_automatic_monitor (AnjutaTask *self)
{
	return ANJUTA_TASK_GET_CLASS (self)->start_automatic_monitor (self);
}

/**
 * anjuta_task_stop_automatic_monitor:
 * @self: Task object.
 *
 * Stops automatic monitoring for self executing tasks. For tasks that 
 * do not support self-starting, this function does nothing.
 */
void
anjuta_task_stop_automatic_monitor (AnjutaTask *self)
{
	ANJUTA_TASK_GET_CLASS (self)->stop_automatic_monitor (self);
}
