/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-refresh-index-command.h
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

#ifndef _GIT_REFRESH_INDEX_COMMAND_H_
#define _GIT_REFRESH_INDEX_COMMAND_H_

#include <glib-object.h>
#include "git-command.h"

G_BEGIN_DECLS

#define GIT_TYPE_REFRESH_INDEX_COMMAND             (git_refresh_index_command_get_type ())
#define GIT_REFRESH_INDEX_COMMAND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIT_TYPE_REFRESH_INDEX_COMMAND, GitRefreshIndexCommand))
#define GIT_REFRESH_INDEX_COMMAND_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIT_TYPE_REFRESH_INDEX_COMMAND, GitRefreshIndexCommandClass))
#define GIT_IS_REFRESH_INDEX_COMMAND(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIT_TYPE_REFRESH_INDEX_COMMAND))
#define GIT_IS_REFRESH_INDEX_COMMAND_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIT_TYPE_REFRESH_INDEX_COMMAND))
#define GIT_REFRESH_INDEX_COMMAND_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIT_TYPE_REFRESH_INDEX_COMMAND, GitRefreshIndexCommandClass))

typedef struct _GitRefreshIndexCommandClass GitRefreshIndexCommandClass;
typedef struct _GitRefreshIndexCommand GitRefreshIndexCommand;


struct _GitRefreshIndexCommandClass
{
	GitCommandClass parent_class;
};

struct _GitRefreshIndexCommand
{
	GitCommand parent_instance;
};

GType git_refresh_index_command_get_type (void) G_GNUC_CONST;
GitCommand *git_refresh_index_command_new (void);

G_END_DECLS

#endif /* _GIT_REFRESH_INDEX_COMMAND_H_ */

