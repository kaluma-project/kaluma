PORTING
=======

```
targets/boards/
  └─ <new-board>/
    ├─ include/
    ├─ src/
    ├─ Make.def
    └─ board.js
```

* Create a folder in `targets/boards/<board>/include` and `targets/boards/<board>/src`.
* Create C files in `targets/boards/<board>/src` implementing all headers in `include/port`.
* Create `Make.def`.
* (Optional) Create `board.js`.
