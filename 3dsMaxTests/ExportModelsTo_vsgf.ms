/*
Last change: 16/07/2019
Author: Ray Tracing Systems.
Exporting 3lds Max scenes in "vsgf" the format of the Hydra renderer.
*/

actionMan.executeAction 0 "40472"  -- MAXScript Listener
clearListener()

fn CopyDir sourceDir outDir =
(
	for f in getFiles (sourceDir + "*.*") do
		copyFile f (outDir + getFilenameFile f + getFilenameType f)

	for d in getDirectories (sourceDir + "*") do
	(
		dataDir = outDir + "data\\"
		makedir dataDir
		
		for f in getFiles (d + "*.*") do
			copyFile f (dataDir  + getFilenameFile f + getFilenameType f)
	)
)


(--start ms			
	hydraDataPath	= "c:\\[Hydra]\\pluginFiles\\scenelib\\"	
	--rootPath 			= "d:\\Works\\Ray-Tracing_Systems\\HydraDevelop\\HydraAPI-tests\\3dsMaxTests\\"		
	rootPath 			= "d:\\RayTracingSystems\\HydraDevelop\\HydraAPI-tests\\3dsMaxTests\\"
	refPath 				= rootPath + "Reference\\"
	
	numExportScenes = 0
	numAllScenes = 0	
			
	files = getFiles (rootPath + "*.max")
		
	for f in files do
	(
		if (loadMAXFile f useFileUnits:true) do numExportScenes += 1
		
		fileName = getFilenameFile f			
		
		render vfb:true outputFile:(refPath + fileName + ".png")
		
		outPath = rootPath + fileName + "\\"		
		makedir outPath 
		
		CopyDir hydraDataPath outPath 
		
		format "Export model: % \n" fileName
		
		numAllScenes += 1
	)
	
	format "Export % models from % \n" numExportScenes numAllScenes
)-- end ms