# Terminal Chat application

This is a chat app for users to chat with each other.

To run :

```
make #compiles the binaries for server and client

#in one terminal
./server

#in a separate terminal
./client # use as many as you would like in separate terminal instances
```

Future plans:

some sort of TUI and more features i guess and channel creation to some users


TOOD:
- Handle /quit much more gracefully and actually exit from the app.
- check channel creation logic and join logic
- ideally when we do create or join we want the name of the channel right after we write the command but currently we take a new input so fix that
