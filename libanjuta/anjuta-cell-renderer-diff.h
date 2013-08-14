/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-cell-renderer-diff.h
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

#ifndef _ANJUTA_CELL_RENDERER_DIFF_H_
#define _ANJUTA_CELL_RENDERER_DIFF_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <string.h>

G_BEGIN_DECLS

#define ANJUTA_TYPE_CELL_RENDERER_DIFF             (anjuta_cell_renderer_diff_get_type ())
#define ANJUTA_CELL_RENDERER_DIFF(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ANJUTA_TYPE_CELL_RENDERER_DIFF, AnjutaCellRendererDiff))
#define ANJUTA_CELL_RENDERER_DIFF_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ANJUTA_TYPE_CELL_RENDERER_DIFF, AnjutaCellRendererDiffClass))
#define ANJUTA_IS_CELL_RENDERER_DIFF(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ANJUTA_TYPE_CELL_RENDERER_DIFF))
#define ANJUTA_IS_CELL_RENDERER_DIFF_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ANJUTA_TYPE_CELL_RENDERER_DIFF))
#define ANJUTA_CELL_RENDERER_DIFF_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ANJUTA_TYPE_CELL_RENDERER_DIFF, AnjutaCellRendererDiffClass))

typedef struct _AnjutaCellRendererDiffClass AnjutaCellRendererDiffClass;
typedef struct _AnjutaCellRendererDiff AnjutaCellRendererDiff;
typedef struct _AnjutaCellRendererDiffPrivate AnjutaCellRendererDiffPrivate;


struct _AnjutaCellRendererDiffClass
{
	GtkCellRendererClass parent_class;
};

struct _AnjutaCellRendererDiff
{
	GtkCellRenderer parent_instance;

	AnjutaCellRendererDiffPrivate *priv;
};

GType anjuta_cell_renderer_diff_get_type (void) G_GNUC_CONST;
void anjuta_cell_renderer_diff_set_diff (AnjutaCellRendererDiff *self, 
                                         const gchar *diff);
GtkCellRenderer *anjuta_cell_renderer_diff_new (void);

G_END_DECLS

#endif /* _ANJUTA_CELL_RENDERER_DIFF_H_ */

