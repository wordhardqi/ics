#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  long value;
  char expr[64];
  bool calculated;

} WP;
void wp_printf(WP* wp);
WP* wp_head();
bool wp_eval(WP* );
void set_watchpoint(WP* wp, char * );
WP* new_wp();
void free_wp(WP *wp);
#endif
