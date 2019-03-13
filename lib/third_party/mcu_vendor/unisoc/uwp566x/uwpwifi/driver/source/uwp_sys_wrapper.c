#include <stdio.h>

#include "uwp_sys_wrapper.h"


void list_add_tail(struct list_head *node, struct list_head *list){
    struct list_head *p_node = list;
    while(p_node->next != list)
        p_node = p_node->next;
    __LIST_ADD(node,p_node,list);
}

int list_del_node(struct list_head *node, struct list_head *list){
    struct list_head *p_node = NULL;

    p_node = list->next;
    if(p_node == list){
        return -3; /* list is empty */
    }

    while((p_node != list) && (p_node != node)){
        p_node = p_node->next;
    }

    if(p_node != node){
        return -4; /* node doesn't exist */
    }

    p_node->next->prev = p_node->prev;
    p_node->prev->next = p_node->next;

    return 0;  
}

