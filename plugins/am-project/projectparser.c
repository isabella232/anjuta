/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * projectparser.c
 * Copyright (C) Sébastien Granjoux 2009 <seb.sfo@free.fr>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "am-project.h"
#include "libanjuta/anjuta-debug.h"
#include "libanjuta/anjuta-project.h"
#include "libanjuta/interfaces/ianjuta-project.h"

#include <gio/gio.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static gchar* output_file = NULL;
static FILE* output_stream = NULL;

static GOptionEntry entries[] =
{
  { "output", 'o', 0, G_OPTION_ARG_FILENAME, &output_file, "Output file (default stdout)", "output_file" },
  { NULL }
};

#define INDENT 4

static void
open_output (void)
{
	output_stream = output_file == NULL ? stdout : fopen (output_file, "wt");
}

static void
close_output (void)
{
	if (output_stream != NULL) fclose (output_stream);
	output_stream = NULL;
}

static void
print (const gchar *message, ...)
{
	va_list args;

	if (output_stream == NULL) open_output();
	
	va_start (args, message);
	vfprintf (output_stream, message, args);
	va_end (args);
	fputc('\n', output_stream);
}

static void list_children (IAnjutaProject *project, AnjutaProjectNode *parent, gint indent, const gchar *path);

static void
list_source (IAnjutaProject *project, AnjutaProjectNode *source, gint indent, const gchar *path)
{
	GFile *file;
	GFile *root;
	gchar *rel_path;

	if (source == NULL) return;
	
	root = anjuta_project_node_get_file (ianjuta_project_get_root (project, NULL));
	
	file = anjuta_project_node_get_file (source);
	rel_path = g_file_get_relative_path (root, file);
	print ("%*sSOURCE (%s): %s", indent * INDENT, "", path, rel_path);
	g_free (rel_path);
}

static void
list_target (IAnjutaProject *project, AnjutaProjectNode *target, gint indent, const gchar *path)
{
	print ("%*sTARGET (%s): %s", indent * INDENT, "", path, anjuta_project_node_get_name (target)); 

	list_children (project, target, indent, path);
}

static void
list_group (IAnjutaProject *project, AnjutaProjectNode *group, gint indent, const gchar *path)
{
	AnjutaProjectNode *parent;
	gchar *rel_path;
	
	parent = anjuta_project_node_parent (group);
	if (anjuta_project_node_get_type (parent) == ANJUTA_PROJECT_ROOT)
	{
		GFile *root;
		
		root = g_file_get_parent (anjuta_project_node_get_file (parent));
		rel_path = g_file_get_relative_path (root, anjuta_project_group_get_directory (group));
		g_object_unref (root);
	}
	else
	{
		GFile *root;
		root = anjuta_project_node_get_file (parent);
		rel_path = g_file_get_relative_path (root, anjuta_project_group_get_directory (group));
	}
	print ("%*sGROUP (%s): %s", indent * INDENT, "", path, rel_path);
	g_free (rel_path);

	list_children (project, group, indent, path);
}

static void
list_package (IAnjutaProject *project, AnjutaProjectNode *package, gint indent, const gchar *path)
{
	print ("%*sPACKAGE (%s): %s", indent * INDENT, "", path, anjuta_project_node_get_name (package)); 
}

static void
list_module (IAnjutaProject *project, AnjutaProjectNode *module, gint indent, const gchar *path)
{
	print ("%*sMODULE (%s): %s", indent * INDENT, "", path, anjuta_project_node_get_name (module));

	list_children (project, module, indent, path);
}

static void
list_property (IAnjutaProject *project)
{
	if (AMP_IS_PROJECT (project))
	{
		AnjutaProjectProperty *list;
		AnjutaProjectProperty *prop;

		list = amp_project_get_property_list (AMP_PROJECT (project));
		for (prop = anjuta_project_property_first (list); prop != NULL; prop = anjuta_project_property_next (prop))
		{
			AnjutaProjectProperty *item;

			item = anjuta_project_property_override (list, prop);
			if (item != NULL)
			{
				AnjutaProjectPropertyInfo *info;
				const gchar *msg = NULL;

				info = anjuta_project_property_get_info(item);
				if (strcmp (info->name, "Name:") == 0)
				{
					msg = "%*sNAME: %s";
				}
				else if (strcmp (info->name, "Version:") == 0)
				{
					msg = "%*sVERSION: %s";
				}
				else if (strcmp (info->name, "Bug report URL:") == 0)
				{
					msg = "%*sBUG_REPORT: %s";
				}
				else if (strcmp (info->name, "Package name:") == 0)
				{
					msg = "%*sTARNAME: %s";
				}
				else if (strcmp (info->name, "URL:") == 0)
				{
					msg = "%*sURL: %s";
				}

				if (msg && (info->value != NULL)) print (msg, INDENT, "", info->value);
			}
		}
	}
}

static void
list_children (IAnjutaProject *project, AnjutaProjectNode *parent, gint indent, const gchar *path)
{
	AnjutaProjectNode *node;
	guint count;

	indent++;

	count = 0;
	for (node = anjuta_project_node_first_child (parent); node != NULL; node = anjuta_project_node_next_sibling (node))
	{
		if (anjuta_project_node_get_type (node) == ANJUTA_PROJECT_MODULE)
		{
			gchar *child_path = g_strdup_printf ("%s%s%d", path != NULL ? path : "", path != NULL ? ":" : "", count);
			list_module (project, node, indent, child_path);
			g_free (child_path);
		}
		count++;
	}

	count = 0;
	for (node = anjuta_project_node_first_child (parent); node != NULL; node = anjuta_project_node_next_sibling (node))
	{
		if (anjuta_project_node_get_type (node) == ANJUTA_PROJECT_PACKAGE)
		{
			gchar *child_path = g_strdup_printf ("%s%s%d", path != NULL ? path : "", path != NULL ? ":" : "", count);
			list_package (project, node, indent, child_path);
			g_free (child_path);
		}
		count++;
	}
	
	count = 0;
	for (node = anjuta_project_node_first_child (parent); node != NULL; node = anjuta_project_node_next_sibling (node))
	{
		if (anjuta_project_node_get_type (node) == ANJUTA_PROJECT_GROUP)
		{
			gchar *child_path = g_strdup_printf ("%s%s%d", path != NULL ? path : "", path != NULL ? ":" : "", count);
			list_group (project, node, indent, child_path);
			g_free (child_path);
		}
		count++;
	}
	
	count = 0;
	for (node = anjuta_project_node_first_child (parent); node != NULL; node = anjuta_project_node_next_sibling (node))
	{
		if (anjuta_project_node_get_type (node) == ANJUTA_PROJECT_TARGET)
		{
			gchar *child_path = g_strdup_printf ("%s%s%d", path != NULL ? path : "", path != NULL ? ":" : "", count);
			list_target (project, node, indent, child_path);
			g_free (child_path);
		}
		count++;
	}
	
	count = 0;
	for (node = anjuta_project_node_first_child (parent); node != NULL; node = anjuta_project_node_next_sibling (node))
	{
		if (anjuta_project_node_get_type (node) == ANJUTA_PROJECT_SOURCE)
		{
			gchar *child_path = g_strdup_printf ("%s%s%d", path != NULL ? path : "", path != NULL ? ":" : "", count);
			list_source (project, node, indent, child_path);
			g_free (child_path);
		}
		count++;
	}
}

static void
list_root (IAnjutaProject *project, AnjutaProjectNode *root)
{
	list_children (project, root, 0, NULL);
}

static AnjutaProjectNode *
get_node (IAnjutaProject *project, const char *path)
{
	AnjutaProjectNode *node = NULL;

	if (path != NULL)
	{
		for (; *path != '\0';)
		{
			gchar *end;
			guint child = g_ascii_strtoull (path, &end, 10);

			if (end == path)
			{
				/* error */
				return NULL;
			}

			if (node == NULL)
			{
				if (child == 0) node = ianjuta_project_get_root (project, NULL);
			}
			else
			{
				node = anjuta_project_node_nth_child (node, child);
			}
			if (node == NULL)
			{
				/* no node */
				return NULL;
			}

			if (*end == '\0') break;
			path = end + 1;
		}
	}

	return node;
}

static GFile *
get_file (AnjutaProjectNode *target, const char *id)
{
	AnjutaProjectNode *group = (AnjutaProjectNode *)anjuta_project_node_parent (target);
	
	return g_file_resolve_relative_path (anjuta_project_group_get_directory (group), id);
}

static AnjutaProjectTargetType
get_type (IAnjutaProject *project, const char *id)
{
	AnjutaProjectTargetType type;
	GList *list;
	GList *item;
	guint num = atoi (id);
	
	list = ianjuta_project_get_target_types (project, NULL);
	type = (AnjutaProjectTargetType)list->data;
	for (item = list; item != NULL; item = g_list_next (item))
	{
		if (num == 0)
		{
			type = (AnjutaProjectTargetType)item->data;
			break;
		}
		num--;
	}
	g_list_free (list);

	return type;
}

static AnjutaProjectProperty *
get_project_property (AmpProject *project, const gchar *id)
{
	AnjutaProjectProperty *list;
	AnjutaProjectProperty *item;
	AnjutaProjectProperty *prop = NULL;
	gint best = G_MAXINT;

	list = amp_project_get_property_list (project);
	for (item = anjuta_project_property_first (list); item != NULL; item = anjuta_project_property_next (item))
	{
		AnjutaProjectPropertyInfo *info = anjuta_project_property_get_info (item);
		const gchar *name = info->name;
		const gchar *ptr;
		const gchar *iptr = id;
		gboolean next = FALSE;
		gint miss = 0;

		for (ptr = name; *ptr != '\0'; ptr++)
		{
				if (!next && (*iptr != '\0') && (g_ascii_toupper (*ptr) == g_ascii_toupper (*iptr)))
				{
					iptr++;
				}
				else
				{
					miss++;
					next = !g_ascii_isspace (*ptr);
				}
		}

		if ((*iptr == '\0') && (miss < best))
		{
			best = miss;
			prop = item;
		}
	}

	return prop;
}

/* Automake parsing function
 *---------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
	IAnjutaProject *project;
	AnjutaProjectNode *node;
	AnjutaProjectNode *sibling;
	AnjutaProjectNode *root = NULL;
	char **command;
	GOptionContext *context;
	GError *error = NULL;

	/* Initialize program */
	g_type_init ();
	
	anjuta_debug_init ();

	/* Parse options */
 	context = g_option_context_new ("list [args]");
  	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_set_summary (context, "test new autotools project manger");
	if (!g_option_context_parse (context, &argc, &argv, &error))
    {
		exit (1);
    }
	if (argc < 2)
	{
		printf ("PROJECT: %s", g_option_context_get_help (context, TRUE, NULL));
		exit (1);
	}

	/* Execute commands */
	for (command = &argv[1]; *command != NULL; command++)
	{
		if (g_ascii_strcasecmp (*command, "load") == 0)
		{
			GFile *file = g_file_new_for_commandline_arg (*(++command));

			if (project == NULL)
			{
				gint best = 0;
				gint probe;
				GType type;
				
				/* Check for project type */
				probe = amp_project_probe (file, NULL);
				if (probe > best)
				{
					best = probe;
					type = AMP_TYPE_PROJECT;
				}

				if (best == 0)
				{
					fprintf (stderr, "Error: No backend for loading project in %s\n", *command);
					break;
				}
				else
				{
					project = IANJUTA_PROJECT (g_object_new (type, NULL));
				}
			}

			root = ianjuta_project_new_root_node (project, file, &error);
			root = ianjuta_project_load_node (project, root, &error);
			g_object_unref (file);
		}
		else if (g_ascii_strcasecmp (*command, "list") == 0)
		{
			list_property (project);
			
			list_root (project, root);
		}
		else if (g_ascii_strcasecmp (*command, "move") == 0)
		{
			if (AMP_IS_PROJECT (project))
			{
				amp_project_move (AMP_PROJECT (project), *(++command));
			}
		}
		else if (g_ascii_strcasecmp (*command, "save") == 0)
		{
			ianjuta_project_save_node (project, root, NULL);
		}
		else if (g_ascii_strcasecmp (*command, "remove") == 0)
		{
			node = get_node (project, *(++command));
			ianjuta_project_remove_node (project, node, NULL);
		}
		else if (g_ascii_strcasecmp (command[0], "add") == 0)
		{
			node = get_node (project, command[2]);
			if (g_ascii_strcasecmp (command[1], "group") == 0)
			{
				if ((command[4] != NULL) && (g_ascii_strcasecmp (command[4], "before") == 0))
				{
					sibling = get_node (project, command[5]);
					amp_project_add_sibling_group (project, node, command[3], FALSE, sibling, NULL);
					command += 2;
				}
				else if ((command[4] != NULL) && (g_ascii_strcasecmp (command[4], "after") == 0))
				{
					sibling = get_node (project, command[5]);
					amp_project_add_sibling_group (project, node, command[3], TRUE, sibling, NULL);
					command += 2;
				}
				else
				{
					ianjuta_project_add_group (project, node, command[3], NULL);
				}
			}
			else if (g_ascii_strcasecmp (command[1], "target") == 0)
			{
				if ((command[5] != NULL) && (g_ascii_strcasecmp (command[5], "before") == 0))
				{
					sibling = get_node (project, command[6]);
					amp_project_add_sibling_target (project, node, command[3], get_type (project, command[4]), FALSE, sibling, NULL);
					command += 2;
				}
				else if ((command[5] != NULL) && (g_ascii_strcasecmp (command[5], "after") == 0))
				{
					sibling = get_node (project, command[6]);
					amp_project_add_sibling_target (project, node, command[3], get_type (project, command[4]), TRUE, sibling, NULL);
					command += 2;
				}
				else
				{
					ianjuta_project_add_target (project, node, command[3], get_type (project, command[4]), NULL);
				}
				command++;
			}
			else if (g_ascii_strcasecmp (command[1], "source") == 0)
			{
				GFile *file = get_file (node, command[3]);

				if ((command[4] != NULL) && (g_ascii_strcasecmp (command[4], "before") == 0))
				{
					sibling = get_file (project, command[5]);
					amp_project_add_sibling_source (project, node, file, FALSE, sibling, NULL);
					command += 2;
				}
				else if ((command[4] != NULL) && (g_ascii_strcasecmp (command[4], "after") == 0))
				{
					sibling = get_node (project, command[5]);
					amp_project_add_sibling_source (project, node, file, TRUE, sibling, NULL);
					command += 2;
				}
				else
				{
					ianjuta_project_add_source (project, node, file, NULL);
				}
				g_object_unref (file);
			}
			else
			{
				fprintf (stderr, "Error: unknown command %s\n", *command);

				break;
			}
			command += 3;
		}
		else if (g_ascii_strcasecmp (command[0], "set") == 0)
		{
			if (AMP_IS_PROJECT (project))
			{
				AnjutaProjectProperty *item;
				AnjutaProjectPropertyInfo *info = NULL;

				item = get_project_property (AMP_PROJECT (project), command[1]);
				if (item != NULL) amp_project_property_set (AMP_PROJECT (project), item, command[2]);
			}
			command += 2;
		}
		else
		{
			fprintf (stderr, "Error: unknown command %s\n", *command);

			break;
		}
		if (error != NULL)
		{
			fprintf (stderr, "Error: %s\n", error->message == NULL ? "unknown error" : error->message);

			g_error_free (error);
			break;
		}
	}

	/* Free objects */
	if (project) g_object_unref (project);
	close_output ();
	
	return (0);
}