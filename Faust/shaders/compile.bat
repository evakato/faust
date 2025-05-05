%VULKAN_SDK%/Bin/glslc.exe raygen.rgen -o raygen.rgen.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe closesthit.rchit -o closesthit.rchit.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe miss.rmiss -o miss.rmiss.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe gbuffer.vert -o gbuffer.vert.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe gbuffer.frag -o gbuffer.frag.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe filter.comp -o filter.comp.spv --target-env=vulkan1.3
%VULKAN_SDK%/Bin/glslc.exe atrous.comp -o atrous.comp.spv --target-env=vulkan1.3
pause