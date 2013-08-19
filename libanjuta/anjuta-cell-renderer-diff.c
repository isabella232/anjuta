/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * anjuta-cell-renderer-diff.c
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

#include "anjuta-cell-renderer-diff.h"

struct _AnjutaCellRendererDiffPrivate
{
	GtkCellRenderer *text_cell;
};


enum
{
	PROP_0,

	PROP_DIFF
};



G_DEFINE_TYPE (AnjutaCellRendererDiff, anjuta_cell_renderer_diff, GTK_TYPE_CELL_RENDERER);

static void
anjuta_cell_renderer_diff_init (AnjutaCellRendererDiff *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, ANJUTA_TYPE_CELL_RENDERER_DIFF, 
	                                          AnjutaCellRendererDiffPrivate);

	self->priv->text_cell = gtk_cell_renderer_text_new ();
}

static void
anjuta_cell_renderer_diff_finalize (GObject *object)
{
	AnjutaCellRendererDiff *self;

	self = ANJUTA_CELL_RENDERER_DIFF (object);

	g_object_unref (self->priv->text_cell);

	G_OBJECT_CLASS (anjuta_cell_renderer_diff_parent_class)->finalize (object);
}

static void
anjuta_cell_renderer_diff_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ANJUTA_IS_CELL_RENDERER_DIFF (object));

	switch (prop_id)
	{
	case PROP_DIFF:
		anjuta_cell_renderer_diff_set_diff (ANJUTA_CELL_RENDERER_DIFF (object),
		                                    g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
anjuta_cell_renderer_diff_render (GtkCellRenderer *cell,
                                  cairo_t *cr,
                                  GtkWidget *widget,
                                  const GdkRectangle *background_area,
                                  const GdkRectangle *cell_area,
                                  GtkCellRendererState flags)
{
	AnjutaCellRendererDiff *self;

	self = ANJUTA_CELL_RENDERER_DIFF (cell);

	gtk_cell_renderer_render (self->priv->text_cell, cr, widget, 
	                          background_area, cell_area, flags);
}

static void
anjuta_cell_renderer_diff_get_preferred_width (GtkCellRenderer *cell,
                                               GtkWidget *widget,
                                               gint *minimum,
                                               gint *natural)
{
	AnjutaCellRendererDiff *self;

	self = ANJUTA_CELL_RENDERER_DIFF (cell);

	gtk_cell_renderer_get_preferred_width (self->priv->text_cell, widget, 
	                                       minimum, natural);
}

static void
anjuta_cell_renderer_diff_get_preferred_height (GtkCellRenderer *cell,
                                                GtkWidget *widget,
                                                gint *minimum,
                                                gint *natural)
{
	AnjutaCellRendererDiff *self;

	self = ANJUTA_CELL_RENDERER_DIFF (cell);

	gtk_cell_renderer_get_preferred_height (self->priv->text_cell, widget, 
	                                        minimum, natural);
}

static void
anjuta_cell_renderer_diff_class_init (AnjutaCellRendererDiffClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkCellRendererClass *parent_class = GTK_CELL_RENDERER_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AnjutaCellRendererDiffPrivate));

	object_class->finalize = anjuta_cell_renderer_diff_finalize;
	object_class->set_property = anjuta_cell_renderer_diff_set_property;
	parent_class->render = anjuta_cell_renderer_diff_render;
	parent_class->get_preferred_width = anjuta_cell_renderer_diff_get_preferred_width;;
	parent_class->get_preferred_height = anjuta_cell_renderer_diff_get_preferred_height;

	g_object_class_install_property (object_class,
	                                 PROP_DIFF,
	                                 g_param_spec_string ("diff",
	                                                      "diff",
	                                                      "Diff to render",
	                                                      "",
	                                                      G_PARAM_WRITABLE));
}


static PangoAttrList *
create_attribute_list (const gchar *diff)
{
	PangoAttrList *list;
	const gchar *line_begin, *line_end;
	guint begin_index, end_index;
	gboolean found_diff = FALSE;
	gboolean found_hunk = FALSE;
	PangoAttribute *attribute;

	list = pango_attr_list_new ();

	/* Make all of the text monospace */
	pango_attr_list_insert (list, pango_attr_family_new ("Monospace"));

	line_begin = diff;

	while (line_begin && *line_begin)
	{
		line_end = strchr (line_begin, '\n');

		if (!line_end)
			line_end = diff + strlen (line_begin);

		begin_index = line_begin - diff;
		end_index = line_end - diff;
		attribute = NULL;

		/* Handle multiple files. Context lines should start with a 
		 * whitespace, so just searching the first few characters 
		 * of the line for "diff" should detect the next file */
		if (g_str_has_prefix (line_begin, "diff"))
		{
			found_diff = TRUE;
			found_hunk = FALSE;
		}

		if (line_begin[0] == '@' && line_begin[1] == '@')
		{
			/* Dark blue */
			attribute = pango_attr_foreground_new (0, 0, 0x8000);

			/* Don't decorate diff headers */
			found_hunk = TRUE;
		}
		else if (found_hunk)
		{
			if (line_begin[0] == '+')
			{
				/* Dark green */
				attribute = pango_attr_foreground_new (0, 0x8000, 0);
			}
			else if (line_begin[0] == '-')
			{
				/* Red */
				attribute = pango_attr_foreground_new (0xffff, 0, 0);	
			}
		}
		else if (found_diff)
		{
			/* Make file headers easier to see by making them bold */
			attribute = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
		}
			
		if (attribute)
		{
			attribute->start_index = begin_index;
			attribute->end_index = end_index;

			pango_attr_list_insert (list, attribute);
		}

		if (*line_end)
			line_begin = line_end + 1;
		else
			line_begin = NULL;
	}

	return list;
}

void
anjuta_cell_renderer_diff_set_diff (AnjutaCellRendererDiff *self, 
                                    const gchar *diff)
{
	PangoAttrList *attributes = NULL;

	if (diff)
		attributes = create_attribute_list (diff);

	g_object_set (G_OBJECT (self->priv->text_cell),
	              "attributes", attributes, 
	              "text", diff, 
	              NULL);

	pango_attr_list_unref (attributes);
}

GtkCellRenderer *
anjuta_cell_renderer_diff_new (void)
{
	return g_object_new (ANJUTA_TYPE_CELL_RENDERER_DIFF, NULL);
}

