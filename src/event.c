#include <event.h>
#include <collections/hashmap.h>
#include <collections/list.h>

// represents a registered listener for an event
typedef struct registered_event {
    void* listener;
    GDF_EventHandlerFP callback;
} registered_event;

// data stored for each event code
typedef struct event_code_entry {
    registered_event* registered_events_list;
} event_code_entry;

typedef struct event_state {
    GDF_HashMap(u32, event_code_entry) entries;
} event_state;

static bool INITIALIZED = false;
static event_state state;

u32 u32_hash(const u8* data, u32 len) {
    u32 x = (u32)data[0] | 
    ((u32)data[1] << 8) | 
    ((u32)data[2] << 16) | 
    ((u32)data[3] << 24);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

bool GDF_InitEvents()
{
    if (INITIALIZED)
        return false;
    GDF_MemZero(&state, sizeof(state));
    state.entries = GDF_HashmapWithHasher(
        u32,
        event_code_entry,
        u32_hash,
        false
    );
    INITIALIZED = true;
    return true;
}

void GDF_ShutdownEvents()
{
    for (
        HashmapEntry* entry = GDF_HashmapIter(state.entries); 
        entry != NULL; 
        GDF_HashmapIterAdvance(&entry)
    ) {
        event_code_entry* event_entry = entry->val; 
        if (event_entry->registered_events_list != NULL)
        {
            GDF_LIST_Destroy(event_entry->registered_events_list);
        }
        event_entry->registered_events_list = NULL;
    }
}

bool GDF_EventRegister(u32 e_code, void* listener, GDF_EventHandlerFP callback)
{
    if (!INITIALIZED)
        return false;

    event_code_entry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry) 
    {
        event_code_entry empty = {};
        entry = GDF_HashmapInsert(state.entries, &e_code, &empty, NULL);

        GDF_ASSERT(entry);

        entry->registered_events_list = GDF_LIST_Create(registered_event);
    }

    // quick check for dupe listeners
    u64 registered_count = GDF_LIST_GetLength(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        if (entry->registered_events_list[0].listener == listener)
        {
            LOG_WARN("Tried to add duplicate event listener. Event code: %u", e_code);
            return false;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = callback;
    GDF_LIST_Push(entry->registered_events_list, event);
    return true;
}

bool GDF_EventUnregister(u32 e_code, void* listener, GDF_EventHandlerFP callback)
{
    if (!INITIALIZED)
        return false;
    
    event_code_entry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry) 
    {
        LOG_WARN("Internal fail in event system.");
        return false;
    }

    u64 registered_count = GDF_LIST_GetLength(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        registered_event e = entry->registered_events_list[i];
        if (e.listener == listener && e.callback == callback)
        {
            registered_event removed_event;
            GDF_LIST_Remove(entry->registered_events_list, i, &removed_event);
            return true;
        }
    }

    LOG_WARN("Couldn't find event listener to unregister. Event code: %u", e_code);
    return false;
}

bool GDF_EventFire(u32 e_code, void* sender, GDF_EventContext ctx)
{
    if (!INITIALIZED)
        return false;

    event_code_entry* entry = GDF_HashmapGet(state.entries, &e_code);

    if (!entry) 
    {
        // LOG_WARN("No entry for a fired event. Is this intended?");
        return true;
    }

    u64 registered_count = GDF_LIST_GetLength(entry->registered_events_list);
    for (u64 i = 0; i < registered_count; i++)
    {
        registered_event e = entry->registered_events_list[i];
        if (e.callback(e_code, sender, e.listener, ctx))
        {
            // just a way to make it so the event cancels if it
            // returns true, useful for stuff potentially
            // but i dont like it so ill prob change it later
            return true;
        }
    }
    return true;
}