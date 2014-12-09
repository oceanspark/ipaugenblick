#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_common.h"
#include <string.h>

void *memory = NULL;

int main(int argc,char **argv)
{
    struct ipaugenblick_buffer_desc *buff,*buff2;
    ipaugenblick_cmd_t *cmd;
    char *p;
    int size = 0,ringset_idx;
    if((memory = ipaugenblick_app_init(&size))== NULL) {
        printf("cannot initialize memory\n");
        return 0;
    } 

    while(1) {
        buff = ipaugenblick_get_free_command_buf(memory);
        if(!buff)
            goto rtx_bufs;
        cmd = (ipaugenblick_cmd_t *)buff;
        cmd->cmd = IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND;
        cmd->u.open_client_sock.my_ipaddress = 0xA5A57F7F;
        cmd->u.open_client_sock.my_port = 0x1234;
        cmd->u.open_client_sock.peer_ipaddress = 0x7F7FA5A5;
        cmd->u.open_client_sock.peer_port = 0x5678;
        printf("%s %d\n",__FILE__,__LINE__);
        ipaugenblick_submit_command_buf(memory,buff);
rtx_bufs:
        for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) {
            buff = ipaugenblick_get_tx_free_buf(memory,ringset_idx);
            if(!buff)
                continue;
            p = (char *)buff;
            sprintf(p,"CLIENT#%d\n",ringset_idx);
            ipaugenblick_submit_tx_buf(memory,buff,ringset_idx);
        }
        for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) {
            buff = ipaugenblick_dequeue_rx_buf(memory,ringset_idx);
            if(!buff) {
                continue;
            }
            p = (char *)buff;
            printf("received %s\n",p);
            ipaugenblick_free_rx_buf(memory,buff,ringset_idx);
        }
    }
    return 0;
}