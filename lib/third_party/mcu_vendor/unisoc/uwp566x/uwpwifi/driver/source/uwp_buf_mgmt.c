#include "uwp_buf_mgmt.h"
#include "mbed_retarget.h"

#define WIFI_LOG_DBG
#include "uwp_log.h"

static char wifi_reserve_ram[CONFIG_UWP_PKT_BUF_MAX * CONFIG_UWP_PKT_BUF_SIZE] __attribute__ ( (section(".SECTIONWIFIDATARAM")) );
static struct list_head wifi_buf_list_employ;
static struct list_head wifi_buf_list_free;
int wifi_rx_employ_num = 0;
int wifi_rx_free_num = CONFIG_UWP_PKT_BUF_MAX;
static void *wifi_list_mutex = NULL;
static uwp_pkt_buf *uwp_pkt_buf_entries[CONFIG_UWP_PKT_BUF_MAX];

/* test */
//#define UWP_WIFI_BUF_TEST

#ifdef UWP_WIFI_BUF_TEST
static uwp_pkt_buf list_test1;
static uwp_pkt_buf list_test2;
#endif

/*fail,ret<0*/
int uwp_pkt_buf_init(void){
    int i = 0;
    struct list_head *p_head = NULL;
    uwp_pkt_buf *p = NULL;

    LOG_DBG("WIFI RESERVE BUF:%p",wifi_reserve_ram);
    UWP_MUTEX_INIT(wifi_list_mutex);//wifi_list_mutex = k_mutex_create();

    if(wifi_list_mutex == NULL){//add the error code?
         LOG_ERR("Create wifi_list_mutex failed.");
         return -1; /* mutex create failed */
    }

    int ret = UWP_MUTEX_LOCK(wifi_list_mutex, 0);//int ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret == 0){
        LOG_ERR("wifi_list_mutex lock failed.");
        return -2; /* lock mutex failed */
    }

    __INIT_LIST_HEAD(&wifi_buf_list_employ);
    __INIT_LIST_HEAD(&wifi_buf_list_free);

    p_head = &wifi_buf_list_free;

#ifndef UWP_WIFI_BUF_TEST
    for(i = 0; i < CONFIG_UWP_PKT_BUF_MAX; i++){
#else
    for(i = 0; i < CONFIG_UWP_PKT_BUF_MAX - 1; i++){
#endif
        p = (uwp_pkt_buf *) UWP_MEM_ALLOC(sizeof(uwp_pkt_buf));
        if(p == NULL){
            LOG_ERR("malloc failed");
            return -ENOMEM;
        }
        uwp_pkt_buf_entries[i] = p;
        p->buf = wifi_reserve_ram + i * CONFIG_UWP_PKT_BUF_SIZE;

        //LOG_DBG("PREPARE BUFF:%p", p->buf);
        list_add_tail(&p->list, p_head);
    }

    ret = UWP_MUTEX_UNLOCK(wifi_list_mutex);//k_mutex_unlock(wifi_list_mutex);
    if(ret == 0){
        LOG_ERR("wifi_list_mutex unlock failed.");
        return -2; /* unlock mutex failed */
    }
    return i;
}
/*fail,ret=NULL*/
void *uwp_pkt_buf_get(void){
    struct list_head *p_head_free = &wifi_buf_list_free;
    struct list_head *p_head_employ = &wifi_buf_list_employ;
    struct list_head *p_node = NULL;
    int ret, ret1;

    ret1 = UWP_MUTEX_LOCK(wifi_list_mutex, UWP_NO_WAIT);//ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret1 == 0){
        LOG_ERR("wifi_list_mutex lock failed.");
        return NULL;
        //return -2; /* lock mutex failed */
    }

    p_node = p_head_free->next;
    ret = list_del_node(p_node, p_head_free);
    if(ret != 0){
        ret1 = UWP_MUTEX_UNLOCK(wifi_list_mutex);//k_mutex_unlock(wifi_list_mutex);
        if(ret1 == 0){
            LOG_ERR("wifi_list_mutex lock failed.");
            return NULL; /* lock mutex failed */
        }
        LOG_ERR("get pkt buf error:%d", ret);
        return NULL;
    }
    wifi_rx_free_num --;
    list_add_tail(p_node, p_head_employ);
    wifi_rx_employ_num ++;
    ret1 = UWP_MUTEX_UNLOCK(wifi_list_mutex);//k_mutex_unlock(wifi_list_mutex);
    if(ret1 == 0){
        LOG_ERR("wifi_list_mutex lock failed.");
        return NULL; /* lock mutex failed */
    }
    //LOG_DBG("GET BUF:%p", ((uwp_pkt_buf *)LIST_FIND_ENTRY(p_node, uwp_pkt_buf, list))->buf);
    return ((uwp_pkt_buf *)LIST_FIND_ENTRY(p_node, uwp_pkt_buf, list))->buf;
}

int uwp_pkt_buf_free(void *buf){
    struct list_head *p_head_free = &wifi_buf_list_free;
    struct list_head *p_head_employ = &wifi_buf_list_employ;
    struct list_head *p_del = NULL;
    int ret, ret1;

    ret1 = UWP_MUTEX_LOCK(wifi_list_mutex, UWP_NO_WAIT);//ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret1 == 0){
        LOG_ERR("wifi_list_mutex lock failed.");
        return -2; /* lock mutex failed */
    }

    p_del = p_head_free->next;
    while((p_del != p_head_free) && (((uwp_pkt_buf *)LIST_FIND_ENTRY(p_del, uwp_pkt_buf, list))->buf != buf))
        p_del = p_del->next;
    if( ((uwp_pkt_buf *)LIST_FIND_ENTRY(p_del, uwp_pkt_buf, list))->buf == buf ){
        LOG_DBG("multiple free\r\n");
        return UWP_MUTEX_UNLOCK(wifi_list_mutex);
    }

    p_del = p_head_employ->next;
    while((p_del != p_head_employ) && (((uwp_pkt_buf *)LIST_FIND_ENTRY(p_del, uwp_pkt_buf, list))->buf != buf))
        p_del = p_del->next;
    if(p_del == p_head_employ){
        LOG_ERR("buf to free is not on employ");
        for(int i = 0; i < CONFIG_UWP_PKT_BUF_MAX; i++){
            if(uwp_pkt_buf_entries[i]->buf == buf){
                        LOG_ERR("buf is legal");
                        p_del = &(uwp_pkt_buf_entries[i]->list);
                        goto ADD_FREE_TAIL;
                    }
         }

        LOG_ERR("free buf is illegal");
        return (UWP_MUTEX_UNLOCK(wifi_list_mutex) + -4);
    }

    ret = list_del_node(p_del, p_head_employ);
    if(ret != 0){
        LOG_DBG("node doesn't exist");
        ret1 = UWP_MUTEX_UNLOCK(wifi_list_mutex);//k_mutex_unlock(wifi_list_mutex);
        if(ret1 == 0){
            LOG_ERR("wifi_list_mutex unlock failed.");
            return -2; /* lock mutex failed */
        }
        return ret;
    }
    wifi_rx_employ_num --;

ADD_FREE_TAIL:
    list_add_tail(p_del, p_head_free);
    wifi_rx_free_num ++;
    //LOG_DBG("FREE BUF:%p", buf);
    ret1 = UWP_MUTEX_UNLOCK(wifi_list_mutex);//k_mutex_unlock(wifi_list_mutex);
    if(ret1 == 0){
        LOG_ERR("wifi_list_mutex lock failed.");
        return -2; /* lock mutex failed */
    }
    return 0;
}

#ifdef UWP_WIFI_BUF_TEST
void uwp_buf_self_test(void){

    LOG_DBG("%s enter", __func__);
    uwp_pkt_buf_init();
    void *p[CONFIG_UWP_PKT_BUF_MAX];
    for(int i = 0; i < CONFIG_UWP_PKT_BUF_MAX - 1; i++){
        p[i] = uwp_pkt_buf_get();
        LOG_DBG("get buf :%x", p[i]);
    }

    LOG_DBG("first free:%x", p[0]);
    uwp_pkt_buf_free(p[0]);

    LOG_DBG("second free:%x",p[0]);
    uwp_pkt_buf_free(p[0]);

    void *p1 = uwp_pkt_buf_get();
    LOG_DBG("get buf:%x",p1);

    /* to free not on employ but legal */
    void *p2 = (void *)(wifi_reserve_ram +  (CONFIG_UWP_PKT_BUF_MAX - 1) * CONFIG_UWP_PKT_BUF_SIZE);
    list_test1.buf = p2;
    uwp_pkt_buf_entries[CONFIG_UWP_PKT_BUF_MAX - 1] = &list_test1;
    /* to free neither on employ nor illegal */
    void *p3 = (void *)(wifi_reserve_ram +  CONFIG_UWP_PKT_BUF_MAX * CONFIG_UWP_PKT_BUF_SIZE);
    list_test2.buf = p3;

    LOG_DBG("free:%x %d", p2, uwp_pkt_buf_free(p2));
    void *p4 = uwp_pkt_buf_get();
    LOG_DBG("get buf :%x", p4);

    LOG_DBG( "free illegal:%d", uwp_pkt_buf_free(p3));
}
#endif
