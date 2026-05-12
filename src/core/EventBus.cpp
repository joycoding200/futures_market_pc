#include "EventBus.h"

EventBus& EventBus::instance() {
    static EventBus bus;
    return bus;
}
