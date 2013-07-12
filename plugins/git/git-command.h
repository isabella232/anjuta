/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-command.h
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

#ifndef _GIT_COMMAND_H_
#define _GIT_COMMAND_H_

#include <glib-object.h>
#include <libanjuta/anjuta-async-task.h>
#include <libgit2-glib/ggit-repository.h>

G_BEGIN_DECLS

#define GIT_TYPE_COMMAND             (git_command_get_type ())
#define GIT_COMMAND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIT_TYPE_COMMAND, GitCommand))
#define GIT_COMMAND_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIT_TYPE_COMMAND, GitCommandClass))
#define GIT_IS_COMMAND(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIT_TYPE_COMMAND))
#define GIT_IS_COMMAND_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIT_TYPE_COMMAND))
#define GIT_COMMAND_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIT_TYPE_COMMAND, GitCommandClass))

typedef struct _GitCommandClass GitCommandClass;
typedef struct _GitCommand GitCommand;
typedef struct _GitCommandPrivate GitCommandPrivate;


struct _GitCommandClass
{
	AnjutaAsyncTaskClass parent_class;
};

struct _GitCommand
{
	AnjutaAsyncTask parent_instance;

	GitCommandPrivate *priv;
};

GType git_command_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _GIT_COMMAND_H_ */

