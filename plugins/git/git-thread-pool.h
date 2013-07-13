/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-thread-pool.h
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

#ifndef _GIT_THREAD_POOL_H_
#define _GIT_THREAD_POOL_H_

#include <glib-object.h>
#include <libanjuta/anjuta-thread-pool.h>
#include "git-command.h"

G_BEGIN_DECLS

#define GIT_TYPE_THREAD_POOL             (git_thread_pool_get_type ())
#define GIT_THREAD_POOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIT_TYPE_THREAD_POOL, GitThreadPool))
#define GIT_THREAD_POOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIT_TYPE_THREAD_POOL, GitThreadPoolClass))
#define GIT_IS_THREAD_POOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIT_TYPE_THREAD_POOL))
#define GIT_IS_THREAD_POOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIT_TYPE_THREAD_POOL))
#define GIT_THREAD_POOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIT_TYPE_THREAD_POOL, GitThreadPoolClass))

typedef struct _GitThreadPoolClass GitThreadPoolClass;
typedef struct _GitThreadPool GitThreadPool;
typedef struct _GitThreadPoolPrivate GitThreadPoolPrivate;


struct _GitThreadPoolClass
{
	AnjutaThreadPoolClass parent_class;
};

struct _GitThreadPool
{
	AnjutaThreadPool parent_instance;

	GitThreadPoolPrivate *priv;
};

GType git_thread_pool_get_type (void) G_GNUC_CONST;

void git_thread_pool_push (GitThreadPool *self, GitCommand *command);
GitThreadPool *git_thread_pool_new (GgitRepository *self);

G_END_DECLS

#endif /* _GIT_THREAD_POOL_H_ */

