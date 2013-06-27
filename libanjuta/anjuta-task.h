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

#ifndef _ANJUTA_TASK_H_
#define _ANJUTA_TASK_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define ANJUTA_TYPE_TASK             (anjuta_task_get_type ())
#define ANJUTA_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ANJUTA_TYPE_TASK, AnjutaTask))
#define ANJUTA_TASK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ANJUTA_TYPE_TASK, AnjutaTaskClass))
#define ANJUTA_IS_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ANJUTA_TYPE_TASK))
#define ANJUTA_IS_TASK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ANJUTA_TYPE_TASK))
#define ANJUTA_TASK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ANJUTA_TYPE_TASK, AnjutaTaskClass))

typedef struct _AnjutaTaskClass AnjutaTaskClass;
typedef struct _AnjutaTask AnjutaTask;
typedef struct _AnjutaTaskPriv AnjutaTaskPriv;

struct _AnjutaTaskClass
{
	GObjectClass parent_class;
	
	/* Virtual Methods */
	guint (*run) (AnjutaTask *self);
	void (*start) (AnjutaTask *self);
	void (*cancel) (AnjutaTask *self);
	void (*notify_data_arrived) (AnjutaTask *self);
	void (*notify_finished) (AnjutaTask *self);
	void (*notify_progress) (AnjutaTask *self, gfloat progress);
	gboolean (*start_automatic_monitor) (AnjutaTask *self);
	void (*stop_automatic_monitor) (AnjutaTask *self);
	
	/* Signals */
	void (*data_arrived) (AnjutaTask *task);
	void (*started) (AnjutaTask *task);
	void (*finished) (AnjutaTask *task);
	void (*progress) (AnjutaTask *task, gfloat progress);

};

struct _AnjutaTask
{
	GObject parent_instance;
	
	AnjutaTaskPriv *priv;
};

GType anjuta_task_get_type (void) G_GNUC_CONST;

void anjuta_task_start (AnjutaTask *self);
void anjuta_task_cancel (AnjutaTask *self);
void anjuta_task_notify_data_arrived (AnjutaTask *self);
void anjuta_task_notify_finished (AnjutaTask *self);
void anjuta_task_notify_progress (AnjutaTask *self, gfloat progress);
gboolean anjuta_task_is_running (AnjutaTask *self);

gboolean anjuta_task_start_automatic_monitor (AnjutaTask *self);
void anjuta_task_stop_automatic_monitor (AnjutaTask *self);

G_END_DECLS

#endif /* _ANJUTA_TASK_H_ */
