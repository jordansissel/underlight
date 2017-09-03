#ifndef _ITERATOR_H_
#define _ITERATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "mgos_timers.h"

typedef void (*callable)(void *param);
typedef bool (*predicate)(void *param);
typedef void (*callable_with_index)(void *param, int i);

typedef uintptr_t mgos_iterator_id;
typedef uintptr_t mgos_iterator_count_id;

mgos_iterator_count_id mgos_iterator_count(int msecs, int limit, callable_with_index cb, void *param);
mgos_iterator_id mgos_iterator(int msecs, predicate has_next, timer_callback cb, void *arg);
void mgos_clear_iterator(mgos_iterator_id iterator_id);

#endif /* _ITERATOR_H_ */