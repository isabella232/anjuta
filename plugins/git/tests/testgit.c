#include <glib.h>
#include "../git-status-factory.h"

static void 
check_status (AnjutaVcsStatus index, AnjutaVcsStatus working_tree, 
              AnjutaVcsStatus expected_index, 
              AnjutaVcsStatus expected_working_tree,
              gchar *test_type)
              
{
	if (g_test_verbose ())
		g_print ("Testing %s\n", test_type);

	g_assert_cmpint (expected_index, ==, index);
	g_assert_cmpint (expected_working_tree, ==, working_tree);
}

static void
test_status (void)
{
	GitStatusFactory *factory;
	GitStatus *status;
	AnjutaVcsStatus working_tree, index;

	factory = git_status_factory_new ();
	status = git_status_factory_create_status (factory, 
											   " M plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_MODIFIED, 
	              "modified in working tree");

	status = git_status_factory_create_status (factory, 
											   "A  plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_ADDED, 
	              ANJUTA_VCS_STATUS_NONE, 
	              "added to index");

	status = git_status_factory_create_status (factory,
	                                           "D  plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_DELETED, 
	              ANJUTA_VCS_STATUS_NONE, 
	              "removed from index");

	status = git_status_factory_create_status (factory,
	                                           "MM plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_MODIFIED, 
	              ANJUTA_VCS_STATUS_MODIFIED, 
	              "modified in index and working tree");

	status = git_status_factory_create_status (factory,
	                                           "?? plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_UNVERSIONED, 
	              ANJUTA_VCS_STATUS_UNVERSIONED, 
	              "untracked");

	status = git_status_factory_create_status (factory,
	                                           "DD plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: both deleted");

	status = git_status_factory_create_status (factory,
	                                           "AU plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: added by us");

	status = git_status_factory_create_status (factory,
	                                           "UD plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: deleted by them");

	status = git_status_factory_create_status (factory,
	                                           "UA plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: added by them");

	status = git_status_factory_create_status (factory,
	                                           "DU plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: deleted by us");

	status = git_status_factory_create_status (factory,
	                                           "AA plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: both added");

	status = git_status_factory_create_status (factory,
	                                           "UU plugins/git/plugin.c");
	index = git_status_get_index_status (status);
	working_tree = git_status_get_working_tree_status (status);

	g_object_unref (status);
	check_status (index, working_tree, 
	              ANJUTA_VCS_STATUS_NONE, 
	              ANJUTA_VCS_STATUS_CONFLICTED, 
	              "conflict: both modified");
	
	g_object_unref (factory);
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/Git/Status", test_status);

	return g_test_run ();
}