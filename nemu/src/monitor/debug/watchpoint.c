#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

WP* wp_head(){
   return head;
}

void reset_wp(WP* wp){
    wp->calculated = false;
    wp->value = 0;

}
bool wp_eval(WP *wp){
    bool success = false;
    if(!wp->calculated){
        wp->value = expr(wp->expr, &success);
        if(success){
            wp->calculated = true;
            return true;
        }else {
            Log("Fail to eval watchpoint with expr %s", wp->expr);
            return false;
        }
    }else{
        long new_val = expr(wp->expr, &success);
        if(success){
            if(new_val != wp->value){
                wp->value = new_val;
                return true;
            }else{
                return false;
            }
        } else{
            Log("Fail to eval watchpoint with expr %s", wp->expr);
            return false;
        }
    }
    //shall not be reached;
    assert(0);
    return false;

}
void set_watchpoint(WP* wp, char * expression){
    reset_wp(wp);
    strcpy(wp->expr,expression);
}
void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        reset_wp(&wp_pool[i]);
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

WP *new_wp() {
    if (free_ == NULL) {
        Log("Cannot get free watchpoint");
        assert(0);
        return NULL;
    } else {
        WP *tmp = free_;
        free_ = free_->next;
        tmp->next = head;
        head = tmp;
    }
}

void free_wp(WP *wp) {
    if (head == NULL) {
        Log("head list is empty");
        return;
    }
    WP *p = head;
    if (head->NO == wp->NO) {
        Log("the node to be free is head");
        head = p->next;
        p->next = free_;
        free_ = p;
        reset_wp(free_);
    } else {
        WP *prev = p;
        p = p->next;
        while (p != NULL && p->NO != wp->NO) {
            prev = p;
            p = p->next;
        }
        if (p == NULL) {
            Log("fail to find node %0x", wp);
            return;
        } else {
            prev->next = p->next;
            p->next = free_;
            free_ = p;
            reset_wp(free_);
        }

    }

}
void wp_printf(WP* wp){
    printf("NO = %-4d, expr: %-32s , value = %-8ld, (calculated = %5s )\n",
           wp->NO,wp->expr,wp->value, wp->calculated? "ture":"false");
}

/* TODO: Implement the functionality of watchpoint */


