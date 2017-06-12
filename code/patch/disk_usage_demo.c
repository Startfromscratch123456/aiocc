#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <guestfs.h>
#define STRNEQ(a, b) (strcmp((a),(b)) != 0)
int main (int argc, char *argv[])
{
    guestfs_h *g;
    char *dom;
    int i;
    char **devices = NULL;
    char **fses = NULL;
    if (argc != 2) {
        exit (EXIT_FAILURE);
    }
    dom = argv[1];
    g = guestfs_create ();
    if (g == NULL) {
        perror ("failed to create libguestfs handle");
        exit (EXIT_FAILURE);
    }
    guestfs_add_domain(g,(const char *)dom,GUESTFS_ADD_DOMAIN_READONLY,1,-1);
    //guestfs_add_domain(g,(const char *)dom,GUESTFS_ADD_DOMAIN_LIVE,1,-1);
    guestfs_set_identifier (g,dom);
    guestfs_set_trace (g, 0);
    guestfs_set_verbose (g,0); 
    if (guestfs_launch (g) == -1) {
        perror ("dengshijun:guestfs_launch\n");
        return -1;
    }

    devices = guestfs_list_devices (g);
    if (devices == NULL){
        perror ("dengshijun:guesfs_list_devices(g) == NULL");
        return -1;
    }
    fses = guestfs_list_filesystems (g);
    if (fses == NULL){
        perror ("dengshijun:guestfs_list_filesystem(g)");
        return -1;
    }

    for (i = 0; fses[i] != NULL; i+= 2) {
        if (STRNEQ (fses[i+1], "") &&
                STRNEQ (fses[i+1], "swap") &&
                STRNEQ (fses[i+1], "unknown")) {
            char *dev = fses[i];
            struct guestfs_statvfs *stat = NULL;

            /* Try mounting and stating the device.  This might reasonably
             *                                  * fail, so don't show errors.
             *                                                                   */
            guestfs_push_error_handler (g, NULL, NULL);
            if (guestfs_mount_ro (g, dev, "/") == 0) {
                stat = guestfs_statvfs (g, "/");
                guestfs_umount_all (g);
            }
            guestfs_pop_error_handler (g);
            if (!stat){
                continue;
            }
            fprintf(stderr,"dengshijun:%s %d %d %.2lf%\n",dev,stat->blocks,stat->bfree,100.0*(stat->blocks-stat->bfree)/stat->blocks);
            free(stat);
        }
    }
    guestfs_close (g);
    free(devices);
    free(fses);
    return 0;
}
