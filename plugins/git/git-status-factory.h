/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * git-status-factory.h
 * Copyright (C) 2013 James Liggett <jim@jim-dekstop>
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

#ifndef _GIT_STATUS_FACTORY_H_
#define _GIT_STATUS_FACTORY_H_

#include <glib-object.h>
#include "git-status.h"

G_BEGIN_DECLS

#define GIT_TYPE_STATUS_FACTORY             (git_status_factory_get_type ())
#define GIT_STATUS_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIT_TYPE_STATUS_FACTORY, GitStatusFactory))
#define GIT_STATUS_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIT_TYPE_STATUS_FACTORY, GitStatusFactoryClass))
#define GIT_IS_STATUS_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIT_TYPE_STATUS_FACTORY))
#define GIT_IS_STATUS_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIT_TYPE_STATUS_FACTORY))
#define GIT_STATUS_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIT_TYPE_STATUS_FACTORY, GitStatusFactoryClass))

typedef struct _GitStatusFactoryClass GitStatusFactoryClass;
typedef struct _GitStatusFactory GitStatusFactory;
typedef struct _GitStatusFactoryPrivate GitStatusFactoryPrivate;

struct _GitStatusFactoryClass
{
	GObjectClass parent_class;
};

struct _GitStatusFactory
{
	GObject parent_instance;

	GitStatusFactoryPrivate *priv;
};

GType git_status_factory_get_type (void) G_GNUC_CONST;
GitStatusFactory * git_status_factory_new (void);
GitStatus *git_status_factory_create_status (GitStatusFactory *self, 
                                             const gchar *status_line);

G_END_DECLS

#endif /* _GIT_STATUS_FACTORY_H_ */

