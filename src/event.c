#include <gdfe/collections/hashmap.h>
#include <gdfe/collections/list.h>
#include <gdfe/event.h>

// represents a registered listener for an event
typedef struct RegisteredEvent {
    void*              listener;
    GDF_EventHandlerFP callback;
} RegisteredEvent;

// data stored for each event code
typedef struct EventCodeEntry {
    RegisteredEvent* registered_events_list;
} EventCodeEntry;

typedef struct EventSysState {
    GDF_HashMap(u32, event_code_entry) entries;
} EventSysState;

static GDF_BOOL      INITIALIZED = GDF_FALSE;
static EventSysState state;

u32 u32_hash(const u8* data, u32 len)
{
    u32 x = (u32)data[0] | ((u32)data[1] << 8) | ((u32)data[2] << 16) | ((u32)data[3] << 24);
    x     = ((x >> 16) ^ x) * 0x45d9f3b;
    x     = ((x >> 16) ^ x) * 0x45d9f3b;
    x     = (x >> 16) ^ x;
    return x;
}

GDF_BOOL gdfe_events_init()
{
    if (INITIALIZED)
        return GDF_FALSE;
    GDF_Memzero(&state, sizeof(state));
    state.entries = GDF_HashmapWithHasher(u32, EventCodeEntry, u32_hash, GDF_FALSE);
    INITIALIZED   = GDF_TRUE;
    return GDF_TRUE;
}

void gdfe_events_shutdown()
{
    for (HashmapEntry* entry = GDF_HashmapIter(state.entries); entry != NULL;
         GDF_HashmapIterAdvance(&entry))
    {
        EventCodeEntry* event_entry = entry->val;
        if (event_entry->registered_events_list != NULL)
        {
            GDF_ListDestroy(event_entry->registered_events_list);
        }
        event_entry->registered_events_list = NULL;
    }
}

GDF_BOOL GDF_EventRegister(u32 e_code, void* listener, GDF_EventHandlerFP callback)
{
    if (!INITIALIZED)
        return GDF_FALSE;

    EventCodeEntry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry)
    {
        EventCodeEntry empty = {};
        entry                = GDF_HashmapInsert(state.entries, &e_code, &empty, NULL);

        GDF_ASSERT(entry);

        entry->registered_events_list = GDF_ListCreate(RegisteredEvent);
    }

    // quick check for dupe listeners
    u64 registered_count = GDF_ListLen(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        if (entry->registered_events_list[0].listener == listener)
        {
            LOG_WARN("Tried to add duplicate event listener. Event code: %u", e_code);
            return GDF_FALSE;
        }
    }

    RegisteredEvent event;
    event.listener = listener;
    event.callback = callback;
    GDF_ListPush(entry->registered_events_list, event);
    return GDF_TRUE;
}

GDF_BOOL GDF_EventUnregister(u32 e_code, void* listener, GDF_EventHandlerFP callback)
{
    if (!INITIALIZED)
        return GDF_FALSE;

    EventCodeEntry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry)
    {
        LOG_WARN("Internal fail in event system.");
        return GDF_FALSE;
    }

    u64 registered_count = GDF_ListLen(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        RegisteredEvent e = entry->registered_events_list[i];
        if (e.listener == listener && e.callback == callback)
        {
            RegisteredEvent removed_event;
            GDF_ListRemove(entry->registered_events_list, i, &removed_event);
            return GDF_TRUE;
        }
    }

    LOG_WARN("Couldn't find event listener to unregister. Event code: %u", e_code);
    return GDF_FALSE;
}

GDF_BOOL GDF_EventFire(u32 e_code, void* sender, GDF_EventContext ctx)
{
    if (!INITIALIZED)
        return GDF_FALSE;

    EventCodeEntry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry)
    {
        // LOG_WARN("No entry for a fired event. Is this intended?");
        return GDF_TRUE;
    }

    u64 registered_count = GDF_ListLen(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        RegisteredEvent e = entry->registered_events_list[i];
        if (e.callback(e_code, sender, e.listener, ctx))
        {
            // just a way to make it so the event cancels if it
            // returns GDF_TRUE, useful for stuff potentially
            // but i dont like it so ill prob change it later
            return GDF_TRUE;
        }
    }
    return GDF_TRUE;
}
