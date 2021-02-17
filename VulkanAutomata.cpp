#define VK_USE_PLATFORM_XLIB_KHR
#define VK_API_VERSION_1_2 VK_MAKE_VERSION(1, 2, 0)
#include <cstring>
#include <ctime>
#include <cmath>
#include <time.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>

bool 	valid 		=	1;	//	Break on error
bool 	output 		=	1;	//	?
int 	loglevel 	=	1;	//	?

//	Messaging systems designed for terminal width of 96 characters

void vr(const std::string& id, std::vector<VkResult>* reslist, VkResult res) {
//	VkResult output message
	reslist->push_back(res);
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
	if(output && loglevel <= 0) {
		std::cout	
			<< "  " << idx_string 		<< ":\t"
			<< (res==0?" ":res_string) 	<< " \t"
			<< id 						<< "\n"; } }

void ov(const std::string& id, auto v) {
//	Single info output message
	int 		padlen	= 4;
	int 		pads	= 10;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output && loglevel <= 0) {
	std::cout 
		<< "\tinfo:\t    "
		<< id 	<< pad 
		<< " [" << v	<< "]\n"; } }

void iv(const std::string& id, auto ov, int idx) {
//	Multiple info output message
	int 		padlen 	= 4;
	int 		pads 	= 9;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output && loglevel <= 0) {
	std::cout 
		<< "\tinfo:\t    "
		<< idx 	<< "\t"
		<< id 	<< pad 
		<< " [" << ov	<< "]\n"; } }

void rv(const std::string& id) {
//	Return void output message
	if(output && loglevel <= 0) {
	std::cout 
		<< "  void: \t" 
		<< id	<< "\n"; } }

void hd(const std::string& id, const std::string& msg) {
//	Header output message
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "____"; }
	if(output && loglevel <= 0) {
	std::cout 
		<< bar 	<< "\n "
		<< id 	<< "\t"
		<< msg 	<< "\n"; } }

void nf(auto *Vk_obj) {
//	NullFlags shorthand
	Vk_obj->pNext = NULL;
	Vk_obj->flags = 0; }

unsigned char ToByte(bool b[8]) {
//	For image P4 output
    unsigned char c = 0;
    for (int i=0; i < 8; i++) { if (b[i]) { c = c | 1 << i; } } return c; }

// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`
int32_t findProperties(
	const 		VkPhysicalDeviceMemoryProperties* 	pMemoryProperties,
				uint32_t 							memoryTypeBitsRequirement,
				VkMemoryPropertyFlags 				requiredProperties 			) {
    const 		uint32_t 	memoryCount = pMemoryProperties->memoryTypeCount;
    for ( uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex ) {
        const 	uint32_t 				memoryTypeBits 			= (1 << memoryIndex);
        const 	bool 					isRequiredMemoryType 	= memoryTypeBitsRequirement & memoryTypeBits;
        const 	VkMemoryPropertyFlags 	properties 				= pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
        const 	bool 					hasRequiredProperties 	= (properties & requiredProperties) == requiredProperties;
        if (isRequiredMemoryType && hasRequiredProperties) { return static_cast<int32_t>(memoryIndex); } } return -1; }

static VKAPI_ATTR VkBool32 VKAPI_CALL
//	Vulkan validation layer message output
	debugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT	messageSeverity, 
				VkDebugUtilsMessageTypeFlagsEXT			messageType, 
		const 	VkDebugUtilsMessengerCallbackDataEXT* 	pCallbackData, 
		void* 											pUserData		) {
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "####"; }

	std::string msg 	= pCallbackData->pMessage;
	std::string msg_fmt = "";
	for(int i = 0; i < msg.size()-1; i++) {
		char chr = msg[i];
		char chrhtml = msg[i+1];
		msg_fmt = 
			(chr == ':' && chrhtml != '/' 
			? msg_fmt+":\n\n" 
			: 
				(chr == '|' 
				? msg_fmt+"\n" 
				: msg_fmt+chr
				)
			);
	}
	msg_fmt = msg_fmt + msg[msg.size()-1];
	if(output) {
		std::cout 
			<< "\n\n"
			<< bar 		<< "\n "
			<< msg_fmt
			<< "\n" 	<< bar
			<< "\n\n";
	}
	valid = 0;
	return VK_FALSE; 
}

struct ShaderCodeInfo {
	std::string 		shaderFilename;
	std::vector<char>	shaderData;
	size_t 				shaderBytes;
	bool 				shaderBytesValid;
};

ShaderCodeInfo getShaderCodeInfo(const std::string& filename) {
	std::ifstream 		file		(filename, std::ios::ate | std::ios::binary);
	size_t 				fileSize = 	(size_t) file.tellg();
	std::vector<char> 	buffer		(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	ShaderCodeInfo rfsc_info[1];
		rfsc_info[0].shaderFilename		= filename;
		rfsc_info[0].shaderData			= buffer;
		rfsc_info[0].shaderBytes		= buffer.size();
		rfsc_info[0].shaderBytesValid	= (rfsc_info[0].shaderBytes%4==0?1:0);
	return rfsc_info[0];
}

//	COMMENTMARKER_0
//	Number of 'v' floats to pass to frag shader
const int VMX = 32;

struct init_ub {
//	struct for uniform buffer passed to frag shader
    float w;		//	Application Width
	float h;		//	Application Height
	float seed;		//	'seed' value
	float frame;	//	Total GPU frames generated
	float clicks;	//	Sum of mousewheel up and mousewheel down
	float mx;		//	Mouse X position
	float my;		//	Mouse Y position
	float mlb;		//	Mouse Left Clicked
	float mrb;		//	Mouse Right Clicked
	float div;		//	Divider Panels
//	'v' parameters for shenanigans
	float v0; 	float v1; 	float v2; 	float v3;
	float v4; 	float v5; 	float v6; 	float v7;
	float v8; 	float v9; 	float v10; 	float v11;
	float v12; 	float v13; 	float v14; 	float v15;
	float v16; 	float v17; 	float v18;	float v19;
	float v20; 	float v21; 	float v22; 	float v23;
	float v24; 	float v25; 	float v26; 	float v27;
	float v28; 	float v29; 	float v30; 	float v31;
};

int main(void) {

//	Timestamp for file handling
	std::string timestamp = std::to_string(time(0));

	if(loglevel != 0) { loglevel = loglevel * -1; }

//	Make local backup: Fragment Shader (automata)
	std::string fbk_auto 	= "res/frag/frag_automata0000.frag";
	std::string cp_auto		= "cp '" + fbk_auto + "' 'fbk/auto_" + timestamp +".frag'";
	system(cp_auto.c_str());

//	Make local backup: Fragment Shader (seed)
	std::string fbk_seed 	= "res/frag/frag_init.frag";
	std::string cp_seed		= "cp '" + fbk_seed + "' 'fbk/seed_" + timestamp +".frag'";
	system(cp_seed.c_str());

//	Make local backup: Render Engine
	std::string fbk_engi 	= "VulkanAutomata.cpp";
	std::string cp_engi		= "cp '" + fbk_engi + "' 'fbk/engi_" + timestamp +".cpp.bk'";
	system(cp_engi.c_str());

//	Make blank file: Plaintext 'v' Coordinates
	std::string mkevo		= "echo > 'fbk/" + timestamp + ".evo'";
	system(mkevo.c_str());

//	If the last automata fragment shader matches the current one,
//		Use the last 'v' coordinates float file for the current timestamp
	std::string frg_last 	= "fbk/LastFrg.chk";
	std::string evo_last 	= "fbk/LastEvo.chk";
	std::string cmp_evo		= 	"if cmp --silent -- '" + fbk_auto + "' '" + frg_last + "';"
							+	" then cp '" + evo_last + "' 'fbk/evo" + timestamp + ".float';"
							+	" fi";
	system(cmp_evo.c_str());

//	Record the current automata fragment as the last used config
	std::string cpfrglst	= "cp '" + fbk_auto + "' '" + frg_last + "'";
	system(cpfrglst.c_str());

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CONFIG");		/**/
	///////////////////////////////////////

	const uint32_t 	APP_W 			= 512;		//	1536	768		512		384
	const uint32_t 	APP_H 			= 320;		//	832		448		320		256
	const long 		FPS 			= 300;		//	2+
	const int 		TEST_CYCLES 	= 0;		//	0+
	const int 		SCR_RECORD 		= 0;		//	0+
	const int 		SCR_RAMP_ACCEL 	= 0;		//	1+
	const int 		SCR_PAD 		= 0;		//	1+
	const int 		SCR_PAD_MULT 	= 0;		//	?
	const char*		PPM_ENC			= "P5";		// 	P4 	P5	P6
	const int 		PPM_P5C			= 0;		// 	0 	1	2
		  float 	div 			= 1.0;		//	1.0, 2.0, ...

	uint32_t 		PD_IDX 			= UINT32_MAX;	//	Physical Device Index
	uint32_t 		GQF_IDX 		= UINT32_MAX;	//	Graphics Queue Family Index
	uint32_t		SURF_FMT 		= UINT32_MAX;	//	Surface Format
	uint32_t		UB_MEMTYPE 		= UINT32_MAX;	//	?? 	Uniform Buffer Memorytype 	?? 
	uint32_t		SCR_MEMTYPE 	= UINT32_MAX;	//	?? 	Screenshot Memorytype 		??

	const uint32_t	SHDRSTGS 		= 2;											//	Shader Stages
	const long 		NS_DELAY 		= 1000000000 / FPS;								//	Nanosecond Delay
	const float 	TRIQUAD_SCALE 	= 1.0;											//	Vertex Shader Triangle Scale
	const float 	VP_SCALE 		= TRIQUAD_SCALE + (1.0-TRIQUAD_SCALE) * 0.5;	//	Vertex Shader Viewport Scale

	const uint32_t 	VERT_FLS 		= 1;	//	Number of Vertex Shader Files
	const uint32_t 	FRAG_FLS 		= 2;	//	Number of Fragment Shader Files
	const uint32_t 	INST_EXS 		= 3;	//	Number of Vulkan Instance Extensions
	const uint32_t 	LDEV_EXS 		= 1;	//	Number of Vulkan Logical Device Extensions
	const uint32_t 	VLID_LRS 		= 1;	//	Number of Vulkan Validation Layers

//	Paths to shader files and extension names
	const char* 	filepath_vert		[VERT_FLS] =
		{	"./app/vert_TriQuad.spv" 					};
	const char* 	filepath_frag		[FRAG_FLS] =
		{	"./app/frag_init.spv",
			"./app/frag_automata0000.spv" 				};
	const char* 	instance_extensions	[INST_EXS] =
		{	"VK_KHR_surface", 
			"VK_KHR_xlib_surface", 
			"VK_EXT_debug_utils"						};
	const char* 	validation_layers	[VLID_LRS] =
		{	"VK_LAYER_KHRONOS_validation" 				};
	const char* 	device_extensions	[LDEV_EXS] =
		{	"VK_KHR_swapchain"							};

//	Config Notification Messages
	ov("Window Width", 			APP_W		);
	ov("Window Height", 		APP_H		);
	ov("Render Cycles", 		TEST_CYCLES	);
	ov("FPS Target", 			FPS			);
	ov("Vertex Shaders", 		VERT_FLS	);
	ov("Fragment Shaders", 		FRAG_FLS	);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "INIT");		/**/
	///////////////////////////////////////

//	VkResult storage
	std::vector<VkResult> vkres;
	vr("init", &vkres, VK_ERROR_UNKNOWN);

	VkApplicationInfo vka_info[1];
		vka_info[0].sType	= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vka_info[0].pNext	= NULL;
		vka_info[0].pApplicationName	= "VulkanAutomata";
		vka_info[0].applicationVersion	= 0;
		vka_info[0].pEngineName			= NULL;
		vka_info[0].engineVersion		= 0;
		vka_info[0].apiVersion			= VK_API_VERSION_1_2;

	VkInstanceCreateInfo vki_info[1];
		vki_info[0].sType 	= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	nf(&vki_info[0]);
		vki_info[0].pApplicationInfo 			= &vka_info[0];
		vki_info[0].enabledLayerCount 			= VLID_LRS;
		vki_info[0].ppEnabledLayerNames 		= validation_layers;
		vki_info[0].enabledExtensionCount 		= INST_EXS;
		vki_info[0].ppEnabledExtensionNames		= instance_extensions;

	VkInstance vki[1];
	vr("vkCreateInstance", &vkres, 
		vkCreateInstance(&vki_info[0], NULL, &vki[0]) );

	VkDebugUtilsMessengerCreateInfoEXT vkdum_info[1];
		vkdum_info[0].sType	= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	nf(&vkdum_info[0]);
		vkdum_info[0].messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkdum_info[0].messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkdum_info[0].pfnUserCallback	= debugCallback;

	VkDebugUtilsMessengerEXT vkdum[1];
	rv("vkGetInstanceProcAddr");
	auto vkgipa_vkcdum = (PFN_vkCreateDebugUtilsMessengerEXT) 
		vkGetInstanceProcAddr( vki[0], "vkCreateDebugUtilsMessengerEXT" );
	vr("vkCreateDebugUtilsMessengerEXT", &vkres, 
		vkgipa_vkcdum(vki[0], &vkdum_info[0], NULL, &vkdum[0]) );

	uint32_t pd_cnt = UINT32_MAX;
	vr("vkEnumeratePhysicalDevices", &vkres, 
		vkEnumeratePhysicalDevices(vki[0], &pd_cnt, NULL) );
		ov("PhysicalDevices", pd_cnt);
	VkPhysicalDevice vkpd[pd_cnt];
	vr("vkEnumeratePhysicalDevices", &vkres, 
		vkEnumeratePhysicalDevices(vki[0], &pd_cnt, vkpd) );

	VkPhysicalDeviceProperties vkpd_props[pd_cnt];
	for(int i = 0; i < pd_cnt; i++) {
		rv("vkGetPhysicalDeviceProperties");
			vkGetPhysicalDeviceProperties(vkpd[i], &vkpd_props[i]);
		if(	PD_IDX == UINT32_MAX 
		&&	vkpd_props[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
			PD_IDX = i;
		}
	}

	uint32_t pdqfp_cnt = UINT32_MAX;
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vkpd[PD_IDX], &pdqfp_cnt, NULL);
		ov("Queue Families", pdqfp_cnt);
	VkQueueFamilyProperties vkqfamprops[pdqfp_cnt];
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vkpd[PD_IDX], &pdqfp_cnt, vkqfamprops);
	
	for(int i = 0; i < pdqfp_cnt; i++) {
		if(	GQF_IDX == UINT32_MAX 
		&&	vkqfamprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
			GQF_IDX = i; } }

	uint32_t gfxq_cnt = vkqfamprops[GQF_IDX].queueCount;
	float queue_priorities[gfxq_cnt];
	for(int i = 0; i < gfxq_cnt; i++) { queue_priorities[i] = 0.0f; }

	iv("Queues", 		gfxq_cnt, GQF_IDX							);
	iv("Queue Flags", 	vkqfamprops[GQF_IDX].queueFlags, GQF_IDX	);

	VkDeviceQueueCreateInfo vkdq_info[1];
		vkdq_info[GQF_IDX].sType	= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	nf(&vkdq_info[GQF_IDX]);
		vkdq_info[GQF_IDX].queueFamilyIndex		= GQF_IDX;
		vkdq_info[GQF_IDX].queueCount			= gfxq_cnt;
		vkdq_info[GQF_IDX].pQueuePriorities		= queue_priorities; 

	VkDeviceCreateInfo vkld_info[1];
		vkld_info[0].sType 	= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	nf(&vkld_info[0]);
		vkld_info[0].queueCreateInfoCount 		= 1;
		vkld_info[0].pQueueCreateInfos 			= vkdq_info;
		vkld_info[0].enabledLayerCount 			= 0;
		vkld_info[0].ppEnabledLayerNames 		= NULL;
		vkld_info[0].enabledExtensionCount 		= 1;
		vkld_info[0].ppEnabledExtensionNames 	= device_extensions;
		vkld_info[0].pEnabledFeatures 			= NULL;

	VkDevice vkld[1];
	vr("vkCreateDevice", &vkres, 
		vkCreateDevice(vkpd[PD_IDX], &vkld_info[0], NULL, &vkld[0]) );
		ov("VkDevice vkld[0]", vkld[0]);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DISPLAY");	/**/
	///////////////////////////////////////

	rv("XOpenDisplay");
	Display *d = 
		XOpenDisplay(NULL);
		ov("DisplayString", DisplayString(d));

	XSetWindowAttributes xswa;
	xswa.event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

	Window rw = XDefaultRootWindow(d);
	rv("XCreateWindow");
	Window w = 
		XCreateWindow ( d, rw, 0, 0, APP_W, APP_H, 0, 
						CopyFromParent, CopyFromParent, CopyFromParent, 0, &xswa );
		ov("Window", w);

	rv("XMapWindow");
		XMapWindow(d, w);

	rv("XFlush");
		XFlush(d);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SURFACE");	/**/
	///////////////////////////////////////

	VkXlibSurfaceCreateInfoKHR vkxls_info[1];
		vkxls_info[0].sType 	= VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	nf(&vkxls_info[0]);
		vkxls_info[0].dpy 		= d;
		vkxls_info[0].window	= w;

	VkSurfaceKHR vksurf[1];
	vr("vkCreateXlibSurfaceKHR", &vkres, 
		vkCreateXlibSurfaceKHR(vki[0], &vkxls_info[0], NULL, &vksurf[0]) );

	VkSurfaceCapabilitiesKHR vksurf_ables[1];
	vr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkpd[PD_IDX], vksurf[0], &vksurf_ables[0]) );
		ov("minImageCount", 		vksurf_ables[0].minImageCount			);
		ov("maxImageCount", 		vksurf_ables[0].maxImageCount			);
		ov("currentExtent.width", 	vksurf_ables[0].currentExtent.width		);
		ov("currentExtent.height", 	vksurf_ables[0].currentExtent.height	);

	uint32_t pd_surf_fmt_cnt = UINT32_MAX;
	vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkpd[PD_IDX], vksurf[0], 
			&pd_surf_fmt_cnt, NULL) );
		ov("PhysicalDeviceSurfaceFormats", pd_surf_fmt_cnt);
	VkSurfaceFormatKHR vksurf_fmt[pd_surf_fmt_cnt];
	vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkpd[PD_IDX], vksurf[0], 
			&pd_surf_fmt_cnt, vksurf_fmt) );
		for(int i = 0; i < pd_surf_fmt_cnt; i++) { 
			iv("VkFormat", 			vksurf_fmt[i].format, 		i); 
			iv("VkColorSpaceKHR", 	vksurf_fmt[i].colorSpace, 	i); 
		}

	for(int i = 0; i < pd_surf_fmt_cnt; i++) {
		if(	SURF_FMT == UINT32_MAX 
		&&	vksurf_fmt[i].colorSpace 	== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		&&	vksurf_fmt[i].format 		== VK_FORMAT_B8G8R8A8_UNORM 			) {
			SURF_FMT = i;
		}
	}

	uint32_t pd_surf_presmode_cnt = UINT32_MAX;
	vr("vkGetPhysicalDeviceSurfacePresentModesKHR", &vkres, 
		vkGetPhysicalDeviceSurfacePresentModesKHR(	vkpd[PD_IDX], vksurf[0],
													&pd_surf_presmode_cnt, NULL			) );
		ov("PhysicalDeviceSurfacePresentModes", pd_surf_fmt_cnt);
	VkPresentModeKHR vkpresmode[pd_surf_presmode_cnt];
	vr("vkGetPhysicalDeviceSurfacePresentModesKHR", &vkres, 
		vkGetPhysicalDeviceSurfacePresentModesKHR(	vkpd[PD_IDX], vksurf[0],
													&pd_surf_presmode_cnt, vkpresmode	) );
		for(int i = 0; i < pd_surf_presmode_cnt; i++) { 
			iv("VkPresentModeKHR", 	vkpresmode[i], i); 
		}

	VkBool32 pd_surf_supported[1];
	vr("vkGetPhysicalDeviceSurfaceSupportKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceSupportKHR(	vkpd[PD_IDX], GQF_IDX, vksurf[0], 
												&pd_surf_supported[0] 				) );
		ov("Surface Supported", (pd_surf_supported[0]?"TRUE":"FALSE"));

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SHADERS");	/**/
	///////////////////////////////////////

	ShaderCodeInfo shader_info_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_vert[i] = getShaderCodeInfo(filepath_vert[i]);
	}

	VkShaderModuleCreateInfo vkshademod_vert_info[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		vkshademod_vert_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_vert_info[i]);
		vkshademod_vert_info[i].codeSize	= shader_info_vert[i].shaderBytes;
		vkshademod_vert_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_vert[i].shaderData.data());
		iv("Vertex shaderFilename", 	 shader_info_vert[i].shaderFilename, 					i);
		iv("Vertex shaderBytes", 		 shader_info_vert[i].shaderBytes, 						i);
		iv("Vertex shaderBytesValid", 	(shader_info_vert[i].shaderBytesValid?"TRUE":"FALSE"), 	i);
	}

	ShaderCodeInfo shader_info_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_frag[i] = getShaderCodeInfo(filepath_frag[i]);
	}

	VkShaderModuleCreateInfo vkshademod_frag_info[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		vkshademod_frag_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_frag_info[i]);
		vkshademod_frag_info[i].codeSize	= shader_info_frag[i].shaderBytes;
		vkshademod_frag_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_frag[i].shaderData.data());
		iv("Fragment shaderFilename", 	 shader_info_frag[i].shaderFilename, 					i);
		iv("Fragment shaderBytes", 		 shader_info_frag[i].shaderBytes, 						i);
		iv("Fragment shaderBytesValid", (shader_info_frag[i].shaderBytesValid?"TRUE":"FALSE"), 	i);
	}

	VkShaderModule vkshademod_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		vr("vkCreateShaderModule", &vkres, 
			vkCreateShaderModule(vkld[0], &vkshademod_vert_info[i], NULL, &vkshademod_vert[i]) );
	}

	VkShaderModule vkshademod_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		vr("vkCreateShaderModule", &vkres, 
			vkCreateShaderModule(vkld[0], &vkshademod_frag_info[i], NULL, &vkshademod_frag[i]) );
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "PIPELINE");	/**/
	///////////////////////////////////////

	VkPipelineShaderStageCreateInfo vkgfxpipe_ss_info[SHDRSTGS];
		vkgfxpipe_ss_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[0]);
		vkgfxpipe_ss_info[0].stage					= VK_SHADER_STAGE_VERTEX_BIT;
		vkgfxpipe_ss_info[0].module					= vkshademod_vert[0];
		vkgfxpipe_ss_info[0].pName					= "main";
		vkgfxpipe_ss_info[0].pSpecializationInfo	= NULL;
		vkgfxpipe_ss_info[1].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[1]);
		vkgfxpipe_ss_info[1].stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgfxpipe_ss_info[1].module					= vkshademod_frag[1];
		vkgfxpipe_ss_info[1].pName					= "main";
		vkgfxpipe_ss_info[1].pSpecializationInfo	= NULL;

	VkPipelineVertexInputStateCreateInfo vkgfxpipe_vertins_info[1];
		vkgfxpipe_vertins_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	nf(&vkgfxpipe_vertins_info[0]);
		vkgfxpipe_vertins_info[0].vertexBindingDescriptionCount		= 0;
		vkgfxpipe_vertins_info[0].pVertexBindingDescriptions		= NULL;
		vkgfxpipe_vertins_info[0].vertexAttributeDescriptionCount	= 0;
		vkgfxpipe_vertins_info[0].pVertexAttributeDescriptions		= NULL;

	VkPipelineInputAssemblyStateCreateInfo vkgfxpipe_ias_info[1];
		vkgfxpipe_ias_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	nf(&vkgfxpipe_ias_info[0]);
		vkgfxpipe_ias_info[0].topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		vkgfxpipe_ias_info[0].primitiveRestartEnable	= VK_FALSE;

	VkViewport vkgfxpipe_viewport[1];
		vkgfxpipe_viewport[0].x			= 0;
		vkgfxpipe_viewport[0].y			= 0;
		vkgfxpipe_viewport[0].width		= vksurf_ables[0].currentExtent.width*VP_SCALE;
		vkgfxpipe_viewport[0].height	= vksurf_ables[0].currentExtent.height*VP_SCALE;
		vkgfxpipe_viewport[0].minDepth	= 0.0f;
		vkgfxpipe_viewport[0].maxDepth	= 1.0f;

	VkRect2D vkgfxpipe_sciz[1];
		vkgfxpipe_sciz[0].offset.x	= 0;
		vkgfxpipe_sciz[0].offset.y	= 0;
		vkgfxpipe_sciz[0].extent	= vksurf_ables[0].currentExtent;

	VkPipelineViewportStateCreateInfo vkgfxpipe_viewport_info[1];
		vkgfxpipe_viewport_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	nf(&vkgfxpipe_viewport_info[0]);
		vkgfxpipe_viewport_info[0].viewportCount	= 1;
		vkgfxpipe_viewport_info[0].pViewports		= vkgfxpipe_viewport;
		vkgfxpipe_viewport_info[0].scissorCount		= 1;
		vkgfxpipe_viewport_info[0].pScissors		= vkgfxpipe_sciz;

	VkPipelineRasterizationStateCreateInfo vkgfxpipe_rast_info[1];
		vkgfxpipe_rast_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	nf(&vkgfxpipe_rast_info[0]);
		vkgfxpipe_rast_info[0].depthClampEnable			= VK_FALSE;
		vkgfxpipe_rast_info[0].rasterizerDiscardEnable	= VK_FALSE;
		vkgfxpipe_rast_info[0].polygonMode				= VK_POLYGON_MODE_FILL;
		vkgfxpipe_rast_info[0].cullMode					= VK_CULL_MODE_NONE;
		vkgfxpipe_rast_info[0].frontFace				= VK_FRONT_FACE_CLOCKWISE;
		vkgfxpipe_rast_info[0].depthBiasEnable			= VK_FALSE;
		vkgfxpipe_rast_info[0].depthBiasConstantFactor	= 0.0f;
		vkgfxpipe_rast_info[0].depthBiasClamp			= 0.0f;
		vkgfxpipe_rast_info[0].depthBiasSlopeFactor		= 0.0f;
		vkgfxpipe_rast_info[0].lineWidth				= 1.0f;

	VkPipelineMultisampleStateCreateInfo vkgfxpipe_ms_info[1];
		vkgfxpipe_ms_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	nf(&vkgfxpipe_ms_info[0]);
		vkgfxpipe_ms_info[0].rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		vkgfxpipe_ms_info[0].sampleShadingEnable	= VK_FALSE;
		vkgfxpipe_ms_info[0].minSampleShading		= 0.0f;
		vkgfxpipe_ms_info[0].pSampleMask			= NULL;
		vkgfxpipe_ms_info[0].alphaToCoverageEnable	= VK_FALSE;
		vkgfxpipe_ms_info[0].alphaToOneEnable		= VK_FALSE;

	VkPipelineColorBlendAttachmentState vkgfxpipe_colblend_ats[1];
		vkgfxpipe_colblend_ats[0].blendEnable			= VK_FALSE;
		vkgfxpipe_colblend_ats[0].srcColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].colorBlendOp			= VK_BLEND_OP_ADD;
		vkgfxpipe_colblend_ats[0].srcAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].alphaBlendOp			= VK_BLEND_OP_ADD;
		vkgfxpipe_colblend_ats[0].colorWriteMask		= 15;

	VkPipelineColorBlendStateCreateInfo vkgfxpipe_colblend_info[1];
		vkgfxpipe_colblend_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	nf(&vkgfxpipe_colblend_info[0]);
		vkgfxpipe_colblend_info[0].logicOpEnable		= VK_FALSE;
		vkgfxpipe_colblend_info[0].logicOp				= VK_LOGIC_OP_NO_OP;
		vkgfxpipe_colblend_info[0].attachmentCount		= 1;
		vkgfxpipe_colblend_info[0].pAttachments			= vkgfxpipe_colblend_ats;
		vkgfxpipe_colblend_info[0].blendConstants[0]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[1]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[2]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[3]	= 1.0f;

	VkDescriptorSetLayoutBinding vkgp_laydes_setbnd[2];
		vkgp_laydes_setbnd[0].binding				= 0;
		vkgp_laydes_setbnd[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkgp_laydes_setbnd[0].descriptorCount		= 1;
		vkgp_laydes_setbnd[0].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgp_laydes_setbnd[0].pImmutableSamplers	= NULL;
		vkgp_laydes_setbnd[1].binding				= 1;
		vkgp_laydes_setbnd[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkgp_laydes_setbnd[1].descriptorCount		= 1;
		vkgp_laydes_setbnd[1].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgp_laydes_setbnd[1].pImmutableSamplers	= NULL;

	VkDescriptorSetLayoutCreateInfo vkgp_laydes_info[1];
		vkgp_laydes_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&vkgp_laydes_info[0]);
		vkgp_laydes_info[0].bindingCount	= 2;
		vkgp_laydes_info[0].pBindings		= vkgp_laydes_setbnd;
		
	VkDescriptorSetLayout vkgp_laydes[2];
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[0]) );
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[1]) );

	VkPipelineLayoutCreateInfo vkgp_lay_info[2];
		vkgp_lay_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&vkgp_lay_info[0]);
		vkgp_lay_info[0].setLayoutCount			= 1;
		vkgp_lay_info[0].pSetLayouts			= vkgp_laydes;
		vkgp_lay_info[0].pushConstantRangeCount	= 0;
		vkgp_lay_info[0].pPushConstantRanges	= NULL;

	VkPipelineLayout vkgp_lay[1];
	vr("vkCreatePipelineLayout", &vkres, 
		vkCreatePipelineLayout(vkld[0], &vkgp_lay_info[0], NULL, &vkgp_lay[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "RNDPASS");	/**/
	///////////////////////////////////////

		vkgfxpipe_ss_info[1].module					= vkshademod_frag[0];

	VkAttachmentDescription vkatd_init[1];
		vkatd_init[0].flags							= 0;
		vkatd_init[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_init[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_init[0].loadOp						= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkatd_init[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_init[0].stencilLoadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_init[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_init[0].initialLayout					= VK_IMAGE_LAYOUT_UNDEFINED;
		vkatd_init[0].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference vkatref_init[1];
		vkatref_init[0].attachment					= 0;
		vkatref_init[0].layout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpass_init[1];
		vksubpass_init[0].flags						= 0;
		vksubpass_init[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_init[0].inputAttachmentCount		= 0;
		vksubpass_init[0].pInputAttachments			= NULL;
		vksubpass_init[0].colorAttachmentCount		= 1;
		vksubpass_init[0].pColorAttachments			= &vkatref_init[0];
		vksubpass_init[0].pResolveAttachments		= NULL;
		vksubpass_init[0].pDepthStencilAttachment	= NULL;
		vksubpass_init[0].preserveAttachmentCount	= 0;
		vksubpass_init[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_init_dep[1];
		vksubpass_init_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_init_dep[0].dstSubpass			= 0;
		vksubpass_init_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_init_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_init_dep[0].srcAccessMask			= 0;
		vksubpass_init_dep[0].dstAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_init_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_init_info[1];
		vkrp_init_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_init_info[0]);
		vkrp_init_info[0].attachmentCount			= 1;
		vkrp_init_info[0].pAttachments				= vkatd_init;
		vkrp_init_info[0].subpassCount				= 1;
		vkrp_init_info[0].pSubpasses				= vksubpass_init;
		vkrp_init_info[0].dependencyCount			= 1;
		vkrp_init_info[0].pDependencies				= vksubpass_init_dep;
	VkRenderPass vkrp_init[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_init_info[0], NULL, &vkrp_init[0]) );
	VkGraphicsPipelineCreateInfo vkgp_init_info[1];
		vkgp_init_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_init_info[0]);
		vkgp_init_info[0].stageCount				= 2;
		vkgp_init_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_init_info[0].pVertexInputState			= &vkgfxpipe_vertins_info[0];
		vkgp_init_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_init_info[0].pTessellationState		= NULL;
		vkgp_init_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_init_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_init_info[0].pMultisampleState			= &vkgfxpipe_ms_info[0];
		vkgp_init_info[0].pDepthStencilState		= NULL;
		vkgp_init_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_init_info[0].pDynamicState				= NULL;
		vkgp_init_info[0].layout					= vkgp_lay[0];
		vkgp_init_info[0].renderPass				= vkrp_init[0];
		vkgp_init_info[0].subpass					= 0;
		vkgp_init_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_init_info[0].basePipelineIndex			= -1;
	VkPipeline vkgfxpipe_init[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_init_info, NULL, &vkgfxpipe_init[0] ) );

	VkAttachmentDescription vkatd_i2l[1];
		vkatd_i2l[0].flags							= 0;
		vkatd_i2l[0].format							= vksurf_fmt[SURF_FMT].format;
		vkatd_i2l[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_i2l[0].loadOp							= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_i2l[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_i2l[0].stencilLoadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_i2l[0].stencilStoreOp					= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_i2l[0].initialLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_i2l[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_i2l[1];
		vkatref_i2l[0].attachment					= 0;
		vkatref_i2l[0].layout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_i2l[1];
		vksubpass_i2l[0].flags						= 0;
		vksubpass_i2l[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_i2l[0].inputAttachmentCount		= 0;
		vksubpass_i2l[0].pInputAttachments			= NULL;
		vksubpass_i2l[0].colorAttachmentCount		= 0;
		vksubpass_i2l[0].pColorAttachments			= NULL;
		vksubpass_i2l[0].pResolveAttachments		= NULL;
		vksubpass_i2l[0].pDepthStencilAttachment	= NULL;
		vksubpass_i2l[0].preserveAttachmentCount	= 0;
		vksubpass_i2l[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_i2l_dep[1];
		vksubpass_i2l_dep[0].srcSubpass				= VK_SUBPASS_EXTERNAL;
		vksubpass_i2l_dep[0].dstSubpass				= 0;
		vksubpass_i2l_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_i2l_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_i2l_dep[0].srcAccessMask			= 0;
		vksubpass_i2l_dep[0].dstAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_i2l_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_i2l_info[1];
		vkrp_i2l_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_i2l_info[0]);
		vkrp_i2l_info[0].attachmentCount			= 1;
		vkrp_i2l_info[0].pAttachments				= vkatd_i2l;
		vkrp_i2l_info[0].subpassCount				= 1;
		vkrp_i2l_info[0].pSubpasses					= vksubpass_i2l;
		vkrp_i2l_info[0].dependencyCount			= 1;
		vkrp_i2l_info[0].pDependencies				= vksubpass_i2l_dep;
	VkRenderPass vkrp_i2l[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_i2l_info[0], NULL, &vkrp_i2l[0]) );
	VkGraphicsPipelineCreateInfo vkgp_i2l_info[1];
		vkgp_i2l_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_i2l_info[0]);
		vkgp_i2l_info[0].stageCount					= 1;
		vkgp_i2l_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_i2l_info[0].pVertexInputState			= &vkgfxpipe_vertins_info[0];
		vkgp_i2l_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_i2l_info[0].pTessellationState			= NULL;
		vkgp_i2l_info[0].pViewportState				= &vkgfxpipe_viewport_info[0];
		vkgp_i2l_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_i2l_info[0].pMultisampleState			= &vkgfxpipe_ms_info[0];
		vkgp_i2l_info[0].pDepthStencilState			= NULL;
		vkgp_i2l_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_i2l_info[0].pDynamicState				= NULL;
		vkgp_i2l_info[0].layout						= vkgp_lay[0];
		vkgp_i2l_info[0].renderPass					= vkrp_i2l[0];
		vkgp_i2l_info[0].subpass					= 0;
		vkgp_i2l_info[0].basePipelineHandle			= VK_NULL_HANDLE;
		vkgp_i2l_info[0].basePipelineIndex			= -1;
	VkPipeline vkgfxpipe_i2l[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_i2l_info, NULL, &vkgfxpipe_i2l[0] ) );

	VkAttachmentDescription vkatd_initSCR[1];
		vkatd_initSCR[0].flags						= 0;
		vkatd_initSCR[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_initSCR[0].samples					= VK_SAMPLE_COUNT_1_BIT;
		vkatd_initSCR[0].loadOp						= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_initSCR[0].storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_initSCR[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_initSCR[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_initSCR[0].initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		vkatd_initSCR[0].finalLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	VkAttachmentReference vkatref_initSCR[1];
		vkatref_initSCR[0].attachment				= 0;
		vkatref_initSCR[0].layout					= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	VkSubpassDescription vksubpass_initSCR[1];
		vksubpass_initSCR[0].flags						= 0;
		vksubpass_initSCR[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_initSCR[0].inputAttachmentCount		= 0;
		vksubpass_initSCR[0].pInputAttachments			= NULL;
		vksubpass_initSCR[0].colorAttachmentCount		= 0;
		vksubpass_initSCR[0].pColorAttachments			= NULL;
		vksubpass_initSCR[0].pResolveAttachments		= NULL;
		vksubpass_initSCR[0].pDepthStencilAttachment	= NULL;
		vksubpass_initSCR[0].preserveAttachmentCount	= 0;
		vksubpass_initSCR[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_initSCR_dep[1];
		vksubpass_initSCR_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_initSCR_dep[0].dstSubpass			= 0;
		vksubpass_initSCR_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_initSCR_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_initSCR_dep[0].srcAccessMask		= 0;
		vksubpass_initSCR_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_initSCR_dep[0].dependencyFlags	= 0;
	VkRenderPassCreateInfo vkrp_initSCR_info[1];
		vkrp_initSCR_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_initSCR_info[0]);
		vkrp_initSCR_info[0].attachmentCount		= 1;
		vkrp_initSCR_info[0].pAttachments			= vkatd_initSCR;
		vkrp_initSCR_info[0].subpassCount			= 1;
		vkrp_initSCR_info[0].pSubpasses				= vksubpass_initSCR;
		vkrp_initSCR_info[0].dependencyCount		= 1;
		vkrp_initSCR_info[0].pDependencies			= vksubpass_initSCR_dep;
	VkRenderPass vkrp_initSCR[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_initSCR_info[0], NULL, &vkrp_initSCR[0]) );
	VkGraphicsPipelineCreateInfo vkgp_initSCR_info[1];
		vkgp_initSCR_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_initSCR_info[0]);
		vkgp_initSCR_info[0].stageCount				= 1;
		vkgp_initSCR_info[0].pStages				= vkgfxpipe_ss_info;
		vkgp_initSCR_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_initSCR_info[0].pInputAssemblyState	= &vkgfxpipe_ias_info[0];
		vkgp_initSCR_info[0].pTessellationState		= NULL;
		vkgp_initSCR_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_initSCR_info[0].pRasterizationState	= &vkgfxpipe_rast_info[0];
		vkgp_initSCR_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_initSCR_info[0].pDepthStencilState		= NULL;
		vkgp_initSCR_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_initSCR_info[0].pDynamicState			= NULL;
		vkgp_initSCR_info[0].layout					= vkgp_lay[0];
		vkgp_initSCR_info[0].renderPass				= vkrp_initSCR[0];
		vkgp_initSCR_info[0].subpass				= 0;
		vkgp_initSCR_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_initSCR_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_initSCR[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_initSCR_info, NULL, &vkgfxpipe_initSCR[0] ) );

	VkAttachmentDescription vkatd_l2SCR[1];
		vkatd_l2SCR[0].flags						= 0;
		vkatd_l2SCR[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_l2SCR[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_l2SCR[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_l2SCR[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_l2SCR[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_l2SCR[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_l2SCR[0].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_l2SCR[0].finalLayout					= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkAttachmentReference vkatref_l2SCR[1];
		vkatref_l2SCR[0].attachment					= 0;
		vkatref_l2SCR[0].layout						= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkSubpassDescription vksubpass_l2SCR[1];
		vksubpass_l2SCR[0].flags					= 0;
		vksubpass_l2SCR[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_l2SCR[0].inputAttachmentCount		= 0;
		vksubpass_l2SCR[0].pInputAttachments		= NULL;
		vksubpass_l2SCR[0].colorAttachmentCount		= 0;
		vksubpass_l2SCR[0].pColorAttachments		= NULL;
		vksubpass_l2SCR[0].pResolveAttachments		= NULL;
		vksubpass_l2SCR[0].pDepthStencilAttachment	= NULL;
		vksubpass_l2SCR[0].preserveAttachmentCount	= 0;
		vksubpass_l2SCR[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_l2SCR_dep[1];
		vksubpass_l2SCR_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_l2SCR_dep[0].dstSubpass			= 0;
		vksubpass_l2SCR_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_l2SCR_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_l2SCR_dep[0].srcAccessMask		= 0;
		vksubpass_l2SCR_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_l2SCR_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_l2SCR_info[1];
		vkrp_l2SCR_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_l2SCR_info[0]);
		vkrp_l2SCR_info[0].attachmentCount			= 1;
		vkrp_l2SCR_info[0].pAttachments				= vkatd_l2SCR;
		vkrp_l2SCR_info[0].subpassCount				= 1;
		vkrp_l2SCR_info[0].pSubpasses				= vksubpass_l2SCR;
		vkrp_l2SCR_info[0].dependencyCount			= 1;
		vkrp_l2SCR_info[0].pDependencies			= vksubpass_l2SCR_dep;
	VkRenderPass vkrp_l2SCR[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_l2SCR_info[0], NULL, &vkrp_l2SCR[0]) );
	VkGraphicsPipelineCreateInfo vkgp_l2SCR_info[1];
		vkgp_l2SCR_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_l2SCR_info[0]);
		vkgp_l2SCR_info[0].stageCount				= 1;
		vkgp_l2SCR_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_l2SCR_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_l2SCR_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_l2SCR_info[0].pTessellationState		= NULL;
		vkgp_l2SCR_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_l2SCR_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_l2SCR_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_l2SCR_info[0].pDepthStencilState		= NULL;
		vkgp_l2SCR_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_l2SCR_info[0].pDynamicState			= NULL;
		vkgp_l2SCR_info[0].layout					= vkgp_lay[0];
		vkgp_l2SCR_info[0].renderPass				= vkrp_l2SCR[0];
		vkgp_l2SCR_info[0].subpass					= 0;
		vkgp_l2SCR_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_l2SCR_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_l2SCR[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_l2SCR_info, NULL, &vkgfxpipe_l2SCR[0] ) );

	VkAttachmentDescription vkatd_l2SCR1[1];
		vkatd_l2SCR1[0].flags						= 0;
		vkatd_l2SCR1[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_l2SCR1[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_l2SCR1[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_l2SCR1[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_l2SCR1[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_l2SCR1[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_l2SCR1[0].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_l2SCR1[0].finalLayout					= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkAttachmentReference vkatref_l2SCR1[1];
		vkatref_l2SCR1[0].attachment				= 0;
		vkatref_l2SCR1[0].layout					= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkSubpassDescription vksubpass_l2SCR1[1];
		vksubpass_l2SCR1[0].flags					= 0;
		vksubpass_l2SCR1[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_l2SCR1[0].inputAttachmentCount	= 0;
		vksubpass_l2SCR1[0].pInputAttachments		= NULL;
		vksubpass_l2SCR1[0].colorAttachmentCount	= 0;
		vksubpass_l2SCR1[0].pColorAttachments		= NULL;
		vksubpass_l2SCR1[0].pResolveAttachments		= NULL;
		vksubpass_l2SCR1[0].pDepthStencilAttachment	= NULL;
		vksubpass_l2SCR1[0].preserveAttachmentCount	= 0;
		vksubpass_l2SCR1[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_l2SCR1_dep[1];
		vksubpass_l2SCR1_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_l2SCR1_dep[0].dstSubpass			= 0;
		vksubpass_l2SCR1_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_l2SCR1_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_l2SCR1_dep[0].srcAccessMask		= 0;
		vksubpass_l2SCR1_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_l2SCR1_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_l2SCR1_info[1];
		vkrp_l2SCR1_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_l2SCR1_info[0]);
		vkrp_l2SCR1_info[0].attachmentCount			= 1;
		vkrp_l2SCR1_info[0].pAttachments			= vkatd_l2SCR1;
		vkrp_l2SCR1_info[0].subpassCount			= 1;
		vkrp_l2SCR1_info[0].pSubpasses				= vksubpass_l2SCR1;
		vkrp_l2SCR1_info[0].dependencyCount			= 1;
		vkrp_l2SCR1_info[0].pDependencies			= vksubpass_l2SCR1_dep;
	VkRenderPass vkrp_l2SCR1[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_l2SCR1_info[0], NULL, &vkrp_l2SCR1[0]) );
	VkGraphicsPipelineCreateInfo vkgp_l2SCR1_info[1];
		vkgp_l2SCR1_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_l2SCR1_info[0]);
		vkgp_l2SCR1_info[0].stageCount				= 1;
		vkgp_l2SCR1_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_l2SCR1_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_l2SCR1_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_l2SCR1_info[0].pTessellationState		= NULL;
		vkgp_l2SCR1_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_l2SCR1_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_l2SCR1_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_l2SCR1_info[0].pDepthStencilState		= NULL;
		vkgp_l2SCR1_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_l2SCR1_info[0].pDynamicState			= NULL;
		vkgp_l2SCR1_info[0].layout					= vkgp_lay[0];
		vkgp_l2SCR1_info[0].renderPass				= vkrp_l2SCR1[0];
		vkgp_l2SCR1_info[0].subpass					= 0;
		vkgp_l2SCR1_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_l2SCR1_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_l2SCR1[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_l2SCR1_info, NULL, &vkgfxpipe_l2SCR1[0] ) );

	VkAttachmentDescription vkatd_SCR2l[1];
		vkatd_SCR2l[0].flags						= 0;
		vkatd_SCR2l[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_SCR2l[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_SCR2l[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_SCR2l[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_SCR2l[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_SCR2l[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_SCR2l[0].initialLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkatd_SCR2l[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_SCR2l[1];
		vkatref_SCR2l[0].attachment					= 0;
		vkatref_SCR2l[0].layout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_SCR2l[1];
		vksubpass_SCR2l[0].flags					= 0;
		vksubpass_SCR2l[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_SCR2l[0].inputAttachmentCount		= 0;
		vksubpass_SCR2l[0].pInputAttachments		= NULL;
		vksubpass_SCR2l[0].colorAttachmentCount		= 0;
		vksubpass_SCR2l[0].pColorAttachments		= NULL;
		vksubpass_SCR2l[0].pResolveAttachments		= NULL;
		vksubpass_SCR2l[0].pDepthStencilAttachment	= NULL;
		vksubpass_SCR2l[0].preserveAttachmentCount	= 0;
		vksubpass_SCR2l[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_SCR2l_dep[1];
		vksubpass_SCR2l_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_SCR2l_dep[0].dstSubpass			= 0;
		vksubpass_SCR2l_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_SCR2l_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_SCR2l_dep[0].srcAccessMask		= 0;
		vksubpass_SCR2l_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_SCR2l_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_SCR2l_info[1];
		vkrp_SCR2l_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_SCR2l_info[0]);
		vkrp_SCR2l_info[0].attachmentCount			= 1;
		vkrp_SCR2l_info[0].pAttachments				= vkatd_SCR2l;
		vkrp_SCR2l_info[0].subpassCount				= 1;
		vkrp_SCR2l_info[0].pSubpasses				= vksubpass_SCR2l;
		vkrp_SCR2l_info[0].dependencyCount			= 1;
		vkrp_SCR2l_info[0].pDependencies			= vksubpass_SCR2l_dep;
	VkRenderPass vkrp_SCR2l[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_SCR2l_info[0], NULL, &vkrp_SCR2l[0]) );
	VkGraphicsPipelineCreateInfo vkgp_SCR2l_info[1];
		vkgp_SCR2l_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_SCR2l_info[0]);
		vkgp_SCR2l_info[0].stageCount				= 1;
		vkgp_SCR2l_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_SCR2l_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_SCR2l_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_SCR2l_info[0].pTessellationState		= NULL;
		vkgp_SCR2l_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_SCR2l_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_SCR2l_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_SCR2l_info[0].pDepthStencilState		= NULL;
		vkgp_SCR2l_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_SCR2l_info[0].pDynamicState			= NULL;
		vkgp_SCR2l_info[0].layout					= vkgp_lay[0];
		vkgp_SCR2l_info[0].renderPass				= vkrp_SCR2l[0];
		vkgp_SCR2l_info[0].subpass					= 0;
		vkgp_SCR2l_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_SCR2l_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_SCR2l[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_SCR2l_info, NULL, &vkgfxpipe_SCR2l[0] ) );

	VkAttachmentDescription vkatd_SCR2l1[1];
		vkatd_SCR2l1[0].flags						= 0;
		vkatd_SCR2l1[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_SCR2l1[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_SCR2l1[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_SCR2l1[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_SCR2l1[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_SCR2l1[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_SCR2l1[0].initialLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkatd_SCR2l1[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_SCR2l1[1];
		vkatref_SCR2l1[0].attachment				= 0;
		vkatref_SCR2l1[0].layout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_SCR2l1[1];
		vksubpass_SCR2l1[0].flags					= 0;
		vksubpass_SCR2l1[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_SCR2l1[0].inputAttachmentCount	= 0;
		vksubpass_SCR2l1[0].pInputAttachments		= NULL;
		vksubpass_SCR2l1[0].colorAttachmentCount	= 0;
		vksubpass_SCR2l1[0].pColorAttachments		= NULL;
		vksubpass_SCR2l1[0].pResolveAttachments		= NULL;
		vksubpass_SCR2l1[0].pDepthStencilAttachment	= NULL;
		vksubpass_SCR2l1[0].preserveAttachmentCount	= 0;
		vksubpass_SCR2l1[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_SCR2l1_dep[1];
		vksubpass_SCR2l1_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_SCR2l1_dep[0].dstSubpass			= 0;
		vksubpass_SCR2l1_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_SCR2l1_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_SCR2l1_dep[0].srcAccessMask		= 0;
		vksubpass_SCR2l1_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_SCR2l1_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_SCR2l1_info[1];
		vkrp_SCR2l1_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_SCR2l1_info[0]);
		vkrp_SCR2l1_info[0].attachmentCount			= 1;
		vkrp_SCR2l1_info[0].pAttachments			= vkatd_SCR2l1;
		vkrp_SCR2l1_info[0].subpassCount			= 1;
		vkrp_SCR2l1_info[0].pSubpasses				= vksubpass_SCR2l1;
		vkrp_SCR2l1_info[0].dependencyCount			= 1;
		vkrp_SCR2l1_info[0].pDependencies			= vksubpass_SCR2l1_dep;
	VkRenderPass vkrp_SCR2l1[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_SCR2l1_info[0], NULL, &vkrp_SCR2l1[0]) );
	VkGraphicsPipelineCreateInfo vkgp_SCR2l1_info[1];
		vkgp_SCR2l1_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_SCR2l1_info[0]);
		vkgp_SCR2l1_info[0].stageCount				= 1;
		vkgp_SCR2l1_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_SCR2l1_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_SCR2l1_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_SCR2l1_info[0].pTessellationState		= NULL;
		vkgp_SCR2l1_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_SCR2l1_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_SCR2l1_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_SCR2l1_info[0].pDepthStencilState		= NULL;
		vkgp_SCR2l1_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_SCR2l1_info[0].pDynamicState			= NULL;
		vkgp_SCR2l1_info[0].layout					= vkgp_lay[0];
		vkgp_SCR2l1_info[0].renderPass				= vkrp_SCR2l1[0];
		vkgp_SCR2l1_info[0].subpass					= 0;
		vkgp_SCR2l1_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_SCR2l1_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_SCR2l1[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_SCR2l1_info, NULL, &vkgfxpipe_SCR2l1[0] ) );

		vkgfxpipe_ss_info[1].module					= vkshademod_frag[1];
	VkAttachmentDescription vkatd_loop_0[2];
		vkatd_loop_0[0].flags						= 0;
		vkatd_loop_0[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_0[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_0[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_0[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_0[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_0[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_0[0].initialLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_0[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_0[1].flags						= 0;
		vkatd_loop_0[1].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_0[1].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_0[1].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_0[1].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_0[1].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_0[1].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_0[1].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_0[1].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference vkatref_loop_0[2];
		vkatref_loop_0[0].attachment				= 0;
		vkatref_loop_0[0].layout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatref_loop_0[1].attachment				= 1;
		vkatref_loop_0[1].layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpass_loop_0[1];
		vksubpass_loop_0[0].flags					= 0;
		vksubpass_loop_0[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_loop_0[0].inputAttachmentCount	= 1;
		vksubpass_loop_0[0].pInputAttachments		= &vkatref_loop_0[0];
		vksubpass_loop_0[0].colorAttachmentCount	= 1;
		vksubpass_loop_0[0].pColorAttachments		= &vkatref_loop_0[1];
		vksubpass_loop_0[0].pResolveAttachments		= NULL;
		vksubpass_loop_0[0].pDepthStencilAttachment	= NULL;
		vksubpass_loop_0[0].preserveAttachmentCount	= 0;
		vksubpass_loop_0[0].pPreserveAttachments	= NULL;
	VkSubpassDependency vksubpass_loop_0_dep[1];
		vksubpass_loop_0_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_loop_0_dep[0].dstSubpass			= 0;
		vksubpass_loop_0_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_0_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_0_dep[0].srcAccessMask		= 0;
		vksubpass_loop_0_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_loop_0_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_loop_0_info[1];
		vkrp_loop_0_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_loop_0_info[0]);
		vkrp_loop_0_info[0].attachmentCount			= 2;
		vkrp_loop_0_info[0].pAttachments			= vkatd_loop_0;
		vkrp_loop_0_info[0].subpassCount			= 1;
		vkrp_loop_0_info[0].pSubpasses				= vksubpass_loop_0;
		vkrp_loop_0_info[0].dependencyCount			= 1;
		vkrp_loop_0_info[0].pDependencies			= vksubpass_loop_0_dep;
	VkRenderPass vkrp_loop_0[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_loop_0_info[0], NULL, &vkrp_loop_0[0]) );
	VkGraphicsPipelineCreateInfo vkgp_loop_0_info[1];
		vkgp_loop_0_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_loop_0_info[0]);
		vkgp_loop_0_info[0].stageCount				= SHDRSTGS;
		vkgp_loop_0_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_loop_0_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_loop_0_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_loop_0_info[0].pTessellationState		= NULL;
		vkgp_loop_0_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_loop_0_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_loop_0_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_loop_0_info[0].pDepthStencilState		= NULL;
		vkgp_loop_0_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_loop_0_info[0].pDynamicState			= NULL;
		vkgp_loop_0_info[0].layout					= vkgp_lay[0];
		vkgp_loop_0_info[0].renderPass				= vkrp_loop_0[0];
		vkgp_loop_0_info[0].subpass					= 0;
		vkgp_loop_0_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_loop_0_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_loop_0[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_loop_0_info, NULL, &vkgfxpipe_loop_0[0] ) );

	VkAttachmentDescription vkatd_loop_1[2];
		vkatd_loop_1[0].flags						= 0;
		vkatd_loop_1[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_1[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_1[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_1[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_1[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_1[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_1[0].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_1[0].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_1[1].flags						= 0;
		vkatd_loop_1[1].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_1[1].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_1[1].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_1[1].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_1[1].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_1[1].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_1[1].initialLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_1[1].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_loop_1[2];
		vkatref_loop_1[0].attachment				= 0;
		vkatref_loop_1[0].layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkatref_loop_1[1].attachment				= 1;
		vkatref_loop_1[1].layout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_loop_1[1];
		vksubpass_loop_1[0].flags					= 0;
		vksubpass_loop_1[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_loop_1[0].inputAttachmentCount	= 1;
		vksubpass_loop_1[0].pInputAttachments		= &vkatref_loop_1[1];
		vksubpass_loop_1[0].colorAttachmentCount	= 1;
		vksubpass_loop_1[0].pColorAttachments		= &vkatref_loop_1[0];
		vksubpass_loop_1[0].pResolveAttachments		= NULL;
		vksubpass_loop_1[0].pDepthStencilAttachment	= NULL;
		vksubpass_loop_1[0].preserveAttachmentCount	= 0;
		vksubpass_loop_1[0].pPreserveAttachments	= NULL;
	VkSubpassDependency vksubpass_loop_1_dep[1];
		vksubpass_loop_1_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_loop_1_dep[0].dstSubpass			= 0;
		vksubpass_loop_1_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_1_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_1_dep[0].srcAccessMask		= 0;
		vksubpass_loop_1_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_loop_1_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_loop_1_info[1];
		vkrp_loop_1_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_loop_1_info[0]);
		vkrp_loop_1_info[0].attachmentCount			= 2;
		vkrp_loop_1_info[0].pAttachments			= vkatd_loop_1;
		vkrp_loop_1_info[0].subpassCount			= 1;
		vkrp_loop_1_info[0].pSubpasses				= vksubpass_loop_1;
		vkrp_loop_1_info[0].dependencyCount			= 1;
		vkrp_loop_1_info[0].pDependencies			= vksubpass_loop_1_dep;
	VkRenderPass vkrp_loop_1[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_loop_1_info[0], NULL, &vkrp_loop_1[0]) );
	VkGraphicsPipelineCreateInfo vkgp_loop_1_info[1];
		vkgp_loop_1_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_loop_1_info[0]);
		vkgp_loop_1_info[0].stageCount				= SHDRSTGS;
		vkgp_loop_1_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_loop_1_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_loop_1_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_loop_1_info[0].pTessellationState		= NULL;
		vkgp_loop_1_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_loop_1_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_loop_1_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_loop_1_info[0].pDepthStencilState		= NULL;
		vkgp_loop_1_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_loop_1_info[0].pDynamicState			= NULL;
		vkgp_loop_1_info[0].layout					= vkgp_lay[0];
		vkgp_loop_1_info[0].renderPass				= vkrp_loop_1[0];
		vkgp_loop_1_info[0].subpass					= 0;
		vkgp_loop_1_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_loop_1_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_loop_1[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_loop_1_info, NULL, &vkgfxpipe_loop_1[0] ) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "BUFFERS");	/**/
	///////////////////////////////////////

	VkSwapchainCreateInfoKHR vkswap_info[1];
		vkswap_info[0].sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	nf(&vkswap_info[0]);
		vkswap_info[0].surface					= vksurf[0];
		vkswap_info[0].minImageCount			= vksurf_ables[0].minImageCount;
		vkswap_info[0].imageFormat				= vksurf_fmt[SURF_FMT].format;
		vkswap_info[0].imageColorSpace			= vksurf_fmt[SURF_FMT].colorSpace;
		vkswap_info[0].imageExtent				= vksurf_ables[0].currentExtent;
		vkswap_info[0].imageArrayLayers			= 1;
		vkswap_info[0].imageUsage				= vksurf_ables[0].supportedUsageFlags;
		vkswap_info[0].imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		vkswap_info[0].queueFamilyIndexCount	= 1;
		vkswap_info[0].pQueueFamilyIndices		= &GQF_IDX;
		vkswap_info[0].preTransform				= vksurf_ables[0].currentTransform;
		vkswap_info[0].compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		vkswap_info[0].presentMode				= VK_PRESENT_MODE_IMMEDIATE_KHR;
		vkswap_info[0].clipped					= VK_FALSE;
		vkswap_info[0].oldSwapchain				= VK_NULL_HANDLE;

	VkImageCreateInfo vkimagecreate_info[1];
		vkimagecreate_info[0].sType 	= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vkimagecreate_info[0].pNext 	= NULL;
		vkimagecreate_info[0].flags 				= 0;
		vkimagecreate_info[0].imageType 			= VK_IMAGE_TYPE_2D;
		vkimagecreate_info[0].format 				= vksurf_fmt[SURF_FMT].format;
		vkimagecreate_info[0].extent.width 			= vksurf_ables[0].currentExtent.width;
		vkimagecreate_info[0].extent.height			= vksurf_ables[0].currentExtent.height;
		vkimagecreate_info[0].extent.depth 			= 1;
		vkimagecreate_info[0].mipLevels 			= 1;
		vkimagecreate_info[0].arrayLayers 			= 1;
		vkimagecreate_info[0].samples 				= VK_SAMPLE_COUNT_1_BIT;
		vkimagecreate_info[0].tiling 				= VK_IMAGE_TILING_LINEAR;
//			vkimagecreate_info[0].usage 				= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
/*			vkimagecreate_info[0].usage 
			=	VK_IMAGE_USAGE_TRANSFER_DST_BIT
			|	VK_IMAGE_USAGE_SAMPLED_BIT
			|	VK_IMAGE_USAGE_STORAGE_BIT
			|	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			|	VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;	/**/
		vkimagecreate_info[0].usage 
			=	VK_IMAGE_USAGE_TRANSFER_DST_BIT
			|	VK_IMAGE_USAGE_SAMPLED_BIT;	/**/
		vkimagecreate_info[0].sharingMode 			= vkswap_info[0].imageSharingMode;
		vkimagecreate_info[0].queueFamilyIndexCount = 0;
		vkimagecreate_info[0].pQueueFamilyIndices 	= NULL;
		vkimagecreate_info[0].initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED;
	VkImage vkswap_img_SCR[1];
	vr("vkCreateImage", &vkres, 
		vkCreateImage(vkld[0], &vkimagecreate_info[0], NULL, &vkswap_img_SCR[0]) );
	ov("vkswap_img_SCR", vkswap_img_SCR[0]);

	VkSwapchainKHR vkswap[1];
	vr("vkCreateSwapchainKHR", &vkres, 
		vkCreateSwapchainKHR(vkld[0], &vkswap_info[0], NULL, &vkswap[0]) );

	uint32_t swap_img_cnt = UINT32_MAX;
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, NULL) );
		ov("SwapchainImages", swap_img_cnt);
	VkImage vkswap_img[swap_img_cnt];
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, vkswap_img) );

	VkImageSubresourceRange vkimg_subres[1];
		vkimg_subres[0].aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimg_subres[0].baseMipLevel	= 0;
		vkimg_subres[0].levelCount		= 1;
		vkimg_subres[0].baseArrayLayer	= 0;
		vkimg_subres[0].layerCount		= 1;

	VkImageViewCreateInfo vkimgview_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkimgview_info[i].sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		nf(&vkimgview_info[i]);
			vkimgview_info[i].image				= vkswap_img[i];
			vkimgview_info[i].viewType			= VK_IMAGE_VIEW_TYPE_2D;
			vkimgview_info[i].format			= vksurf_fmt[SURF_FMT].format;
			vkimgview_info[i].components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].subresourceRange	= vkimg_subres[0];
	}
	VkImageView vkimgview[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateImageView", &vkres, 
			vkCreateImageView(vkld[0], &vkimgview_info[i], NULL, &vkimgview[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_init_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_init_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_init_info[i]);
			vkfbuf_init_info[i].renderPass		= vkrp_init[0];
			vkfbuf_init_info[i].attachmentCount	= vkrp_init_info[0].attachmentCount;
			vkfbuf_init_info[i].pAttachments	= &vkimgview[i];
			vkfbuf_init_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_init_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_init_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_init[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_init_info[i], NULL, &vkfbuf_init[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_i2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_i2l_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_i2l_info[i]);
			vkfbuf_i2l_info[i].renderPass		= vkrp_i2l[0];
			vkfbuf_i2l_info[i].attachmentCount	= vkrp_i2l_info[0].attachmentCount;
			vkfbuf_i2l_info[i].pAttachments		= &vkimgview[i];
			vkfbuf_i2l_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_i2l_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_i2l_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_i2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_i2l_info[i], NULL, &vkfbuf_i2l[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_l2SCR_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_l2SCR_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_l2SCR_info[i]);
			vkfbuf_l2SCR_info[i].renderPass			= vkrp_l2SCR[0];
			vkfbuf_l2SCR_info[i].attachmentCount	= vkrp_l2SCR_info[0].attachmentCount;
			vkfbuf_l2SCR_info[i].pAttachments		= &vkimgview[i];
			vkfbuf_l2SCR_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_l2SCR_info[i].height				= vksurf_ables[0].currentExtent.height;
			vkfbuf_l2SCR_info[i].layers				= 1;
	}
	VkFramebuffer vkfbuf_l2SCR[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_l2SCR_info[i], NULL, &vkfbuf_l2SCR[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_SCR2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_SCR2l_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_SCR2l_info[i]);
			vkfbuf_SCR2l_info[i].renderPass			= vkrp_SCR2l[0];
			vkfbuf_SCR2l_info[i].attachmentCount	= vkrp_SCR2l_info[0].attachmentCount;
			vkfbuf_SCR2l_info[i].pAttachments		= &vkimgview[i];
			vkfbuf_SCR2l_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_SCR2l_info[i].height				= vksurf_ables[0].currentExtent.height;
			vkfbuf_SCR2l_info[i].layers				= 1;
	}
	VkFramebuffer vkfbuf_SCR2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_SCR2l_info[i], NULL, &vkfbuf_SCR2l[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_l2SCR1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_l2SCR1_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_l2SCR1_info[i]);
			vkfbuf_l2SCR1_info[i].renderPass		= vkrp_l2SCR1[0];
			vkfbuf_l2SCR1_info[i].attachmentCount	= vkrp_l2SCR1_info[0].attachmentCount;
			vkfbuf_l2SCR1_info[i].pAttachments		= &vkimgview[(i+1)%2];
			vkfbuf_l2SCR1_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_l2SCR1_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_l2SCR1_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_l2SCR1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_l2SCR1_info[i], NULL, &vkfbuf_l2SCR1[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_SCR2l1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_SCR2l1_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_SCR2l1_info[i]);
			vkfbuf_SCR2l1_info[i].renderPass		= vkrp_SCR2l1[0];
			vkfbuf_SCR2l1_info[i].attachmentCount	= vkrp_SCR2l1_info[0].attachmentCount;
			vkfbuf_SCR2l1_info[i].pAttachments		= &vkimgview[(i+1)%2];
			vkfbuf_SCR2l1_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_SCR2l1_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_SCR2l1_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_SCR2l1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_SCR2l1_info[i], NULL, &vkfbuf_SCR2l1[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_loop_0_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_loop_0_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_loop_0_info[i]);
			vkfbuf_loop_0_info[i].renderPass		= vkrp_loop_0[0];
			vkfbuf_loop_0_info[i].attachmentCount	= vkrp_loop_0_info[0].attachmentCount;
			vkfbuf_loop_0_info[i].pAttachments		= vkimgview;
			vkfbuf_loop_0_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_loop_0_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_loop_0_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_loop_0[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_loop_0_info[i], NULL, &vkfbuf_loop_0[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_loop_1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_loop_1_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_loop_1_info[i]);
			vkfbuf_loop_1_info[i].renderPass		= vkrp_loop_1[0];
			vkfbuf_loop_1_info[i].attachmentCount	= vkrp_loop_1_info[0].attachmentCount;
			vkfbuf_loop_1_info[i].pAttachments		= vkimgview;
			vkfbuf_loop_1_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_loop_1_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_loop_1_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_loop_1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_loop_1_info[i], NULL, &vkfbuf_loop_1[i]) );
	}

	VkClearValue vkclear_col_init[1];
		vkclear_col_init[0].color		= { 0.3f, 0.0f, 0.3f, 1.0f };
	VkClearValue vkclear_col_i2l[1];
		vkclear_col_i2l[0].color		= { 0.3f, 0.3f, 0.3f, 1.0f };
	VkClearValue vkclear_col_initSCR[1];
		vkclear_col_initSCR[0].color	= { 0.5f, 0.5f, 0.5f, 1.0f };
	VkClearValue vkclear_col_l2SCR[1];
		vkclear_col_l2SCR[0].color		= { 0.5f, 0.5f, 0.5f, 1.0f };
	VkClearValue vkclear_col_SCR2l[1];
		vkclear_col_SCR2l[0].color		= { 0.5f, 0.5f, 0.5f, 1.0f };
	VkClearValue vkclear_col_l2SCR1[1];
		vkclear_col_l2SCR[0].color		= { 0.5f, 0.5f, 0.5f, 1.0f };
	VkClearValue vkclear_col_SCR2l1[1];
		vkclear_col_SCR2l[0].color		= { 0.5f, 0.5f, 0.5f, 1.0f };
	VkClearValue vkclear_col_loop_0[1];
		vkclear_col_loop_0[0].color		= { 0.0f, 0.2f, 0.3f, 1.0f };
	VkClearValue vkclear_col_loop_1[1];
		vkclear_col_loop_1[0].color		= { 0.6f, 0.3f, 0.0f, 1.0f };

	VkRenderPassBeginInfo vkrpbegin_init_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_init_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_init_info[i].pNext	= NULL;
		vkrpbegin_init_info[i].renderPass		= vkrp_init[0];
		vkrpbegin_init_info[i].framebuffer		= vkfbuf_init[i];
		vkrpbegin_init_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_init_info[i].clearValueCount	= 1;
		vkrpbegin_init_info[i].pClearValues		= vkclear_col_init;
	}

	VkRenderPassBeginInfo vkrpbegin_i2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_i2l_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_i2l_info[i].pNext	= NULL;
		vkrpbegin_i2l_info[i].renderPass		= vkrp_i2l[0];
		vkrpbegin_i2l_info[i].framebuffer		= vkfbuf_i2l[i];
		vkrpbegin_i2l_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_i2l_info[i].clearValueCount	= 1;
		vkrpbegin_i2l_info[i].pClearValues		= vkclear_col_i2l;
	}

	VkRenderPassBeginInfo vkrpbegin_l2SCR_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_l2SCR_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_l2SCR_info[i].pNext	= NULL;
		vkrpbegin_l2SCR_info[i].renderPass		= vkrp_l2SCR[0];
		vkrpbegin_l2SCR_info[i].framebuffer		= vkfbuf_l2SCR[i];
		vkrpbegin_l2SCR_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_l2SCR_info[i].clearValueCount	= 1;
		vkrpbegin_l2SCR_info[i].pClearValues	= vkclear_col_l2SCR;
	}

	VkRenderPassBeginInfo vkrpbegin_SCR2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_SCR2l_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_SCR2l_info[i].pNext	= NULL;
		vkrpbegin_SCR2l_info[i].renderPass		= vkrp_SCR2l[0];
		vkrpbegin_SCR2l_info[i].framebuffer		= vkfbuf_SCR2l[i];
		vkrpbegin_SCR2l_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_SCR2l_info[i].clearValueCount	= 1;
		vkrpbegin_SCR2l_info[i].pClearValues	= vkclear_col_SCR2l;
	}

	VkRenderPassBeginInfo vkrpbegin_l2SCR1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_l2SCR1_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_l2SCR1_info[i].pNext	= NULL;
		vkrpbegin_l2SCR1_info[i].renderPass			= vkrp_l2SCR1[0];
		vkrpbegin_l2SCR1_info[i].framebuffer		= vkfbuf_l2SCR1[i];
		vkrpbegin_l2SCR1_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_l2SCR1_info[i].clearValueCount	= 1;
		vkrpbegin_l2SCR1_info[i].pClearValues		= vkclear_col_l2SCR1;
	}

	VkRenderPassBeginInfo vkrpbegin_SCR2l1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_SCR2l1_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_SCR2l1_info[i].pNext	= NULL;
		vkrpbegin_SCR2l1_info[i].renderPass			= vkrp_SCR2l1[0];
		vkrpbegin_SCR2l1_info[i].framebuffer		= vkfbuf_SCR2l1[i];
		vkrpbegin_SCR2l1_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_SCR2l1_info[i].clearValueCount	= 1;
		vkrpbegin_SCR2l1_info[i].pClearValues		= vkclear_col_SCR2l1;
	}

	VkRenderPassBeginInfo vkrpbegin_loop_0_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_loop_0_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_loop_0_info[i].pNext	= NULL;
		vkrpbegin_loop_0_info[i].renderPass			= vkrp_loop_0[0];
		vkrpbegin_loop_0_info[i].framebuffer		= vkfbuf_loop_0[i];
		vkrpbegin_loop_0_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_loop_0_info[i].clearValueCount	= 1;
		vkrpbegin_loop_0_info[i].pClearValues		= vkclear_col_loop_0;
	}
	VkRenderPassBeginInfo vkrpbegin_loop_1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_loop_1_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_loop_1_info[i].pNext	= NULL;
		vkrpbegin_loop_1_info[i].renderPass			= vkrp_loop_1[0];
		vkrpbegin_loop_1_info[i].framebuffer		= vkfbuf_loop_1[i];
		vkrpbegin_loop_1_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_loop_1_info[i].clearValueCount	= 1;
		vkrpbegin_loop_1_info[i].pClearValues		= vkclear_col_loop_1;
	}

	VkCommandPoolCreateInfo vkcompool_info[1];
		vkcompool_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	nf(&vkcompool_info[0]);
		vkcompool_info[0].queueFamilyIndex	= GQF_IDX;

	VkCommandPool vkcompool[1];
	vr("vkCreateCommandPool", &vkres, 
		vkCreateCommandPool(vkld[0], &vkcompool_info[0], NULL, &vkcompool[0]) );

	VkCommandBufferAllocateInfo vkcombuf_alloc_info[1];
		vkcombuf_alloc_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vkcombuf_alloc_info[0].pNext	= NULL;
		vkcombuf_alloc_info[0].commandPool			= vkcompool[0];
		vkcombuf_alloc_info[0].level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkcombuf_alloc_info[0].commandBufferCount	= swap_img_cnt;

	VkCommandBuffer vkcombuf_init[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_init) );
	}

	VkCommandBuffer vkcombuf_i2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_i2l) );
	}

	VkCommandBuffer vkcombuf_initSCR[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_initSCR) );
	}

	VkCommandBuffer vkcombuf_l2SCR[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_l2SCR) );
	}

	VkCommandBuffer vkcombuf_SCR2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_SCR2l) );
	}

	VkCommandBuffer vkcombuf_l2SCR1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_l2SCR1) );
	}

	VkCommandBuffer vkcombuf_SCR2l1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_SCR2l1) );
	}

	VkCommandBuffer vkcombuf_SCR[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_SCR) );
	}

	VkCommandBuffer vkcombuf_SCR1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_SCR1) );
	}

	VkCommandBuffer vkcombuf_loop[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_loop) );
	}

	VkCommandBufferBeginInfo vkcombufbegin_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkcombufbegin_info[i].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		nf(&vkcombufbegin_info[i]);
			vkcombufbegin_info[i].pInheritanceInfo	= NULL;
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DESCRIP");	/**/
	///////////////////////////////////////

	VkSamplerCreateInfo vksmplr_info[1];
		vksmplr_info[0].sType	= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	nf(&vksmplr_info[0]);
		vksmplr_info[0].magFilter				= VK_FILTER_NEAREST;
		vksmplr_info[0].minFilter				= VK_FILTER_NEAREST;
		vksmplr_info[0].mipmapMode				= VK_SAMPLER_MIPMAP_MODE_NEAREST;
		vksmplr_info[0].addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].mipLodBias				= 1.0f;
		vksmplr_info[0].anisotropyEnable		= VK_FALSE;
		vksmplr_info[0].maxAnisotropy			= 1.0f;
		vksmplr_info[0].compareEnable			= VK_FALSE;
		vksmplr_info[0].compareOp				= VK_COMPARE_OP_NEVER;
		vksmplr_info[0].minLod					= 1.0f;
		vksmplr_info[0].maxLod					= 1.0f;
		vksmplr_info[0].borderColor				= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		vksmplr_info[0].unnormalizedCoordinates	= VK_FALSE;

	VkSampler vksmplr[1];
	vr("vkCreateSampler", &vkres, 
		vkCreateSampler(vkld[0], &vksmplr_info[0], NULL, &vksmplr[0]) );

	VkDescriptorPoolSize vkdescpool_size[2];
		vkdescpool_size[0].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescpool_size[0].descriptorCount	= 1;
		vkdescpool_size[1].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescpool_size[1].descriptorCount	= 1;

	VkDescriptorPoolCreateInfo vkdescpool_info[1];
		vkdescpool_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&vkdescpool_info[0]);
		vkdescpool_info[0].maxSets			= 1;
		vkdescpool_info[0].poolSizeCount	= 2;
		vkdescpool_info[0].pPoolSizes		= vkdescpool_size;

	VkDescriptorPool vkdescpool[2];
	vr("vkCreateDescriptorPool", &vkres, 
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[0]) );
	vr("vkCreateDescriptorPool", &vkres, 
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[1]) );

	VkDescriptorSetAllocateInfo vkdescset_alloc_info[1];
		vkdescset_alloc_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescset_alloc_info[0].pNext	= NULL;
		vkdescset_alloc_info[0].descriptorPool		= vkdescpool[0];
		vkdescset_alloc_info[0].descriptorSetCount	= vkdescpool_info[0].maxSets;
		vkdescset_alloc_info[0].pSetLayouts			= vkgp_laydes;
		vkdescset_alloc_info[1].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescset_alloc_info[1].pNext	= NULL;
		vkdescset_alloc_info[1].descriptorPool		= vkdescpool[1];
		vkdescset_alloc_info[1].descriptorSetCount	= vkdescpool_info[0].maxSets;
		vkdescset_alloc_info[1].pSetLayouts			= vkgp_laydes;

	VkDescriptorSet vkdescset_0[1];
	vr("vkAllocateDescriptorSets", &vkres, 
		vkAllocateDescriptorSets(vkld[0], &vkdescset_alloc_info[0], &vkdescset_0[0]) );
	VkDescriptorSet vkdescset_1[1];
	vr("vkAllocateDescriptorSets", &vkres, 
		vkAllocateDescriptorSets(vkld[0], &vkdescset_alloc_info[1], &vkdescset_1[0]) );

	VkDescriptorImageInfo vkdesc_img_info_0[1];
		vkdesc_img_info_0[0].sampler		= vksmplr[0];
		vkdesc_img_info_0[0].imageView		= vkimgview[1];
		vkdesc_img_info_0[0].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo vkdesc_img_info_1[1];
		vkdesc_img_info_1[0].sampler		= vksmplr[0];
		vkdesc_img_info_1[0].imageView		= vkimgview[0];
		vkdesc_img_info_1[0].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet vkdescset_write_0[2];
		vkdescset_write_0[0].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_0[0].pNext	= NULL;
		vkdescset_write_0[0].dstSet				= vkdescset_0[0];
		vkdescset_write_0[0].dstBinding			= 0;
		vkdescset_write_0[0].dstArrayElement	= 0;
		vkdescset_write_0[0].descriptorCount	= 1;
		vkdescset_write_0[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescset_write_0[0].pImageInfo			= &vkdesc_img_info_0[0];
		vkdescset_write_0[0].pBufferInfo		= NULL;
		vkdescset_write_0[0].pTexelBufferView	= NULL;

	VkWriteDescriptorSet vkdescset_write_1[2];
		vkdescset_write_1[0].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_1[0].pNext	= NULL;
		vkdescset_write_1[0].dstSet				= vkdescset_1[0];
		vkdescset_write_1[0].dstBinding			= 0;
		vkdescset_write_1[0].dstArrayElement	= 0;
		vkdescset_write_1[0].descriptorCount	= 1;
		vkdescset_write_1[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescset_write_1[0].pImageInfo			= &vkdesc_img_info_1[0];
		vkdescset_write_1[0].pBufferInfo		= NULL;
		vkdescset_write_1[0].pTexelBufferView	= NULL;

//	COMMENTMARKER_0
	VkDeviceSize vk_devsz_buff[1];
		vk_devsz_buff[0]	= (sizeof(float) * 10) + (sizeof(float) * VMX);
		ov("Uniform init_ub size", vk_devsz_buff[0]);
		vk_devsz_buff[0] = (vk_devsz_buff[0] > 256 ? vk_devsz_buff[0] : 256);
		ov("Uniform init_ub size", vk_devsz_buff[0]);

	VkBufferCreateInfo vkbuff_ub_info[1];
		vkbuff_ub_info[0].sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_ub_info[0]);
		vkbuff_ub_info[0].size						= vk_devsz_buff[0];
		vkbuff_ub_info[0].usage						= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		vkbuff_ub_info[0].sharingMode				= vkswap_info[0].imageSharingMode;
		vkbuff_ub_info[0].queueFamilyIndexCount		= 0;
		vkbuff_ub_info[0].pQueueFamilyIndices		= NULL;

	VkBuffer vkbuff_ub[1];
	vr("vkCreateBuffer", &vkres, 
		vkCreateBuffer(vkld[0], &vkbuff_ub_info[0], NULL, &vkbuff_ub[0]) );

	VkMemoryRequirements vkmemreq_vkbuff_ub[1];
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vkld[0], vkbuff_ub[0], &vkmemreq_vkbuff_ub[0]);
		ov("Uniform init_u0 size", 				vkmemreq_vkbuff_ub[0].size				);
		ov("Uniform init_u0 alignment", 		vkmemreq_vkbuff_ub[0].alignment			);
		ov("Uniform init_u0 memoryTypeBits", 	vkmemreq_vkbuff_ub[0].memoryTypeBits	);

	VkDescriptorBufferInfo vkdescbuf_ub_info[1];
		vkdescbuf_ub_info[0].buffer		= vkbuff_ub[0];
		vkdescbuf_ub_info[0].offset		= 0;
		vkdescbuf_ub_info[0].range		= vkmemreq_vkbuff_ub[0].size;

		vkdescset_write_0[1].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_0[1].pNext	= NULL;
		vkdescset_write_0[1].dstSet				= vkdescset_0[0];
		vkdescset_write_0[1].dstBinding			= 1;
		vkdescset_write_0[1].dstArrayElement	= 0;
		vkdescset_write_0[1].descriptorCount	= 1;
		vkdescset_write_0[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescset_write_0[1].pImageInfo			= NULL;
		vkdescset_write_0[1].pBufferInfo		= vkdescbuf_ub_info;
		vkdescset_write_0[1].pTexelBufferView	= NULL;

		vkdescset_write_1[1].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_1[1].pNext	= NULL;
		vkdescset_write_1[1].dstSet				= vkdescset_1[0];
		vkdescset_write_1[1].dstBinding			= 1;
		vkdescset_write_1[1].dstArrayElement	= 0;
		vkdescset_write_1[1].descriptorCount	= 1;
		vkdescset_write_1[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescset_write_1[1].pImageInfo			= NULL;
		vkdescset_write_1[1].pBufferInfo		= vkdescbuf_ub_info;
		vkdescset_write_1[1].pTexelBufferView	= NULL;

	VkPhysicalDeviceMemoryProperties vkpd_memprops[1];
	rv("vkGetPhysicalDeviceMemoryProperties");
		vkGetPhysicalDeviceMemoryProperties(vkpd[PD_IDX], &vkpd_memprops[0]);
		ov("memoryTypeCount", 	vkpd_memprops[0].memoryTypeCount					);
		for(int i = 0; i < 		vkpd_memprops[0].memoryTypeCount; 				i++	) {
			iv("propertyFlags", vkpd_memprops[0].memoryTypes[i].propertyFlags,	i	); 
			iv("heapIndex", 	vkpd_memprops[0].memoryTypes[i].heapIndex, 		i	); }
		ov("memoryHeapCount", 	vkpd_memprops[0].memoryHeapCount				);
		for(int i = 0; i < 		vkpd_memprops[0].memoryHeapCount; 				i++	) {
			iv("size", 			vkpd_memprops[0].memoryHeaps[i].size,			i	); 
			iv("flags", 		vkpd_memprops[0].memoryHeaps[i].flags,			i	); }

		for(int i = 0; i < 		vkpd_memprops[0].memoryTypeCount; 				i++	) {
			if( vkpd_memprops[0].memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			&&	UB_MEMTYPE == UINT32_MAX ) {
				UB_MEMTYPE = i; } }

	VkMemoryRequirements devmem_SCR_reqs[1];
	rv("vkGetImageMemoryRequirements");
		vkGetImageMemoryRequirements(vkld[0], vkswap_img_SCR[0], &devmem_SCR_reqs[0]);

	ov("size", devmem_SCR_reqs[0].size);
	ov("alignment", devmem_SCR_reqs[0].alignment);
	ov("memoryTypeBits", devmem_SCR_reqs[0].memoryTypeBits);

	for(int i = 0; i < vkpd_memprops[0].memoryTypeCount; i++) {
		if( vkpd_memprops[0].memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		&&	SCR_MEMTYPE == UINT32_MAX ) {
			SCR_MEMTYPE = i; } }

	for(int i = 0; i < vkpd_memprops[0].memoryTypeCount; i++) {
		if( (	vkpd_memprops[0].memoryTypes[i].propertyFlags
			& 	(	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				|	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				)
			) 	!= 0 
		) { iv("MEM", vkpd_memprops[0].memoryTypes[i].propertyFlags, i); }
	}

	ov("SCR_MEMTYPE", SCR_MEMTYPE);
	SCR_MEMTYPE = 8;
	ov("SCR_MEMTYPE_OVERWRITE", SCR_MEMTYPE);

	int mem_index = findProperties(
		&vkpd_memprops[0], 
		devmem_SCR_reqs[0].memoryTypeBits,
		0x00000001
	);

	ov("SCR_MEMTYPE_Rec", mem_index);

	VkMemoryAllocateInfo devmem_SCR_allo_info[1];
		devmem_SCR_allo_info[0].sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		devmem_SCR_allo_info[0].pNext	= NULL;
		devmem_SCR_allo_info[0].allocationSize	= devmem_SCR_reqs[0].size;
		devmem_SCR_allo_info[0].memoryTypeIndex	= SCR_MEMTYPE; 

	VkDeviceMemory devmem_SCR[1];
	vr("vkAllocateMemory", &vkres, 
		vkAllocateMemory(vkld[0], &devmem_SCR_allo_info[0], NULL, &devmem_SCR[0]) );

	vr("vkBindImageMemory", &vkres, 
		vkBindImageMemory(vkld[0], vkswap_img_SCR[0], devmem_SCR[0], 0) );

	ov("SCR DevMem", devmem_SCR[0]);

	VkImageViewCreateInfo vkimgview_SCR_info[2];
		vkimgview_SCR_info[0].sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_SCR_info[0]);
		vkimgview_SCR_info[0].image				= vkswap_img_SCR[0];
		vkimgview_SCR_info[0].viewType			= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_SCR_info[0].format			= vksurf_fmt[SURF_FMT].format;
		vkimgview_SCR_info[0].components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[0].components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[0].components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[0].components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[0].subresourceRange	= vkimg_subres[0];

		vkimgview_SCR_info[1].sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_SCR_info[1]);
		vkimgview_SCR_info[1].image				= vkswap_img[0];
		vkimgview_SCR_info[1].viewType			= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_SCR_info[1].format			= vksurf_fmt[SURF_FMT].format;
		vkimgview_SCR_info[1].components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[1].components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[1].components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[1].components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_SCR_info[1].subresourceRange	= vkimg_subres[0];

	VkImageView vkimgview_SCR[2];
	for(int i = 0; i < 2; i++) {
		vr("vkCreateImageView", &vkres, 
			vkCreateImageView(vkld[0], &vkimgview_SCR_info[i], NULL, &vkimgview_SCR[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_initSCR_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_initSCR_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_initSCR_info[i]);
			vkfbuf_initSCR_info[i].renderPass		= vkrp_initSCR[0];
			vkfbuf_initSCR_info[i].attachmentCount	= vkrp_initSCR_info[0].attachmentCount;
			vkfbuf_initSCR_info[i].pAttachments		= &vkimgview_SCR[i];
			vkfbuf_initSCR_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_initSCR_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_initSCR_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_initSCR[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_initSCR_info[i], NULL, &vkfbuf_initSCR[i]) );
	}

	VkRenderPassBeginInfo vkrpbegin_initSCR_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_initSCR_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_initSCR_info[i].pNext	= NULL;
		vkrpbegin_initSCR_info[i].renderPass		= vkrp_initSCR[0];
		vkrpbegin_initSCR_info[i].framebuffer		= vkfbuf_initSCR[i];
		vkrpbegin_initSCR_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_initSCR_info[i].clearValueCount	= 1;
		vkrpbegin_initSCR_info[i].pClearValues		= vkclear_col_initSCR;
	}

	VkMemoryAllocateInfo vkbuff_ub_memallo_info[1];
		vkbuff_ub_memallo_info[0].sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkbuff_ub_memallo_info[0].pNext	= NULL;
		vkbuff_ub_memallo_info[0].allocationSize	= vk_devsz_buff[0];
		vkbuff_ub_memallo_info[0].memoryTypeIndex	= UB_MEMTYPE;

	VkDeviceMemory vkbuff_ub_devmem[1];
	vr("vkAllocateMemory", &vkres, 
		vkAllocateMemory(vkld[0], &vkbuff_ub_memallo_info[0], NULL, &vkbuff_ub_devmem[0]) );

	void *pvoid_memmap;
	vr("vkMapMemory", &vkres, 
		vkMapMemory(vkld[0], vkbuff_ub_devmem[0], 
					vkdescbuf_ub_info[0].offset, 
					vkdescbuf_ub_info[0].range,
					0, &pvoid_memmap) );
//	COMMENTMARKER_0
	init_ub ubo_init_ub[1];
		ubo_init_ub[0].w 		= float(APP_W);
		ubo_init_ub[0].h 		= float(APP_H);
		ubo_init_ub[0].seed 	= float(0.0);
		ubo_init_ub[0].frame 	= float(0.0);
		ubo_init_ub[0].clicks 	= float(0.0);
		ubo_init_ub[0].mx 		= float(0.0);
		ubo_init_ub[0].my 		= float(0.0);
		ubo_init_ub[0].mlb 		= float(0.0);
		ubo_init_ub[0].mrb 		= float(0.0);
		ubo_init_ub[0].div 		= div;
		ubo_init_ub[0].v0 		= float(0.0);
		ubo_init_ub[0].v1 		= float(0.0);
		ubo_init_ub[0].v2 		= float(0.0);
		ubo_init_ub[0].v3 		= float(0.0);
		ubo_init_ub[0].v4 		= float(0.0);
		ubo_init_ub[0].v5 		= float(0.0);
		ubo_init_ub[0].v6 		= float(0.0);
		ubo_init_ub[0].v7 		= float(0.0);
		ubo_init_ub[0].v8 		= float(0.0);
		ubo_init_ub[0].v9 		= float(0.0);
		ubo_init_ub[0].v10 		= float(0.0);
		ubo_init_ub[0].v11 		= float(0.0);
		ubo_init_ub[0].v12 		= float(0.0);
		ubo_init_ub[0].v13 		= float(0.0);
		ubo_init_ub[0].v14 		= float(0.0);
		ubo_init_ub[0].v15 		= float(0.0);
		ubo_init_ub[0].v16 		= float(0.0);
		ubo_init_ub[0].v17 		= float(0.0);
		ubo_init_ub[0].v18 		= float(0.0);
		ubo_init_ub[0].v19 		= float(0.0);
		ubo_init_ub[0].v20 		= float(0.0);
		ubo_init_ub[0].v21 		= float(0.0);
		ubo_init_ub[0].v22 		= float(0.0);
		ubo_init_ub[0].v23 		= float(0.0);
		ubo_init_ub[0].v24 		= float(0.0);
		ubo_init_ub[0].v25 		= float(0.0);
		ubo_init_ub[0].v26 		= float(0.0);
		ubo_init_ub[0].v27 		= float(0.0);
		ubo_init_ub[0].v28 		= float(0.0);
		ubo_init_ub[0].v29 		= float(0.0);
		ubo_init_ub[0].v30 		= float(0.0);
		ubo_init_ub[0].v31 		= float(0.0);

	ov("sizeof(ubo_init_ub[0])", sizeof(ubo_init_ub[0]));

	rv("memcpy");
		memcpy(pvoid_memmap, &ubo_init_ub[0], sizeof(ubo_init_ub[0]));

	vr("vkBindBufferMemory", &vkres, 
		vkBindBufferMemory(vkld[0], vkbuff_ub[0], vkbuff_ub_devmem[0], vkdescbuf_ub_info[0].offset) );

	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 2, vkdescset_write_0, 0, NULL);
	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 2, vkdescset_write_1, 0, NULL);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDINIT");	/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_init[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_init[i], &vkrpbegin_init_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_init[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_init[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_init[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_0, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_init[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_init[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_init[i]) );

	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SYNC");		/**/
	///////////////////////////////////////

	VkQueue vkq[1];
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vkld[0], GQF_IDX, 0, &vkq[0]);

	VkSemaphoreCreateInfo vksemaph_info[1];
		vksemaph_info[0].sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	nf(&vksemaph_info[0]);

	VkSemaphore vksemaph_image[1];
	VkSemaphore vksemaph_rendr[1];

	VkFenceCreateInfo vkfence_info[1];
		vkfence_info[0].sType	= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	nf(&vkfence_info[0]);

	VkFence vkfence_aqimg[1];
	vr("vkCreateFence", &vkres, 
		vkCreateFence(vkld[0], &vkfence_info[0], NULL, &vkfence_aqimg[0]) );

	VkPipelineStageFlags qsubwait	= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	uint32_t aqimg_idx[1];

	if(loglevel != 0) { loglevel = loglevel * -1; }

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_INIT");	/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {
		if(valid) {

			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_init[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDI2L");		/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_i2l[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_i2l[0], &vkrpbegin_i2l_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_i2l[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_i2l[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_I2L");		/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_i2l[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDINITSCR");	/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_initSCR[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_initSCR[0], &vkrpbegin_initSCR_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_initSCR[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_initSCR[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_INITSCR");	/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_initSCR[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDL2SCR");	/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_l2SCR[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_l2SCR[0], &vkrpbegin_l2SCR_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_l2SCR[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_l2SCR[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_L2SCR");	/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_l2SCR[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDSC0");		/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {
		iv("vkswap_img", vkswap_img[i], i);
	}

	VkImageCopy vkimagecopy_SCR[1];
		vkimagecopy_SCR[0].srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimagecopy_SCR[0].srcSubresource.mipLevel			= 0;
		vkimagecopy_SCR[0].srcSubresource.baseArrayLayer	= vkimagecreate_info[0].arrayLayers - 1;
		vkimagecopy_SCR[0].srcSubresource.layerCount		= vkimagecreate_info[0].arrayLayers;
		vkimagecopy_SCR[0].srcOffset.x						= 0;
		vkimagecopy_SCR[0].srcOffset.y						= 0;
		vkimagecopy_SCR[0].srcOffset.z						= 0;
		vkimagecopy_SCR[0].dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimagecopy_SCR[0].dstSubresource.mipLevel			= 0;
		vkimagecopy_SCR[0].dstSubresource.baseArrayLayer	= vkimagecreate_info[0].arrayLayers - 1;
		vkimagecopy_SCR[0].dstSubresource.layerCount		= vkimagecreate_info[0].arrayLayers;
		vkimagecopy_SCR[0].dstOffset.x						= 0;
		vkimagecopy_SCR[0].dstOffset.y						= 0;
		vkimagecopy_SCR[0].dstOffset.z						= 0;
		vkimagecopy_SCR[0].extent							= vkimagecreate_info[0].extent;

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_SCR[0], &vkcombufbegin_info[0]) );

		rv("vkCmdCopyImage");
			vkCmdCopyImage ( 
				vkcombuf_SCR[0], 
				vkswap_img[0], 			
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				vkswap_img_SCR[0], 	
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&vkimagecopy_SCR[0] 
		);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_SCR[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_SC0");		/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_SCR[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDSC02L");	/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_SCR2l[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_SCR2l[0], &vkrpbegin_SCR2l_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_SCR2l[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_SCR2l[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_SC02L");	/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_SCR2l[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDL2SCR1");	/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_l2SCR1[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_l2SCR1[0], &vkrpbegin_l2SCR1_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_l2SCR1[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_l2SCR1[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_L2SCR1");	/**/
	///////////////////////////////////////
/*
	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_l2SCR1[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}
*/
	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDSC1");		/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {
		iv("vkswap_img", vkswap_img[i], i);
	}

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_SCR1[0], &vkcombufbegin_info[0]) );

		rv("vkCmdCopyImage");
			vkCmdCopyImage ( 
				vkcombuf_SCR1[0], 
				vkswap_img[1], 			
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				vkswap_img_SCR[0], 	
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&vkimagecopy_SCR[0] 
		);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_SCR1[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_SC1");		/**/
	///////////////////////////////////////
/*
	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_SCR1[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDSC12L");	/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_SCR2l1[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_SCR2l1[0], &vkrpbegin_SCR2l1_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_SCR2l1[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_SCR2l1[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_SC12L");	/**/
	///////////////////////////////////////
/*
	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_SCR2l1[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SAVEIMG");	/**/
	///////////////////////////////////////

	void* SCR_data;

	vr("vkMapMemory", &vkres, 
		vkMapMemory(vkld[0], devmem_SCR[0], 0, VK_WHOLE_SIZE, 0, &SCR_data) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDLOOP");	/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_loop[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_loop[i], &vkrpbegin_loop_1_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_loop_1[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_0, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_loop[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_loop[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_loop[i]) );

	}

	for(int i = 1; i < 2; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_loop[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_loop[i], &vkrpbegin_loop_0_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_loop_0[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_1, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_loop[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_loop[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_loop[i]) );

	}

	if(loglevel != 0) { loglevel = loglevel * -1; }

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_LOOP");	/**/
	///////////////////////////////////////

	if(loglevel != 0) { loglevel = loglevel * -1; }

	int ilimit = TEST_CYCLES;
	int idx = 0;
	int SCR_count = 0;
	int div_reset = 0;
	bool new_select = 0;
	int div_chan = 0;
	int div_chan_max = VMX;
	int chk_max = 0;
	int chk_seek = 0;
	int pause = 0;

	int rec_on = 0;
	int rec_frames = 12;
	int quit_now = 0;

	float ubv[div_chan_max] = {};
	float chk[div_chan_max] = {};
	float hst[4096][div_chan_max] = {{}};

	float f;
	int fidx = 0;
	std::string floatfile = "fbk/evo" + timestamp + ".float";
	std::ifstream fin(floatfile.c_str(), std::ios::in | std::ios::binary);
    while (fin.read(reinterpret_cast<char*>(&f), sizeof(float))) {
		hst[fidx/32][fidx%div_chan_max] = f;
		fidx++;
	}
	chk_max = fidx/32;

	do {
		if(valid) {
			ubo_init_ub[0].mrb = 0.0;
			ubo_init_ub[0].mlb = 0.0;
			XEvent xe;
			XSelectInput(d, w, ButtonPressMask | KeyPressMask);
			if(XCheckWindowEvent(d, w, ButtonPressMask | KeyPressMask, &xe)) {

				int do_save = 0;

				if(xe.xbutton.button == 10){ div = 1.0; }
				if(xe.xbutton.button == 11){ div = 2.0; }
				if(xe.xbutton.button == 12){ div = 3.0; }
				if(xe.xbutton.button == 13){ div = 4.0; }
				if(xe.xbutton.button == 14){ div = 5.0; }
				if(xe.xbutton.button == 15){ div = 6.0; }
				if(xe.xbutton.button == 16){ div = 7.0; }
				if(xe.xbutton.button == 17){ div = 8.0; }
				if(xe.xbutton.button == 18){ div = 9.0; }
				if(xe.xbutton.button == 19){ div = 10.0; }

				if(xe.xbutton.button == 86){ rec_frames = rec_frames + 1; }
				if(xe.xbutton.button == 82){ rec_frames = (rec_frames > 1) ? rec_frames - 1 : 1; }

				ubo_init_ub[0].div 	= div;
				ubo_init_ub[0].mx 	= (float)xe.xbutton.x;
				ubo_init_ub[0].my 	= (float)xe.xbutton.y;
				float 	divi		= ( floor((ubo_init_ub[0].mx*div)/(ubo_init_ub[0].w)) )
									+ ( floor((ubo_init_ub[0].my*div)/(ubo_init_ub[0].h)) ) * div;
				float 	dspace 		= (divi+1.0) / (div*div);
						dspace 		= (div == 1.0) ? 0.5 : dspace;

				
				ubv[0] 	= ubo_init_ub[0].v0;	ubv[1] 	= ubo_init_ub[0].v1;
				ubv[2] 	= ubo_init_ub[0].v2;	ubv[3] 	= ubo_init_ub[0].v3;
				ubv[4] 	= ubo_init_ub[0].v4;	ubv[5] 	= ubo_init_ub[0].v5;
				ubv[6] 	= ubo_init_ub[0].v6;	ubv[7] 	= ubo_init_ub[0].v7;
				ubv[8] 	= ubo_init_ub[0].v8;	ubv[9] 	= ubo_init_ub[0].v9;
				ubv[10] = ubo_init_ub[0].v10;	ubv[11] = ubo_init_ub[0].v11;
				ubv[12] = ubo_init_ub[0].v12;	ubv[13] = ubo_init_ub[0].v13;
				ubv[14] = ubo_init_ub[0].v14;	ubv[15] = ubo_init_ub[0].v15;
				ubv[16] = ubo_init_ub[0].v16;	ubv[17] = ubo_init_ub[0].v17;
				ubv[18] = ubo_init_ub[0].v18;	ubv[19] = ubo_init_ub[0].v19;
				ubv[20] = ubo_init_ub[0].v20;	ubv[21] = ubo_init_ub[0].v21;
				ubv[22] = ubo_init_ub[0].v22;	ubv[23] = ubo_init_ub[0].v23;
				ubv[24] = ubo_init_ub[0].v24;	ubv[25] = ubo_init_ub[0].v25;
				ubv[26] = ubo_init_ub[0].v26;	ubv[27] = ubo_init_ub[0].v27;
				ubv[28] = ubo_init_ub[0].v28;	ubv[29] = ubo_init_ub[0].v29;
				ubv[30] = ubo_init_ub[0].v30;	ubv[31] = ubo_init_ub[0].v31;


				if(xe.xbutton.button == 4) { ubo_init_ub[0].clicks = ubo_init_ub[0].clicks + 1.0; } 
				if(xe.xbutton.button == 5) { ubo_init_ub[0].clicks = ubo_init_ub[0].clicks - 1.0; } 
				if(xe.xbutton.button == 1) { ubo_init_ub[0].mlb = 1.0; }
				if(xe.xbutton.button == 3) { ubo_init_ub[0].mrb = 1.0; }

				if(xe.xbutton.button == 65) { pause = (pause) ? 0 : 1; }

				if(xe.xbutton.button == 9) {
					ubo_init_ub[0].mrb = 2.0;
					do_save = 1;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						chk[ubi] = ubv[ubi] * dspace * 2.0; } }

				if(xe.xbutton.button == 2 || do_save) {

					std::string floatfile = "fbk/evo" + timestamp + ".float";
					std::ofstream fout(floatfile.c_str(), std::ios::out | std::ios::binary | std::ios::app);
					for(int fi = 0; fi < div_chan_max; fi++) {
						float f = ( (float)((int)((ubv[fi] * dspace)*1000.0))/1000.0 );
						fout.write(
							reinterpret_cast<const char*>( &f ),
							sizeof( float ) ); }
					fout.close();

					std::string floatlast = "fbk/LastEvo.chk";
					std::ofstream flsout(floatlast.c_str(), std::ios::out | std::ios::binary | std::ios::app);
					for(int fi = 0; fi < div_chan_max; fi++) {
						float f = ( (float)((int)((ubv[fi] * dspace)*1000.0))/1000.0 );
						flsout.write(
							reinterpret_cast<const char*>( &f ),
							sizeof( float ) ); }
					flsout.close();

					std::string params = "\t(\t";
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						std::string pws = (ubi%4  == 3) 							? ",\t\n\t\t" 	: ",\t";
									pws = (ubi%(div_chan_max-1) == 0 && ubi > 0) 	? "" 			: pws;
						params 	= params 
								+ std::to_string( (float)((int)((ubv[ubi] * dspace)*1000.0))/1000.0 ) 
								+ pws;  }
					params = params + "\t);\n";
					std::cout << params;
					std::string save_param 	= "echo \"" + params + "\" >> 'fbk/" + timestamp + ".evo'";
					system(save_param.c_str());
					//std::string save_last 	= "echo \"" + params + "\" >> 'fbk/LastEvo.chk'";
					//system(save_last.c_str()); 
					new_select = 0;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						hst[chk_max][ubi] = (float)((int)((ubv[ubi] * dspace)*1000.0))/1000.0; }
					chk_max++;
					if(do_save) { chk_seek = chk_max; }
				}

				if(xe.xbutton.button == 9) {
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = ubv[ubi] * dspace * 2.0; } }

				if(xe.xbutton.button == 41) {
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = ubv[ubi] * dspace * 2.0; } }

				float do_rand = 0;
				if(xe.xbutton.button == 23) {
					ubo_init_ub[0].mrb = 2.0;
					do_rand = 1; 
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = chk[ubi]; } }

				if(xe.xbutton.button == 24) {
					ubo_init_ub[0].mrb = 2.0;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = chk[ubi]; } }

				if(xe.xbutton.button == 113 && chk_max > 0) {
					chk_seek = (chk_seek+(chk_max-1))%chk_max;
					ubo_init_ub[0].mrb = 2.0;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = hst[chk_seek][ubi] * 2.0; }
						std::cout 	<< "\tSeek: " << chk_seek
									<< " / " 	<< (chk_max-1) << "\n"; }

				if(xe.xbutton.button == 114 && chk_max > 0) {
					chk_seek = (chk_seek+1)%chk_max;
					ubo_init_ub[0].mrb = 2.0;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						ubv[ubi] = hst[chk_seek][ubi] * 2.0; }
						std::cout 	<< "\tSeek: " << chk_seek
									<< " / " 	<< (chk_max-1) << "\n"; }

				if(xe.xbutton.button == 8 || xe.xbutton.button == 26 || do_rand ) {
					float rcm = 1.0;
					if(xe.xbutton.button == 8 || do_rand) { 
						ubo_init_ub[0].mrb = 2.0;
						rcm = 5.0; } 
					else { ubo_init_ub[0].mlb = 2.0; }
					int chan = 0;
					for(int ubi = 0; ubi < div_chan_max; ubi++) {
						chan = rand()%(rand()%10+2);
						if(chan == 0) {
							float 	rchange 	= (((float)(rand()%1001)/1000.0) - 0.5) 
									 			* (0.01* rcm * (ubo_init_ub[0].clicks));
							if(ubv[ubi] + rchange > -2.0 && ubv[ubi] + rchange < 2.0 ) {
									ubv[ubi] 	= ubv[ubi] + rchange; } } } }

				if(xe.xbutton.button == 27 || xe.xbutton.button == 50) { ubo_init_ub[0].mrb = 2.0; }

				if(xe.xbutton.button == 25) { 
					div_chan = (div_chan + 1) % div_chan_max;
					std::cout 	<< "Chan: " << div_chan
								<< " : " 	<< ubv[div_chan] << "\n"; }
				if(xe.xbutton.button == 39) { 
					div_chan = (div_chan + (div_chan_max - 1)) % div_chan_max;
					std::cout 	<< "Chan: " << div_chan
								<< " : " 	<< ubv[div_chan] << "\n"; }

				float chg = 0.001 * (ubo_init_ub[0].clicks);
				if(xe.xbutton.button == 38) {
					if(ubv[div_chan]-chg >= -3.5 && ubv[div_chan]-chg <= 3.5) {
						ubv[div_chan] = ubv[div_chan] - chg;
						std::cout 	<< "Chan: " << div_chan
									<< " : " 	<< ubv[div_chan] << "\n"; } }
				if(xe.xbutton.button == 40) {
					if(ubv[div_chan]+chg >= -3.5 && ubv[div_chan]+chg <= 3.5) {
						ubv[div_chan] = ubv[div_chan] + chg;
						std::cout 	<< "Chan: " << div_chan
									<< " : " 	<< ubv[div_chan] << "\n"; } }

				ubo_init_ub[0].v0 	= ubv[0];	ubo_init_ub[0].v1 	= ubv[1];
				ubo_init_ub[0].v2 	= ubv[2];	ubo_init_ub[0].v3 	= ubv[3];
				ubo_init_ub[0].v4 	= ubv[4];	ubo_init_ub[0].v5 	= ubv[5];
				ubo_init_ub[0].v6 	= ubv[6];	ubo_init_ub[0].v7 	= ubv[7];
				ubo_init_ub[0].v8 	= ubv[8];	ubo_init_ub[0].v9 	= ubv[9];
				ubo_init_ub[0].v10 	= ubv[10];	ubo_init_ub[0].v11 	= ubv[11];
				ubo_init_ub[0].v12 	= ubv[12];	ubo_init_ub[0].v13 	= ubv[13];
				ubo_init_ub[0].v14 	= ubv[14];	ubo_init_ub[0].v15 	= ubv[15];
				ubo_init_ub[0].v16 	= ubv[16];	ubo_init_ub[0].v17 	= ubv[17];
				ubo_init_ub[0].v18 	= ubv[18];	ubo_init_ub[0].v19 	= ubv[19];
				ubo_init_ub[0].v20 	= ubv[20];	ubo_init_ub[0].v21 	= ubv[21];
				ubo_init_ub[0].v22 	= ubv[22];	ubo_init_ub[0].v23 	= ubv[23];
				ubo_init_ub[0].v24 	= ubv[24];	ubo_init_ub[0].v25 	= ubv[25];
				ubo_init_ub[0].v26 	= ubv[26];	ubo_init_ub[0].v27 	= ubv[27];
				ubo_init_ub[0].v28 	= ubv[28];	ubo_init_ub[0].v29 	= ubv[29];
				ubo_init_ub[0].v30 	= ubv[30];	ubo_init_ub[0].v31 	= ubv[31];

				if(xe.xbutton.button <= 9 || 1) {
					std::cout
						<< " evt:" 		<< xe.xbutton.button
						<< ", x:" 		<< xe.xbutton.x
						<< ", y:" 		<< xe.xbutton.y
						<< ", cs:" 		<< ubo_init_ub[0].clicks
						<< ", divi:" 	<< divi
						<< ", dspace:" 	<< dspace
						<< ", chan: " 	<< div_chan
						<< "\n";
				}

				if(xe.xbutton.button == 22) { quit_now = 1; }
				if(xe.xbutton.button == 104) { rec_on = (rec_on) ? 0 : 1; }
				

			}
		}

		if (!pause) {
			if(valid) {
					ov("i", idx);
					ubo_init_ub[0].frame = float(idx+1)*1.0;
				rv("memcpy");
					memcpy(pvoid_memmap, &ubo_init_ub[0], sizeof(ubo_init_ub[0]));

				rv("nanosleep(NS_DELAY)");
					nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

				vr("vkResetFences", &vkres, 
					vkResetFences(vkld[0], 1, vkfence_aqimg) );

				vr("vkCreateSemaphore", &vkres, 
					vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_image[0]) );

				if(valid) {
					vr("vkAcquireNextImageKHR", &vkres, 
						vkAcquireNextImageKHR(vkld[0], vkswap[0], UINT64_MAX, vksemaph_image[0], 
							VK_NULL_HANDLE, &aqimg_idx[0]) );
						iv("aqimg_idx", aqimg_idx[0], idx);
				
					vr("vkCreateSemaphore", &vkres, 
						vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_rendr[0]) );

					VkSubmitInfo vksub_info[1];
						vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
						vksub_info[0].pNext	= NULL;
						vksub_info[0].waitSemaphoreCount	= 1;
						vksub_info[0].pWaitSemaphores		= vksemaph_image;
						vksub_info[0].pWaitDstStageMask		= &qsubwait;
						vksub_info[0].commandBufferCount	= 1;
						vksub_info[0].pCommandBuffers		= &vkcombuf_loop[aqimg_idx[0]];
						vksub_info[0].signalSemaphoreCount	= 1;
						vksub_info[0].pSignalSemaphores		= vksemaph_rendr;

					if(valid) {
						vr("vkQueueSubmit", &vkres, 
							vkQueueSubmit(vkq[0], 1, vksub_info, vkfence_aqimg[0]) );
						
						int res_idx = vkres.size();
						do {
							vr("vkWaitForFences <100ms>", &vkres, 
								vkWaitForFences(vkld[0], 1, vkfence_aqimg, VK_TRUE, 100000000) );
								res_idx = vkres.size()-1;
						} while (vkres[res_idx] == VK_TIMEOUT);

						VkPresentInfoKHR vkpresent_info[1];
							vkpresent_info[0].sType 	= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
							vkpresent_info[0].pNext 	= NULL;
							vkpresent_info[0].waitSemaphoreCount	= 1;
							vkpresent_info[0].pWaitSemaphores 		= vksemaph_rendr;
							vkpresent_info[0].swapchainCount 		= 1;
							vkpresent_info[0].pSwapchains 			= vkswap;
							vkpresent_info[0].pImageIndices 		= aqimg_idx;
							vkpresent_info[0].pResults 				= NULL;

						ov("vkswap_img", vkswap_img[aqimg_idx[0]]);

						if(valid) {
							vr("vkQueuePresentKHR", &vkres, 
								vkQueuePresentKHR(vkq[0], &vkpresent_info[0]) );
						}
					}
				}
			}


			bool do_scr = 0;
			if(		idx == 1
				|| 	idx == 2
				|| 	idx == 5
				|| 	idx == 10
				|| 	idx == 25
				|| 	idx == 75
				|| 	idx == 125
				|| 	idx == 250
				|| 	idx == 500
				||  idx == 1000
				||  (idx%2500 == 0 && idx > 0) ) {
					do_scr = 1; } else { do_scr = 0; }

			int SCR_pad_len = SCR_PAD * SCR_PAD_MULT;
			int SCR_ramp 	= 1;

			if(SCR_RECORD > 0 && idx < SCR_RECORD * SCR_RECORD * SCR_RAMP_ACCEL) {
				SCR_ramp = (
					( (idx - SCR_pad_len > 0) ? (idx-SCR_pad_len) : 0 ) / (SCR_RECORD * SCR_RAMP_ACCEL)
				) + 1;
			} else { SCR_ramp = SCR_RECORD; }

			if( 	SCR_RECORD 	> 0 
				&& 	idx 		> TEST_CYCLES - ((SCR_RECORD * SCR_RECORD * SCR_RAMP_ACCEL)+SCR_pad_len) 
			) { 	SCR_ramp	=(TEST_CYCLES - (idx+SCR_pad_len)) / (SCR_RECORD * SCR_RAMP_ACCEL) + 1; }

			SCR_ramp = (SCR_ramp < 1) ? 1 : SCR_ramp;
			

			if(SCR_RECORD > 0 && valid && idx%SCR_ramp == 0 || do_scr && valid || rec_on && valid && idx%rec_frames == 0 ) {

				if(loglevel != 0) { loglevel = loglevel * -1; }
				if(!rec_on) { ov("Frameskip", SCR_ramp); } else { ov("Frameskip", rec_frames); }
				if(loglevel != 0) { loglevel = loglevel * -1; }

				if(idx%2 == 1) {
					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_L2SCR");	/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_l2SCR[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}

					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_SCR");		/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_SCR[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}

					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_SCR2L");	/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_SCR2l[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}
				}

				if(idx%2 == 0) {
					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_L2SC1");	/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_l2SCR1[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}

					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_SC1");		/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_SCR1[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}

					  ///////////////////////////////////////
					 /**/	hd("STAGE:", "DO_SC12L");	/**/
					///////////////////////////////////////

					for(int i = 0; i < 1; i++) {
						if(valid) {
							rv("nanosleep(NS_DELAY)");
								nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

							VkSubmitInfo vksub_info[1];
								vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
								vksub_info[0].pNext	= NULL;
								vksub_info[0].waitSemaphoreCount	= 0;
								vksub_info[0].pWaitSemaphores		= NULL;
								vksub_info[0].pWaitDstStageMask		= NULL;
								vksub_info[0].commandBufferCount	= 1;
								vksub_info[0].pCommandBuffers		= &vkcombuf_SCR2l1[i];
								vksub_info[0].signalSemaphoreCount	= 0;
								vksub_info[0].pSignalSemaphores		= NULL;

							vr("vkQueueSubmit", &vkres, 
								vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
						}
					}
				}

				  ///////////////////////////////////////
				 /**/	hd("STAGE:", "SCRSHOT");	/**/
				///////////////////////////////////////

				if(SCR_RECORD > 0 && valid && idx%SCR_ramp == 0 || do_scr && valid || rec_on && valid && idx%rec_frames == 0 ) {
					int padframes = 0;

	/*
					if(SCR_RECORD > 0 && valid && idx%SCR_ramp == 0) {
						padframes = (idx > SCR_PAD * SCR_PAD_MULT) ? 1 : ((SCR_PAD+1)-(idx/SCR_PAD_MULT));
					}
	*/
					if(SCR_RECORD > 0 && valid && idx%SCR_ramp == 0) {
						padframes	=	(int)(floor( (
											floor(
												( (long)SCR_PAD / (long)( SCR_PAD + SCR_count ) ) * (long)SCR_PAD 
											) + ( (long)SCR_PAD - (long)( (long)SCR_count / (long)SCR_PAD ) )
										) / (long)2 )); }

					int pfds = (padframes+do_scr > 0) ? padframes+do_scr : 1;
						pfds = (idx==1) ? pfds+((pfds*4)/5) : pfds;
						pfds = (idx==2) ? pfds+((pfds*3)/5) : pfds;
						pfds = (idx==3) ? pfds+((pfds*2)/5) : pfds;
						pfds = (idx==4) ? pfds+((pfds*1)/5) : pfds;

					if(rec_on) { 
						pfds = 1; 
						if(SCR_count%180 == 0 && SCR_count > 0) { pause = 1; } }

					for(int pad = 0; pad < pfds; pad++) {

						std::string 	
							ppm_header 	= (do_scr) ? "P6" : PPM_ENC;
						std::string 	
							ppm_depth 	= (ppm_header == "P4") ? "1" : "255";
						std::string 	
							ppm_extn 	= (ppm_header == "P6") ? "PPM" : (ppm_header == "P5") ? "PGM" : "PBM";
						std::string 	
							ppmfile 	= "out/PPM" 
										+ std::to_string(SCR_count) 
										+ "." + ppm_extn;
						if(do_scr) {	
							ppmfile 	= "scr/SCR_" 
										+ timestamp
										+  "_" 
										+ std::to_string(idx)
										+ "." + ppm_extn; 
						}

						if(loglevel != 0) { loglevel = loglevel * -1; }
						ov("Open File", ppmfile);
						if(loglevel != 0) { loglevel = loglevel * -1; }

						if(ppm_header == "P6") {
							std::ofstream file(ppmfile.c_str(), std::ios::out | std::ios::binary);
								file 	<<	ppm_header 								<< "\n"
									 	<< 	vksurf_ables[0].currentExtent.width		<< " "
								 		<< 	vksurf_ables[0].currentExtent.height	<< " "
										<< 	ppm_depth << "\n";
								for(int i = 0; i < vksurf_ables[0].currentExtent.height; i++) {
									for(int j = 0; j < vksurf_ables[0].currentExtent.width; j++) {
										for(int k = 2; k >= 0; k--) {
											file.write(
												(const char*)SCR_data
												+ ((j*4) + k) 
												+  (i*4*vksurf_ables[0].currentExtent.width)
												, 1 ); } } }
							file.close();
						}

						if(ppm_header == "P5") {
							std::ofstream file(ppmfile.c_str(), std::ios::out | std::ios::binary);
								file 	<<	ppm_header 								<< "\n"
									 	<< 	vksurf_ables[0].currentExtent.width		<< " "
								 		<< 	vksurf_ables[0].currentExtent.height	<< " "
										<< 	ppm_depth << "\n";
								for(int i = 0; i < vksurf_ables[0].currentExtent.height; i++) {
									for(int j = 0; j < vksurf_ables[0].currentExtent.width; j++) {
										file.write(
											(const char*)SCR_data
											+ ((j*4) + (2-PPM_P5C))
											+  (i*4*vksurf_ables[0].currentExtent.width)
											, 1 ); } }
							file.close();
						}

						if(ppm_header == "P4") {
							uint32_t 	copied_data;
							bool 		bits[8];
							int 		bitcount = 0;
							char 		b = 0;
							const char	zb = 0;
							std::ofstream file(ppmfile.c_str(), std::ios::out | std::ios::binary);
								file 	<<	ppm_header 								<< "\n"
									 	<< 	vksurf_ables[0].currentExtent.width		<< " "
								 		<< 	vksurf_ables[0].currentExtent.height	<< " "
										<< 	ppm_depth << "\n";
								for(int i = 0; i < vksurf_ables[0].currentExtent.height; i++) {
									for(int j = 0; j < vksurf_ables[0].currentExtent.width; j++) {
										if(bitcount%8 == 0 && bitcount > 0) {
											b = ToByte(bits);
											file.write(&b, 1);
											bitcount = 0;
										}
										memcpy(
											&copied_data, 
											SCR_data + ((j*4) + 2) + (i*4*vksurf_ables[0].currentExtent.width) + 4*16,
											1
										);
										bits[7-(bitcount%8)] = (!copied_data) ? 1 : 0;
										bitcount++;
									}
								}
							file.close();
						}

						ov("Close File", ppmfile);

						if(!do_scr) {
							SCR_count++;
						} else {
							do_scr = 0;
							std::string scr_convert = "convert '" + ppmfile + "' '" + ppmfile + ".png'";
							system(scr_convert.c_str());
							std::string scr_remove 	= "rm '" + ppmfile + "'";
							system(scr_remove.c_str());
						}

					}
				}

			} else { ov("SCR_VALID", 0); }

			idx++;
		} else {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY*30}}, NULL); }
	} while (valid && (idx < ilimit || ilimit < 1) && !quit_now);

	if(loglevel != 0) { loglevel = loglevel * -1; }

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CLIFILE");	/**/
	///////////////////////////////////////

	if(SCR_RECORD > 0 && valid || SCR_count > 0 && quit_now && valid) {
		std::string 	
			ppm_header 	= PPM_ENC;
		std::string 	
			ppm_depth 	= (ppm_header == "P4") ? "1" : "255";
		std::string 	
			ppm_extn 	= (ppm_header == "P6") ? "PPM" : (ppm_header == "P5") ? "PGM" : "PBM";

		rv("SystemCLI: Convert");
		for(int i = 0; i < SCR_count; i++) {
			std::string ppmfile = "out/PPM" + std::to_string(i) + "." + ppm_extn;
			std::string pngfile = "out/IMG" + std::to_string(i) + ".png";
//			std::string cli_cmd_convert = "convert '" + ppmfile + "' '" + pngfile + "'";
			std::string cli_cmd_convert = "pnmtopng '" + ppmfile + "' > '" + pngfile + "' -quiet";
			if(i%((SCR_count/25)+1) == 1 || i == 0 || i == SCR_count-1) {
				std::string pcnt = std::to_string(i+1) + " / " + std::to_string(SCR_count);
				ov("SystemCLI: Converting", pcnt);
			}
			system(cli_cmd_convert.c_str());
		}
		rv("Done!");

		rv("SystemCLI: Remove PPM");
		for(int i = 0; i < SCR_count; i++) {
			std::string ppmfile = "out/PPM" + std::to_string(i) + "." + ppm_extn;
			std::string cli_cmd_remove 	= "rm '" + ppmfile + "'";
			system(cli_cmd_remove.c_str());
		}
		rv("Done!");
			
		rv("SystemCLI: Thumbnail");
			int thumb = (SCR_count / 20) + 150;
				thumb = (thumb < SCR_count) ? thumb : SCR_count / 2;
			std::string pngthumb = "out/IMG" + std::to_string(thumb) + ".png";
			std::string pngthumbzero = "out/IMG" + std::to_string(0) + ".png";
			std::string cli_cmd_thumb 	= "cp '" + pngthumb + "' '" + pngthumbzero + "'";
			system(cli_cmd_thumb.c_str());
		rv("Done!");

		rv("SystemCLI: png2vid.sh");
		system("./png2vid.sh");
		rv("Done!");
		
		rv("SystemCLI: Remove png");
		for(int i = 0; i < SCR_count; i++) {
			std::string pngfile = "out/IMG" + std::to_string(i) + ".png";
			std::string cli_cmd_remove 	= "rm '" + pngfile + "'";
			system(cli_cmd_remove.c_str());
		}
		rv("Done!");
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "ENDPROG");	/**/
	///////////////////////////////////////

	if(!valid) {
		hd("STAGE:", "ABORTED");
	}

	rv("XCloseDisplay");
		XCloseDisplay(d);

	rv("return");
		return 0;

}



