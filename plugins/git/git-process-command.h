/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta
 * Copyright (C) James Liggett 2008 <jrliggett@cox.net>
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

#ifndef _GIT_PROCESS_COMMAND_H_
#define _GIT_PROCESS_COMMAND_H_

#include <glib-object.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <libanjuta/anjuta-sync-command.h>
#include <libanjuta/anjuta-launcher.h>
#include <libanjuta/anjuta-utils.h>

G_BEGIN_DECLS

#define GIT_TYPE_PROCESS_COMMAND             (git_process_command_get_type ())
#define GIT_PROCESS_COMMAND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIT_TYPE_PROCESS_COMMAND, GitProcessCommand))
#define GIT_PROCESS_COMMAND_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIT_TYPE_PROCESS_COMMAND, GitProcessCommandClass))
#define GIT_IS_PROCESS_COMMAND(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIT_TYPE_PROCESS_COMMAND))
#define GIT_IS_PROCESS_COMMAND_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIT_TYPE_PROCESS_COMMAND))
#define GIT_PROCESS_COMMAND_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIT_TYPE_PROCESS_COMMAND, GitProcessCommandClass))

typedef struct _GitProcessCommandClass GitProcessCommandClass;
typedef struct _GitProcessCommand GitProcessCommand;
typedef struct _GitProcessCommandPriv GitProcessCommandPriv;

struct _GitProcessCommandClass
{
	AnjutaSyncCommandClass parent_class;
	
	/* Virtual methods */
	void (*output_handler) (GitProcessCommand *git_process_command, const gchar *output);
	void (*error_handler) (GitProcessCommand *git_process_command, const gchar *output);
};

struct _GitProcessCommand
{
	AnjutaSyncCommand parent_instance;
	
	GitProcessCommandPriv *priv;
};

GType git_process_command_get_type (void) G_GNUC_CONST;
void git_process_command_add_arg (GitProcessCommand *self, const gchar *arg);
void git_process_command_add_list_to_args (GitProcessCommand *self, GList *list);
void git_process_command_append_error (GitProcessCommand *self, const gchar *error_line);
void git_process_command_push_info (GitProcessCommand *self, const gchar *info);
GQueue *git_process_command_get_info_queue (GitProcessCommand *self);

/* Generic output handlers */
void git_process_command_send_output_to_info (GitProcessCommand *git_command, 
									  const gchar *output);
									  
/* Static helper methods */
GList *git_process_command_copy_string_list (GList *path_list);

/* Password handling */
void git_process_command_set_check_passwd_prompt (GitProcessCommand *self, 
                                                  gboolean check);

G_END_DECLS

#endif /* _GIT_PROCESS_COMMAND_H_ */
