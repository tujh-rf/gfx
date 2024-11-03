# Direct3D9 Simple Fullscreen

For graphical application is important to use full screen and not only small window, here is a basic tutorial how to switch to the predefined screen resolution.

> Direct3d9 was designed a long time ago and has a concept that graphical adapter can have only one display:
>```
>HMONITOR GetAdapterMonitor(
>  [in] UINT Adapter
>);
>```
> It leads some issues because fullscreen application will ===always=== use the main monitor, or the monitor where the window is now if it switches from the window to the fullscreen.

> Starting from Windows 8.1 it is possible that in the full screen mode the application will show only the black screen. For some of the operation system Microsoft created a bugfix, for some the (re)installation of the DirectX9 runtume will be needed: https://www.microsoft.com/en-ie/download/details.aspx?id=8109

---
