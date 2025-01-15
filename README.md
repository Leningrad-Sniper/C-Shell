# C Shell

Custom Shell Implementation
This is a custom shell implementation with features similar to bash, including built-in commands, background process handling, I/O redirection, and signal handling.
Features
Basic System Commands

Shell Prompt

Displays username, system name, and current directory path
Uses '~' to represent shell's home directory
Shows absolute paths when outside home directory


Input Processing

Supports ';' separated multiple commands
Handles '&' for background process execution
Accounts for arbitrary spaces and tabs
Shows appropriate error messages for invalid commands


Directory Navigation (hop)

Changes current working directory
Supports absolute and relative paths
Implements '.', '..', '~', and '-' flags
Shows full path after directory change
Handles multiple arguments sequentially


Directory Listing (reveal)

Lists files and directories in lexicographic order
Supports -a (show hidden) and -l (detailed info) flags
Color coding: green for executables, white for files, blue for directories
Compatible with '.', '..', '~', and '-' symbols
Handles various flag combinations (e.g., -la, -al)


Command History (log)

Stores up to 15 commands
Preserves commands across sessions
Supports command execution by index
Includes log purge to clear history
Doesn't store duplicate consecutive commands



Process Management

Process Information (proclore)

Shows process details including:

PID
Process Status (R/R+/S/S+/Z)
Process Group
Virtual Memory
Executable Path




File Search (seek)

Searches for files/directories in specified locations
Supports -d (directories only), -f (files only), and -e (execute/enter) flags
Shows relative paths from target directory
Color-coded output for files and directories


Shell Configuration (.myshrc)

Supports custom aliases
Allows single-word command mappings



Advanced Features

I/O Redirection

Implements '>', '>>', and '<' operators
Creates output files if they don't exist
Handles file permissions appropriately
Shows proper error messages for missing input files


Pipe Implementation

Supports multiple piped commands
Handles pipe errors gracefully
Works in conjunction with I/O redirection


Process Management

activities: Lists all running shell-spawned processes
Signal Handling:

ping: Sends signals to processes
Ctrl-C: Interrupts foreground processes
Ctrl-D: Logs out of shell
Ctrl-Z: Stops foreground processes


Process Control:

fg: Brings background processes to foreground
bg: Resumes stopped background processes




Bonus Feature: Neonate

Displays most recent PID
Updates at specified intervals
Terminates on 'x' key press



Usage Notes

Background processes show PID when started
Process completion messages show automatically
Error handling implemented for invalid commands and paths
Color coding helps distinguish between different file types
All paths support both absolute and relative formats

Implementation Details

Erroneous commands are stored in command history
Process states indicated with '+' for foreground processes
File permissions set to 0644 for created files
Input validation implemented for all commands
Comprehensive error messages provided for invalid operations

Limitations

Multiple I/O redirections not supported
Background process support limited to system commands
Alias support limited to single-word mappings
No multi-word alias support
Path names assumed to be without whitespace characters
