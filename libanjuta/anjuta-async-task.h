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

#ifndef _ANJUTA_ASYNC_TASK_H_
#define _ANJUTA_ASYNC_TASK_H_

#include <glib-object.h>
#include "anjuta-task.h"

G_BEGIN_DECLS

#define ANJUTA_TYPE_ASYNC_TASK             (anjuta_async_task_get_type ())
#define ANJUTA_ASYNC_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ANJUTA_TYPE_ASYNC_TASK, AnjutaAsyncTask))
#define ANJUTA_ASYNC_TASK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ANJUTA_TYPE_ASYNC_TASK, AnjutaAsyncTaskClass))
#define ANJUTA_IS_ASYNC_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ANJUTA_TYPE_ASYNC_TASK))
#define ANJUTA_IS_ASYNC_TASK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ANJUTA_TYPE_ASYNC_TASK))
#define ANJUTA_ASYNC_TASK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ANJUTA_TYPE_ASYNC_TASK, AnjutaAsyncTaskClass))

typedef struct _AnjutaAsyncTaskClass AnjutaAsyncTaskClass;
typedef struct _AnjutaAsyncTask AnjutaAsyncTask;
typedef struct _AnjutaAsyncTaskPriv AnjutaAsyncTaskPriv;

struct _AnjutaAsyncTaskClass
{
	AnjutaTaskClass parent_class;
};

struct _AnjutaAsyncTask
{
	AnjutaTask parent_instance;
	
	AnjutaAsyncTaskPriv *priv;
};

GType anjuta_async_task_get_type (void) G_GNUC_CONST;

void anjuta_async_task_lock (AnjutaAsyncTask *self);
void anjuta_async_task_unlock (AnjutaAsyncTask *self);

G_END_DECLS

#endif /* _ANJUTA_ASYNC_TASK_H_ */
