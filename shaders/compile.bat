@echo off
set GLSLC=C:\VulkanSDK\1.3.290.0\Bin\glslc.exe

for %%f in (*.vert) do (
    %GLSLC% %%f -o %%f.spv
)

for %%f in (*.frag) do (
    %GLSLC% %%f -o %%f.spv
)

pause
