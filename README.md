# ***• Anti-Debugging L0r***

![anti-debug](https://github.com/rootL0r/AntiDebugging/assets/157466888/a7d92410-a2ea-43d1-a26e-dccb60d11e90)

---

***• Some of Anti-Debugging techniques Used in Malware Development***

![image](https://github.com/rootL0r/AntiDebugging/assets/157466888/2469caf8-55f1-430d-a1f5-a32e8779d09b)

---

# Techniques Used in Basic

## 1. IsDebuggerPresent Function

The `IsDebuggerPresent` function is a Windows API call that checks if the current process is being debugged. It returns `TRUE` if a debugger is present and `FALSE` if no debugger is detected.

---

# Techniques Used in Advanced

1. **NtQueryInformationProcess**
   - Utilizes `NtQueryInformationProcess` to check for a debug port associated with the current process. Presence of a non-zero debug port indicates that the process is being debugged.

2. **IsDebuggerPresent**
   - Checks if a debugger is currently attached to the process using the `IsDebuggerPresent` Windows API function.

3. **CheckRemoteDebuggerPresent**
   - Determines if a debugger is attached to the current process or any associated process using the `CheckRemoteDebuggerPresent` Windows API function.

4. **Thread Handling**
   - **Block Debugger Threads**: Suspends all threads in the current process except the main thread to prevent debugger execution interference.
   - **Hide Thread from Debugger**: Uses `NtSetInformationThread` with `ThreadHideFromDebugger` flag to make the current thread invisible to debuggers.

---

Feel free to explore the code and adapt these techniques to make your malware more resilient against analysis.

---

**Credits**: This repository is maintained by [L0r](https://github.com/rootL0r) .
