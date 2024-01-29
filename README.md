# rmake
This is a bare metal oriented build utility designed to work with VScode.

rmake is designed to be used with vscode and the “Task Runner” plugin.
Copy the ".vscode" folder, so you can compile your project with Task Runner.

Make symlinks in /usr/bin/rmake for linux and C:\Windows\System32 for windows.

## warning
- extensions (.c and .cpp) must be in lowercase
- it does not manage names with spaces (folders or files)
- the crt0.c file must be "unique" so you cannot have several crt0.c files, rmake will position it first during compilation