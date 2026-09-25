#ifndef ecu_handler_h_
#define ecu_handler_h_

#include <commands.h>
#include <panel_data.h>
#include <aps_protocol.h>
#include <logger.h>
#include <zb_handler.h>

#include <utils.h>

// Budget for one request / reply exchange, unrelated frames included.
#define ECU_REPLY_BUDGET_MS 3000

// Consecutive failed polls before an inverter is reported offline. Counted in
// polls, not time: with a large fleet one round can last longer than the
// poll interval.
#define ECU_OFFLINE_AFTER 30

// Watchdog: consecutive heartbeat failures before the zigbee module is
// reinitialised, then an exponential backoff between attempts.
#define ECU_WATCHDOG_FAILURES 3
#define ECU_WATCHDOG_BACKOFF_MIN_MS 60000UL
#define ECU_WATCHDOG_BACKOFF_MAX_MS 3600000UL

typedef enum
{
    ECU_POLL_OK,          // `reading` holds fresh data
    ECU_POLL_UNSUPPORTED, // the inverter answered, but there is no decoder for it
    ECU_POLL_FAILED,
    ECU_POLL_WENT_OFFLINE, // failed, and this is the poll that crossed ECU_OFFLINE_AFTER
} EcuPollResult;

void ecu_begin();
void ecu_reset();
void ecu_initialize();

void ecu_ping();
bool ecu_heart_beat();
// Heartbeat + watchdog. Returns false while the module is considered down.
bool ecu_check_alive();

void ecu_noop();

bool ecu_pair(Inverter *inverter);
// Queries the model code (0xDC). Sets INV_ID_TRIED whatever the outcome.
bool ecu_identify(Inverter *inverter);
EcuPollResult ecu_poll(Inverter *inverter, Reading *reading);

#endif
