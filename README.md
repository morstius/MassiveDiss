"# MassiveDiss" 

#instructions
To build the solution you might need to retarget to a Windows SDK version 10 subversion that you have installed on your machine

I am trying to load a '.obj' file from the 'MassiveDiss/models' folder. If the object file is referencing a '.mtl' file then I am attempting to load it
from the same location. Material files can include sub folder information, but the base folder is the 'MassiveDiss/models' folder.

All the textures are expected to be in 'MassiveDiss/textures'. The hard coded textures will be in there and the other texture paths will be read from the material file.
With 'MassiveDiss/textures' as the base folder.

The loading of the object file happens in line 622 in MassiveDiss.cpp. Where I use the prefix 'models/'. The base model is found at 'http://casual-effects.com/data/' under 'San Miguel 2.0'.
I created a bigger set by putting 4 models in a 2-by-2 fashion. It is using the same textures as the model from casual-effects but requires the object and material files from 
'https://leeds365-my.sharepoint.com/:f:/g/personal/sc17vok_leeds_ac_uk/EsbSg3Nw9JhAgbj_VREiDXgBijp6Zaa4Cd1gRDTSaHY_7Q?e=Jk4uRs'


#controls
t - toggles the frustum culling on and off
c - toggles the occlusion culling on and off
m - toggles drawing methods, whether the triangles are filled or just outlined (this is just to see more of the scene and see how the objects are structured)

w a s d - to move around
q e - to change elevation
mouse - to look around