/*
```
https://docs.rs/tokio/latest/tokio/sync/struct.RwLock.html
The priority policy of Tokio’s read-write lock is fair (or write-preferring), in order to ensure that readers cannot starve writers. Fairness is ensured using a first-in, first-out queue for the tasks awaiting the lock; if a task that wishes to acquire the write lock is at the head of the queue, read locks will not be given out until the write lock has been released. This is in contrast to the Rust standard library’s std::sync::RwLock, where the priority policy is dependent on the operating system’s implementation.
```
*/
