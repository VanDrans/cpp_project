/* restaurant.c —— 多线程餐厅模拟器 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

/* ================== 配置 ================== */
#define NUM_WAITERS        3
#define NUM_COOKS          2
#define NUM_STOVES         2
#define MENU_SIZE          5
#define QUEUE_CAPACITY     8
#define STOCK_INIT         10
#define ORDERS_PER_WAITER  5
#define SUPPLY_TIMES       3

#define TOTAL_STAFF  (NUM_WAITERS + NUM_COOKS + 3) /* +经理+供应商+老板 */