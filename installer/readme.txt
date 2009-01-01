Welcome to openDarkEngine 0.2.9 (Segmentation Fault). This checkpoint release will allow you to view any TDP/TG, TMA or SS2 levels in 'no clipping' mode.

A few things to keep in mind:

1. To be able to play the levels, you will need to have all the game data files (the .crf files and .mis files) already copied to the hard drive. Currently OPDE does not access the CD drive for files.
2. Use the created shortcuts to run the program. Don't run opde.exe directly.
3. To improve the performance adjust the value of MaxAtlasSize in opde.cfg.
4. Use WASD to navigate, the mouse to look around, F5 to take screenshot, and Esc to quit. For additional fun you can try P, O and I.

And finally, Microsoft has the habit of updating DirectX without changing the version number. The originally released DirectX 9.0c is very different from the DirectX 9.0c that is available for download on Microsoft's site today. If you encounter any problems with running openDarkEngine download the latest DirectX installer and run it to update your DirectX. If the problem persists please follow these steps:

1. Press "Start", choose "Run", type "dxdiag" and press "ok". On the window that opens click on "Save All Information..." and save it to "DxDiag.txt".
2. Open a command prompt and navigate to the directory that you have installed openDarkEngine, then enter the following command:
tree/f >tree.txt
This should create a file called "tree.txt" in the same folder.
3. In the directory that you installed openDarkEngine zip up all the .cfg files and .log files, the two files "DxDiag.txt" and "tree.txt" that were created above, and the generated shortcuts ("opde (Thief 1)", "opde (Thief 2)" and "opde (Shock 2)") and send the zip file to opendarkengine@gmail.com .

Thanks for trying openDarkEngine!

The openDarkEngine team