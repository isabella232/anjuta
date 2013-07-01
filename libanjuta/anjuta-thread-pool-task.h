/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-thread-pool-task.h
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

#ifndef _ANJUTA_THREAD_POOL_TASK_H_
#define _ANJUTA_THREAD_POOL_TASK_H_

#include <glib-object.h>
#include "anjuta-async-task.h"

G_BEGIN_DECLS

#define ANJUTA_TYPE_THREAD_POOL_TASK             (anjuta_thread_pool_task_get_type ())
#define ANJUTA_THREAD_POOL_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ANJUTA_TYPE_THREAD_POOL_TASK, AnjutaThreadPoolTask))
#define ANJUTA_THREAD_POOL_TASK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ANJUTA_TYPE_THREAD_POOL_TASK, AnjutaThreadPoolTaskClass))
#define ANJUTA_IS_THREAD_POOL_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ANJUTA_TYPE_THREAD_POOL_TASK))
#define ANJUTA_IS_THREAD_POOL_TASK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ANJUTA_TYPE_THREAD_POOL_TASK))
#define ANJUTA_THREAD_POOL_TASK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ANJUTA_TYPE_THREAD_POOL_TASK, AnjutaThreadPoolTaskClass))

typedef struct _AnjutaThreadPoolTaskClass AnjutaThreadPoolTaskClass;
typedef struct _AnjutaThreadPoolTask AnjutaThreadPoolTask;
typedef struct _AnjutaThreadPoolTaskPrivate AnjutaThreadPoolTaskPrivate;


struct _AnjutaThreadPoolTaskClass
{
	AnjutaAsyncTaskClass parent_class;
};

struct _AnjutaThreadPoolTask
{
	AnjutaAsyncTask parent_instance;

	AnjutaThreadPoolTaskPrivate *priv;
};

GType anjuta_thread_pool_task_get_type (void) G_GNUC_CONST;
gboolean anjuta_thread_pool_task_is_ready (AnjutaThreadPoolTask *self);
gboolean anjuta_thread_pool_task_is_waiting (AnjutaThreadPoolTask *self);
void anjuta_thread_pool_task_notify_waiting (AnjutaThreadPoolTask *self);

G_END_DECLS

#endif /* _ANJUTA_THREAD_POOL_TASK_H_ */

