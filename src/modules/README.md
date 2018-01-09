Defingin a Builtin Module
=========================

1. Create a module folder: `src/modules/<module_name>`
2. Create a `module.json` file in the module folder like below.

   ```js
   {
     "js": true, // has javascript impl.
     "native": true // has native C impl.
   }
   ```
   If `.js=true`, there should be `<module_name>.js` file. This js file will be complied by host Jerryscript and then the snapshot will be generated in `src/gen/kameleon_modules.c`
   
   
   If `.native=true`, there should be `module_<module_name>.h` and `module_<module_name>.c`. When user try to load this module by `require()`, Firstly js module will be loaded if `.js=true`. If `.js=false` and `.native=true`, the native module will be loaded.
3. Create a `<module_name>_magic_strings.h` file for magic string definitions.

