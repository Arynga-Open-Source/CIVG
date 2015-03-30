/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * Common dispatcher interface.
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Opaque structure that identify specific instance of dispatcher.
 */
typedef struct dispatcher_Dispatcher dispatcher_Dispatcher;

/**
 * @brief Dispatched event handler function.
 * @param[in] eventId Id of dispatched event.
 */
typedef void (*DISPATCHER_EVENT_HANDLER)(int eventId);
/**
 * @brief Sets function that will handle dispatched events.
 * @param dispatcher Pointer to dispather instance.
 * @param handler Pionter to handler function.
 */
extern void dispatcher_setEventHandler(dispatcher_Dispatcher* dispatcher, DISPATCHER_EVENT_HANDLER handler);

/**
 * @brief Posts event to be dispatched.
 * @param dispatcher Pointer to dispather instance.
 * @param eventId Id of event to be dispatched.
 */
extern void dispatcher_postEvent(dispatcher_Dispatcher* dispatcher, int eventId);

# ifdef __cplusplus
}
# endif

#endif
