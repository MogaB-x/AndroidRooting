# AndroidRooting

This project demonstrates the process of obtaining root access on an Android device by injecting custom binaries into the system and integrating them into the Android boot sequence. The implementation is based on the Android Rooting Lab documentation from [SEED Labs](https://seedsecuritylabs.org/Labs_20.04/Files/Android_Rooting/Android_Rooting.pdf).

Key steps of the project include:  
- Replacing the original `app_process64` binary with a custom daemon launcher (`app_daemon`).  
- Adding and configuring custom binaries (`mysu` and `mydaemon`) in the `/system/bin` directory.  
- Adjusting permissions to ensure proper execution during system boot.  
- Testing functionality in a virtual Android environment.  

This project aims to explore Android system internals, gain hands-on experience with system-level privilege escalation, and learn the mechanisms behind Android rooting.
