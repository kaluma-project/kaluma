# Raspberry Pi Pico

## Flash

Flash partitions

```
|------------------------------------------------|
|      A     | B |       C       |       D       |
|--------------------|---|-----------|-----------|
|    960K    |64K|     1536K     |     1536K     |
|------------------------------------------------|
             |------------- flash.c -------------|

|------ 1MB -----|------------- 3MB -------------|
|---------------------- 4MB ---------------------|
```

- A : Binary (firmware)
- B : Storage (key-value database)
- C : User program (js)
- D : File system (lfs)
  (Total : 4MB)