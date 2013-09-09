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


/* Line types */
typedef enum
{
	LINE_TYPE_HEADER,
	LINE_TYPE_HUNK_HEADER,
	LINE_TYPE_ADD,
	LINE_TYPE_DELETE,
	LINE_TYPE_CONTEXT
} LineType;

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
	LineType type;

	list = pango_attr_list_new ();

	/* Make all of the text monospace */
	pango_attr_list_insert (list, pango_attr_family_new ("Monospace"));

	/* Assume that diff points to one line of a unified diff */
	type = LINE_TYPE_CONTEXT;
	
	if (diff && diff[0])
	{
		if (*diff != ' ')
		{
			if (diff[0] == '@' && diff[1] == '@')
				type = LINE_TYPE_HUNK_HEADER;
			else if (diff[0] == '+')
			{
				if (g_str_has_prefix (diff, "+++ "))
				    type = LINE_TYPE_HEADER;
				else
					type = LINE_TYPE_ADD;
			}
			else if (diff[0] == '-')
			{
				if (g_str_has_prefix (diff, "--- "))
					type = LINE_TYPE_HEADER;
				else
					type = LINE_TYPE_DELETE;
			}
			else
				type = LINE_TYPE_HEADER;
		}
	}

	switch (type)
	{
		case LINE_TYPE_HEADER:
			/* Make file headers easier to see by making them bold */
			pango_attr_list_insert (list, 
			                        pango_attr_weight_new (PANGO_WEIGHT_BOLD));
			break;
		case LINE_TYPE_HUNK_HEADER:
			/* Dark blue */
			pango_attr_list_insert (list, 
			                        pango_attr_foreground_new (0, 0, 0x8000));
			break;
		case LINE_TYPE_ADD:
			/* Dark green */
			pango_attr_list_insert (list, 
			                        pango_attr_foreground_new (0, 0x8000, 0));
			break;
		case LINE_TYPE_DELETE:
			/* Red */
			pango_attr_list_insert (list, 
			                        pango_attr_foreground_new (0xffff, 0, 0));
			break;
		default:
			break;
	};

	return list;
}

void
anjuta_cell_renderer_diff_set_diff (AnjutaCellRendererDiff *self, 
                                    const gchar *diff)
{
	PangoAttrList *attributes = NULL;
	gchar *newline;
	gchar *diff_without_newline = NULL;

	if (diff)
	{
		newline = strchr (diff, '\n');

		if (newline)
		{
			diff_without_newline = g_strndup (diff, newline - diff);
			g_object_set (G_OBJECT (self->priv->text_cell),
			                        "text", diff_without_newline,
			                        NULL);
			g_free (diff_without_newline);
		}
		else
		{
			g_object_set (G_OBJECT (self->priv->text_cell),
			              "text", diff,
			              NULL);
		}

		attributes = create_attribute_list (diff);

		g_object_set (G_OBJECT (self->priv->text_cell),
	          		  "attributes", attributes,
		              NULL);

		pango_attr_list_unref (attributes);
	}
	else
	{
		g_object_set (G_OBJECT (self->priv->text_cell),
		              "text", "", NULL);
	}
}

GtkCellRenderer *
anjuta_cell_renderer_diff_new (void)
{
	return g_object_new (ANJUTA_TYPE_CELL_RENDERER_DIFF, NULL);
}

