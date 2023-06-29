
#ifndef __APP_DRV_CAN_H__
#define __APP_DRV_CAN_H__

#include <stdint.h>

typedef void (*app_can_event_cb_t)(uint32_t id, uint8_t *buf, uint8_t len);

void app_can_init(uint32_t baud, uint32_t baudfd, uint8_t is_fd);
int app_can_send(uint8_t *buf, uint32_t id, uint8_t len);
void app_can_add_event_cb(app_can_event_cb_t event_cb, uint8_t event_code, void* user_data);


#endif

