PORTING
=======

```
targets/
  └─ <new-target>/
    ├─ include/
    │  └─ ...
    ├─ src/
    │  └─ ...
    ├─ boards/
    │  └─ <new-board>/
    │     └─ board.h
    ├─ target.cmake
    └─ board.js
```

* Create a folder for a new target in `targets/<target>`.
* Create a folder for a new board in `targets/<target>/boards/<board>`.
* Create C files in `targets/<target>/src` implementing all headers in `include/port`.
* Create `board.h` file in `targets/<target>/boards/<board>`
* Create `target.cmake`. for the CMAKE
* Create `board.js`.
