# RAIL-DRIVER-
 Model railroad board and firmware based on PIC18F4585 

For Board Layout

Important points to remember
---------------------------


Step one: 
----------------

- Generate schematic and validate circuit with ERC
- Pay particular attention to net class, especially for the power section

Second step : 
----------------

- Component placement and automatic routing. 
- To fill in the top and bottom layers, you need to draw a polygon all around the board, selecting the right layer (Top or Bottom) in the top left-hand corner after routing (otherwise it won't work), then restart routing.
- Validate with a DRC, selecting only the Top, Bottom, Via and Pad layers.
- Finalization of the silkscreen (layer 21) by placing all indications correctly and then generating the legend for the drills on layer 144 using drillegend-stack.ulp

Step three: 
-----------------

- Generate the files for manufacturing with the CAM Processor by clicking on the CAM_JOB/RailDriverCamJob.cam file.
- In the top-left File menu, select the .brd file for the corresponding design. 
- The directories and file names must be correct in the various tabs (check and correct if necessary). 
- All that's left to do is to run PROCESS JOB to produce the files.

Fourth step: 
-----------------

- To modify the selection of unknown casings, modify the 3D_RENDERING/eagle3d/ulp/3dusrpac.dat file.  
- Update the configuration files directory in 3dconf.dat 
- Generate a 3D rendering of the design by launching the ULP icon in the layout screen 3D_RENDERING/eagle3d/ulp/3d50.ulp 
- Specify the .brd file, which will generate a .pov file that can be processed by povray to obtain the 3D rendering. 
- Select "User-defined model" in the General tab 
- Add layer 25 in Miscellaneous Case Design
- Modify Writing on plate to display layers 21 and 25 only. 
- Add layers 25 and 27 in Case Reference 
- Click on Create POVRay file and Exit. 
- Copy the generated EAGLE_FILES/RailDriver/RailDriver.pov file into 3D_RENDERING/eagle3d/povray
- Repeat the same operation, moving the camera to shoot from below (using Y) and/or from the other sides (using X and Z).
- Click on the 3D_RENDERING/eagle3d/povray/RailDriver.pov file to start building the 3D view with povray. Use 800 x 600 AA 0.3 for a reasonable image creation time, or 1600 x 1200 A 0.3 for a better resolution, but the image creation time will take several tens of minutes.
- In eagle, make a screen copy of the different layers built during the construction of the manufacturing files. Place all views in PNGVIEW.

HOW TO ADD A 3D MODEL : 
------------------------------

- Retrieve the model in stl format from the Internet (e.g. snapeda). If the model is in step format, it must first be converted to stp, for example on https://polyd.com/fr/convertir-step-en-stl-en-ligne
- Use the stl2pov.exe utility available in utilities (eagle installation directory) (launch a Windows shell) stl2pov.exe myModel.stl > myModel.inc. Copy it to 3D_RENDERING/eagle3d/povray. 

For example: stl2pov.exe "TO220HeatSink\TO220_Heatsink.stp" > "TO220HeatSink\TO220HeatSink.inc".
G
- Modify the myModel.inc file so that it is understood by povray, taking as an example: fk_222_sa.inc
- Add the following include to the 3D_RENDERING/eagle3d/povray/e3d_user.inc file: #include "myModel.inc". 
- Add the following information to the 3D_RENDERING/eagle3d/ulp/3dusrpac.dat file:

Par exemple : FK222:0:1:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:HEATSINK_FK_222(:HEATSINK_FK_222_GRND


Index meaning

[00] Eagle component package name
[01] Output name
[02] Output value
[03] Define color bands
[04] SMD offset (parts will be moved pcb_cuhight up/down)
[05] LED options (The LED options dialog will be displayed)
[06] Ready for sockets (see explanation)
[07] Quartz height request
[08] Has part a macro (for example, SMD jumpers)
[09] SMD resistor, Generate number combination
[10] Socket macro
[11] Height of socket in 1/10mm
[12] Comments concerning socket
[13] Internal for administration purpose (not used at present)
[14] Y axis rotation correction
[15] Correction offset x
[16] Offset correction y
[17] Correction offset z
[18] Use Prefix from Part?
[19] Shunt on pinheader (a dialog will be displayed)
[20] Logo selection dialog will be shown
[21] Reserved[8]
[29] Bounding-Box Minimum
[30] Bounding-Box Maximum
[31] POV-Ray macro (Name of pov macro and a left parenthesis)
[32] Package comments (German)
[33] Package comments (English)

Fifth stage: 
-----------------

- Update the BOM to purchase components


Sixth step:
---------------

-Compress CAM_JOB and PNGVIEW directory to start manufacturing at AllPcb: https://www.allpcb.com/ 

For Firmware : Please read documentation 
