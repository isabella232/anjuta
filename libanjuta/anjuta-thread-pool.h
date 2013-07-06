/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-thread-pool.h
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

#ifndef _ANJUTA_THREAD_POOL_H_
#define _ANJUTA_THREAD_POOL_H_

#include <glib-object.h>
#include "anjuta-thread-pool-task.h"

G_BEGIN_DECLS

#define ANJUTA_TYPE_THREAD_POOL             (anjuta_thread_pool_get_type ())
#define ANJUTA_THREAD_POOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ANJUTA_TYPE_THREAD_POOL, AnjutaThreadPool))
#define ANJUTA_THREAD_POOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ANJUTA_TYPE_THREAD_POOL, AnjutaThreadPoolClass))
#define ANJUTA_IS_THREAD_POOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ANJUTA_TYPE_THREAD_POOL))
#define ANJUTA_IS_THREAD_POOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ANJUTA_TYPE_THREAD_POOL))
#define ANJUTA_THREAD_POOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ANJUTA_TYPE_THREAD_POOL, AnjutaThreadPoolClass))

typedef struct _AnjutaThreadPoolClass AnjutaThreadPoolClass;
typedef struct _AnjutaThreadPool AnjutaThreadPool;
typedef struct _AnjutaThreadPoolPrivate AnjutaThreadPoolPrivate;


struct _AnjutaThreadPoolClass
{
	GObjectClass parent_class;
};

struct _AnjutaThreadPool
{
	GObject parent_instance;

	AnjutaThreadPoolPrivate *priv;
};

GType anjuta_thread_pool_get_type (void) G_GNUC_CONST;

void anjuta_thread_pool_push (AnjutaThreadPool *self, 
                              AnjutaThreadPoolTask *task);
AnjutaThreadPool *anjuta_thread_pool_new (void);

G_END_DECLS

#endif /* _ANJUTA_THREAD_POOL_H_ */

