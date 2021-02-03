PORTING
=======

```
targets/
  └─ <new-board>/
    ├─ include/
    ├─ src/
    ├─ Make.def
    └─ board.js
```

* Create a folder in `targets/<board>/include` and `targets/<board>/src`.
* Create C files in `targets/<board>/src` implementing all headers in `include/port`.
* Create `target.cmake`. for the CMAKE
* (Optional) Create `board.js`.
