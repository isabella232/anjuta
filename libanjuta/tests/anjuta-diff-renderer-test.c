/*
 * anjuta-diff-renderer-test.c
 *
 * Copyright (C) 2013 - James Liggett
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libanjuta/anjuta-cell-renderer-diff.h>

enum
{
	COL_DIFF,

	NUM_COLS
};

static void
on_window_destroy (GtkWidget *window, gpointer user_data)
{
	gtk_main_quit ();
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkWidget *tree_view;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkListStore *list_store;
	GtkTreeIter iter;
	const gchar diff[] = 
		"diff --git a/libanjuta/Makefile.am b/libanjuta/Makefile.am\n"
		"index 4ac227e..d47c978 100644\n"
		"--- a/libanjuta/Makefile.am\n"
		"+++ b/libanjuta/Makefile.am\n"
		"@@ -138,7 +138,9 @@ libanjuta_3_la_SOURCES= \\\n"
		"        anjuta-close-button.c \\\n"
		"        anjuta-close-button.h \\\n"
		"        anjuta-modeline.c \\\n"
		"-       anjuta-modeline.h\n"
		"+       anjuta-modeline.h \\\n"
		"+       anjuta-cell-renderer-diff.c \\\n"
		"+       anjuta-cell-renderer-diff.h\n"
		"\n"
		"# Glade module\n"
		"if ENABLE_GLADE_CATALOG\n";
	
	const gchar multi_file_diff[] =
		"diff --git a/AUTHORS b/AUTHORS\n"
		"index 9f20595..7f6ead6 100644\n"
		"--- a/AUTHORS\n"
		"+++ b/AUTHORS\n"
		"@@ -11,7 +11,7 @@ Maintainers and Lead Developers:\n"
		" Developers:\n"
		"-------------------------------------------------------------------------------\n"
		" 	Massimo Cora'  <maxcvs@email.it> (Italy)\n"
		"-	Carl-Anton Ingmarsson <ca.ingmarsson@gmail.com>\n"	
		"+	Carl-Anton Ingmarsson <carlantoni@gnome.org> (Sweden)\n"
 		"\n"
		" Past Developers:\n"
		"-------------------------------------------------------------------------------\n"
		"diff --git a/plugins/git/git-clone-command.c b/plugins/git/git-clone-command.c\n"
		"index 8fbb96a..f1665e2 100644\n"
		"--- a/plugins/git/git-clone-command.c\n"
		"+++ b/plugins/git/git-clone-command.c\n"
		"@@ -1,7 +1,7 @@\n"
		" /* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */\n"
		" /*\n"
		"  * anjuta\n"
		"- * Copyright (C) Carl-Anton Ingmarsson 2009 <ca.ingmarsson@gmail.com>\n"
		"+ * Copyright (C) Carl-Anton Ingmarsson 2009 <carlantoni@gnome.org>\n"
		"  *\n" 
		"  * anjuta is free software.\n"
		"  *\n";

	const gchar non_diff[] = "non-diff text";
	const gchar hunk_line[] = "@@";
	const gchar broken_hunk[] = "@";

	gtk_init (&argc, &argv);
		

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Diff renderer test");

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	tree_view = gtk_tree_view_new ();

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

	renderer = anjuta_cell_renderer_diff_new ();
	column = gtk_tree_view_column_new ();

	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "diff", COL_DIFF);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
	gtk_container_add (GTK_CONTAINER (window), scrolled_window);

	list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);

	/* Test general diffs */
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, diff, -1);

	/* Test multi-file diffs */
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, multi_file_diff, -1);

	/* Test non-diff text */
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, non_diff, -1);

	/* Test hunk line detection */
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, hunk_line, -1);

	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, broken_hunk, -1);

	/* Test empty and NULL strings */
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, "", -1);

	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter, COL_DIFF, NULL, -1);

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), 
	                         GTK_TREE_MODEL (list_store));

	g_signal_connect (G_OBJECT (window), "destroy",
	                  G_CALLBACK (on_window_destroy),
	                  NULL);

	gtk_widget_set_size_request (window, 650, 400);

	gtk_widget_show_all (window);
	gtk_main();

	return 0;
}