
#include "quakedef.h"

// global video state
viddef_t vid;

// LordHavoc: these are only set in wgl
qboolean isG200 = false; // LordHavoc: the Matrox G200 can't do per pixel alpha, and it uses a D3D driver for GL... ugh...
qboolean isRagePro = false; // LordHavoc: the ATI Rage Pro has limitations with per pixel alpha (the color scaler does not apply to per pixel alpha images...), although not as bad as a G200.

// GL_ARB_multitexture
int gl_textureunits = 0;
// GL_ARB_texture_env_combine or GL_EXT_texture_env_combine
int gl_combine_extension = false;
// GL_EXT_compiled_vertex_array
int gl_supportslockarrays = false;
// GLX_SGI_video_sync or WGL_EXT_swap_control
int gl_videosyncavailable = false;
// stencil available
int gl_stencil = false;
// 3D textures available
int gl_texture3d = false;
// GL_ARB_texture_cubemap
int gl_texturecubemap = false;
// GL_ARB_texture_env_dot3
int gl_dot3arb = false;

// LordHavoc: if window is hidden, don't update screen
int vid_hidden = true;
// LordHavoc: if window is not the active window, don't hog as much CPU time,
// let go of the mouse, turn off sound, and restore system gamma ramps...
int vid_activewindow = true;

cvar_t vid_fullscreen = {CVAR_SAVE, "vid_fullscreen", "1"};
cvar_t vid_width = {CVAR_SAVE, "vid_width", "640"};
cvar_t vid_height = {CVAR_SAVE, "vid_height", "480"};
cvar_t vid_bitsperpixel = {CVAR_SAVE, "vid_bitsperpixel", "16"};
cvar_t vid_stencil = {CVAR_SAVE, "vid_stencil", "0"};

cvar_t vid_mouse = {CVAR_SAVE, "vid_mouse", "1"};
cvar_t gl_combine = {0, "gl_combine", "1"};

cvar_t in_pitch_min = {0, "in_pitch_min", "-90"};
cvar_t in_pitch_max = {0, "in_pitch_max", "90"};

cvar_t m_filter = {CVAR_SAVE, "m_filter","0"};

// brand of graphics chip
const char *gl_vendor;
// graphics chip model and other information
const char *gl_renderer;
// begins with 1.0.0, 1.1.0, 1.2.0, 1.2.1, 1.3.0, 1.3.1, or 1.4.0
const char *gl_version;
// extensions list, space separated
const char *gl_extensions;
// WGL, GLX, or AGL
const char *gl_platform;
// another extensions list, containing platform-specific extensions that are
// not in the main list
const char *gl_platformextensions;
// name of driver library (opengl32.dll, libGL.so.1, or whatever)
char gl_driver[256];

// GL_ARB_multitexture
void (GLAPIENTRY *qglMultiTexCoord2f) (GLenum, GLfloat, GLfloat);
void (GLAPIENTRY *qglActiveTexture) (GLenum);
void (GLAPIENTRY *qglClientActiveTexture) (GLenum);

// GL_EXT_compiled_vertex_array
void (GLAPIENTRY *qglLockArraysEXT) (GLint first, GLint count);
void (GLAPIENTRY *qglUnlockArraysEXT) (void);


// general GL functions

void (GLAPIENTRY *qglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

void (GLAPIENTRY *qglClear)(GLbitfield mask);

//void (GLAPIENTRY *qglAlphaFunc)(GLenum func, GLclampf ref);
void (GLAPIENTRY *qglBlendFunc)(GLenum sfactor, GLenum dfactor);
void (GLAPIENTRY *qglCullFace)(GLenum mode);

void (GLAPIENTRY *qglDrawBuffer)(GLenum mode);
void (GLAPIENTRY *qglReadBuffer)(GLenum mode);
void (GLAPIENTRY *qglEnable)(GLenum cap);
void (GLAPIENTRY *qglDisable)(GLenum cap);
//GLboolean GLAPIENTRY *qglIsEnabled)(GLenum cap);

void (GLAPIENTRY *qglEnableClientState)(GLenum cap);
void (GLAPIENTRY *qglDisableClientState)(GLenum cap);

void (GLAPIENTRY *qglGetBooleanv)(GLenum pname, GLboolean *params);
void (GLAPIENTRY *qglGetDoublev)(GLenum pname, GLdouble *params);
void (GLAPIENTRY *qglGetFloatv)(GLenum pname, GLfloat *params);
void (GLAPIENTRY *qglGetIntegerv)(GLenum pname, GLint *params);

GLenum (GLAPIENTRY *qglGetError)(void);
const GLubyte* (GLAPIENTRY *qglGetString)(GLenum name);
void (GLAPIENTRY *qglFinish)(void);
void (GLAPIENTRY *qglFlush)(void);

void (GLAPIENTRY *qglClearDepth)(GLclampd depth);
void (GLAPIENTRY *qglDepthFunc)(GLenum func);
void (GLAPIENTRY *qglDepthMask)(GLboolean flag);
void (GLAPIENTRY *qglDepthRange)(GLclampd near_val, GLclampd far_val);
void (GLAPIENTRY *qglColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

void (GLAPIENTRY *qglDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
void (GLAPIENTRY *qglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void (GLAPIENTRY *qglVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
//void (GLAPIENTRY *qglNormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr);
void (GLAPIENTRY *qglColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void (GLAPIENTRY *qglTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void (GLAPIENTRY *qglArrayElement)(GLint i);

void (GLAPIENTRY *qglColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void (GLAPIENTRY *qglTexCoord2f)(GLfloat s, GLfloat t);
void (GLAPIENTRY *qglVertex2f)(GLfloat x, GLfloat y);
void (GLAPIENTRY *qglVertex3f)(GLfloat x, GLfloat y, GLfloat z);
void (GLAPIENTRY *qglBegin)(GLenum mode);
void (GLAPIENTRY *qglEnd)(void);

void (GLAPIENTRY *qglMatrixMode)(GLenum mode);
void (GLAPIENTRY *qglOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
void (GLAPIENTRY *qglFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
void (GLAPIENTRY *qglViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
//void (GLAPIENTRY *qglPushMatrix)(void);
//void (GLAPIENTRY *qglPopMatrix)(void);
void (GLAPIENTRY *qglLoadIdentity)(void);
void (GLAPIENTRY *qglLoadMatrixd)(const GLdouble *m);
void (GLAPIENTRY *qglLoadMatrixf)(const GLfloat *m);
//void (GLAPIENTRY *qglMultMatrixd)(const GLdouble *m);
//void (GLAPIENTRY *qglMultMatrixf)(const GLfloat *m);
//void (GLAPIENTRY *qglRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void (GLAPIENTRY *qglRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
//void (GLAPIENTRY *qglScaled)(GLdouble x, GLdouble y, GLdouble z);
//void (GLAPIENTRY *qglScalef)(GLfloat x, GLfloat y, GLfloat z);
//void (GLAPIENTRY *qglTranslated)(GLdouble x, GLdouble y, GLdouble z);
void (GLAPIENTRY *qglTranslatef)(GLfloat x, GLfloat y, GLfloat z);

void (GLAPIENTRY *qglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);

void (GLAPIENTRY *qglStencilFunc)(GLenum func, GLint ref, GLuint mask);
void (GLAPIENTRY *qglStencilMask)(GLuint mask);
void (GLAPIENTRY *qglStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
void (GLAPIENTRY *qglClearStencil)(GLint s);

//void (GLAPIENTRY *qglTexEnvf)(GLenum target, GLenum pname, GLfloat param);
void (GLAPIENTRY *qglTexEnvi)(GLenum target, GLenum pname, GLint param);
//void (GLAPIENTRY *qglTexParameterf)(GLenum target, GLenum pname, GLfloat param);
void (GLAPIENTRY *qglTexParameteri)(GLenum target, GLenum pname, GLint param);

void (GLAPIENTRY *qglGenTextures)(GLsizei n, GLuint *textures);
void (GLAPIENTRY *qglDeleteTextures)(GLsizei n, const GLuint *textures);
void (GLAPIENTRY *qglBindTexture)(GLenum target, GLuint texture);
void (GLAPIENTRY *qglPrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
GLboolean (GLAPIENTRY *qglAreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
GLboolean (GLAPIENTRY *qglIsTexture)(GLuint texture);
void (GLAPIENTRY *qglPixelStoref)(GLenum pname, GLfloat param);
void (GLAPIENTRY *qglPixelStorei)(GLenum pname, GLint param);

void (GLAPIENTRY *qglTexImage1D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglTexImage2D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglCopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void (GLAPIENTRY *qglCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void (GLAPIENTRY *qglCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void (GLAPIENTRY *qglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);


void (GLAPIENTRY *qglDrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

//void (GLAPIENTRY *qglColorTableEXT)(int, int, int, int, int, const void *);

void (GLAPIENTRY *qglTexImage3D)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY *qglCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);


int GL_CheckExtension(const char *name, const gl_extensionfunctionlist_t *funcs, const char *disableparm, int silent)
{
	int failed = false;
	const gl_extensionfunctionlist_t *func;

	Con_Printf("checking for %s...  ", name);

	for (func = funcs;func && func->name;func++)
		*func->funcvariable = NULL;

	if (disableparm && COM_CheckParm(disableparm))
	{
		Con_Printf("disabled by commandline\n");
		return false;
	}

	if (strstr(gl_extensions, name) || strstr(gl_platformextensions, name) || (strncmp(name, "GL_", 3) && strncmp(name, "WGL_", 4) && strncmp(name, "GLX_", 4) && strncmp(name, "AGL_", 4)))
	{
		for (func = funcs;func && func->name != NULL;func++)
		{
			// functions are cleared before all the extensions are evaluated
			if (!(*func->funcvariable = (void *) GL_GetProcAddress(func->name)))
			{
				if (!silent)
					Con_Printf("missing function \"%s\" - broken driver!\n", func->name);
				failed = true;
			}
		}
		// delay the return so it prints all missing functions
		if (failed)
			return false;
		Con_Printf("enabled\n");
		return true;
	}
	else
	{
		Con_Printf("not detected\n");
		return false;
	}
}

static gl_extensionfunctionlist_t opengl110funcs[] =
{
	{"glClearColor", (void **) &qglClearColor},
	{"glClear", (void **) &qglClear},
//	{"glAlphaFunc", (void **) &qglAlphaFunc},
	{"glBlendFunc", (void **) &qglBlendFunc},
	{"glCullFace", (void **) &qglCullFace},
	{"glDrawBuffer", (void **) &qglDrawBuffer},
	{"glReadBuffer", (void **) &qglReadBuffer},
	{"glEnable", (void **) &qglEnable},
	{"glDisable", (void **) &qglDisable},
//	{"glIsEnabled", (void **) &qglIsEnabled},
	{"glEnableClientState", (void **) &qglEnableClientState},
	{"glDisableClientState", (void **) &qglDisableClientState},
	{"glGetBooleanv", (void **) &qglGetBooleanv},
	{"glGetDoublev", (void **) &qglGetDoublev},
	{"glGetFloatv", (void **) &qglGetFloatv},
	{"glGetIntegerv", (void **) &qglGetIntegerv},
	{"glGetError", (void **) &qglGetError},
	{"glGetString", (void **) &qglGetString},
	{"glFinish", (void **) &qglFinish},
	{"glFlush", (void **) &qglFlush},
	{"glClearDepth", (void **) &qglClearDepth},
	{"glDepthFunc", (void **) &qglDepthFunc},
	{"glDepthMask", (void **) &qglDepthMask},
	{"glDepthRange", (void **) &qglDepthRange},
	{"glDrawElements", (void **) &qglDrawElements},
	{"glColorMask", (void **) &qglColorMask},
	{"glVertexPointer", (void **) &qglVertexPointer},
//	{"glNormalPointer", (void **) &qglNormalPointer},
	{"glColorPointer", (void **) &qglColorPointer},
	{"glTexCoordPointer", (void **) &qglTexCoordPointer},
	{"glArrayElement", (void **) &qglArrayElement},
	{"glColor4f", (void **) &qglColor4f},
	{"glTexCoord2f", (void **) &qglTexCoord2f},
	{"glVertex2f", (void **) &qglVertex2f},
	{"glVertex3f", (void **) &qglVertex3f},
	{"glBegin", (void **) &qglBegin},
	{"glEnd", (void **) &qglEnd},
	{"glMatrixMode", (void **) &qglMatrixMode},
	{"glOrtho", (void **) &qglOrtho},
	{"glFrustum", (void **) &qglFrustum},
	{"glViewport", (void **) &qglViewport},
//	{"glPushMatrix", (void **) &qglPushMatrix},
//	{"glPopMatrix", (void **) &qglPopMatrix},
	{"glLoadIdentity", (void **) &qglLoadIdentity},
	{"glLoadMatrixd", (void **) &qglLoadMatrixd},
	{"glLoadMatrixf", (void **) &qglLoadMatrixf},
//	{"glMultMatrixd", (void **) &qglMultMatrixd},
//	{"glMultMatrixf", (void **) &qglMultMatrixf},
//	{"glRotated", (void **) &qglRotated},
	{"glRotatef", (void **) &qglRotatef},
//	{"glScaled", (void **) &qglScaled},
//	{"glScalef", (void **) &qglScalef},
//	{"glTranslated", (void **) &qglTranslated},
	{"glTranslatef", (void **) &qglTranslatef},
	{"glReadPixels", (void **) &qglReadPixels},
	{"glStencilFunc", (void **) &qglStencilFunc},
	{"glStencilMask", (void **) &qglStencilMask},
	{"glStencilOp", (void **) &qglStencilOp},
	{"glClearStencil", (void **) &qglClearStencil},
//	{"glTexEnvf", (void **) &qglTexEnvf},
	{"glTexEnvi", (void **) &qglTexEnvi},
//	{"glTexParameterf", (void **) &qglTexParameterf},
	{"glTexParameteri", (void **) &qglTexParameteri},
	{"glPixelStoref", (void **) &qglPixelStoref},
	{"glPixelStorei", (void **) &qglPixelStorei},
	{"glGenTextures", (void **) &qglGenTextures},
	{"glDeleteTextures", (void **) &qglDeleteTextures},
	{"glBindTexture", (void **) &qglBindTexture},
	{"glPrioritizeTextures", (void **) &qglPrioritizeTextures},
	{"glAreTexturesResident", (void **) &qglAreTexturesResident},
	{"glIsTexture", (void **) &qglIsTexture},
	{"glTexImage1D", (void **) &qglTexImage1D},
	{"glTexImage2D", (void **) &qglTexImage2D},
	{"glTexSubImage1D", (void **) &qglTexSubImage1D},
	{"glTexSubImage2D", (void **) &qglTexSubImage2D},
	{"glCopyTexImage1D", (void **) &qglCopyTexImage1D},
	{"glCopyTexImage2D", (void **) &qglCopyTexImage2D},
	{"glCopyTexSubImage1D", (void **) &qglCopyTexSubImage1D},
	{"glCopyTexSubImage2D", (void **) &qglCopyTexSubImage2D},
	{NULL, NULL}
};

static gl_extensionfunctionlist_t drawrangeelementsfuncs[] =
{
	{"glDrawRangeElements", (void **) &qglDrawRangeElements},
	{NULL, NULL}
};

static gl_extensionfunctionlist_t drawrangeelementsextfuncs[] =
{
	{"glDrawRangeElementsEXT", (void **) &qglDrawRangeElementsEXT},
	{NULL, NULL}
};

static gl_extensionfunctionlist_t multitexturefuncs[] =
{
	{"glMultiTexCoord2fARB", (void **) &qglMultiTexCoord2f},
	{"glActiveTextureARB", (void **) &qglActiveTexture},
	{"glClientActiveTextureARB", (void **) &qglClientActiveTexture},
	{NULL, NULL}
};

static gl_extensionfunctionlist_t compiledvertexarrayfuncs[] =
{
	{"glLockArraysEXT", (void **) &qglLockArraysEXT},
	{"glUnlockArraysEXT", (void **) &qglUnlockArraysEXT},
	{NULL, NULL}
};

static gl_extensionfunctionlist_t texture3dfuncs[] =
{
	{"glTexImage3DEXT", (void **) &qglTexImage3D},
	{"glTexSubImage3DEXT", (void **) &qglTexSubImage3D},
	{"glCopyTexSubImage3DEXT", (void **) &qglCopyTexSubImage3D},
	{NULL, NULL}
};

void VID_CheckExtensions(void)
{
	gl_stencil = vid_stencil.integer;
	gl_combine_extension = false;
	gl_dot3arb = false;
	gl_supportslockarrays = false;
	gl_textureunits = 1;

	if (!GL_CheckExtension("OpenGL 1.1.0", opengl110funcs, NULL, false))
		Sys_Error("OpenGL 1.1.0 functions not found\n");

	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);
	Con_Printf ("%s_EXTENSIONS: %s\n", gl_platform, gl_platformextensions);

	Con_Printf("Checking OpenGL extensions...\n");

	if (!GL_CheckExtension("glDrawRangeElements", drawrangeelementsfuncs, "-nodrawrangeelements", true))
		GL_CheckExtension("GL_EXT_draw_range_elements", drawrangeelementsextfuncs, "-nodrawrangeelements", false);

	if (GL_CheckExtension("GL_ARB_multitexture", multitexturefuncs, "-nomtex", false))
	{
		qglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gl_textureunits);
		gl_combine_extension = GL_CheckExtension("GL_ARB_texture_env_combine", NULL, "-nocombine", false) || GL_CheckExtension("GL_EXT_texture_env_combine", NULL, "-nocombine", false);
		if (gl_combine_extension)
			gl_dot3arb = GL_CheckExtension("GL_ARB_texture_env_dot3", NULL, "-nodot3", false);
	}

	gl_texture3d = GL_CheckExtension("GL_EXT_texture3D", texture3dfuncs, "-notexture3d", false);
	gl_texturecubemap = GL_CheckExtension("GL_ARB_texture_cube_map", NULL, "-nocubemap", false);
	gl_supportslockarrays = GL_CheckExtension("GL_EXT_compiled_vertex_array", compiledvertexarrayfuncs, "-nocva", false);

	// we don't care if it's an extension or not, they are identical functions, so keep it simple in the rendering code
	if (qglDrawRangeElements == NULL)
		qglDrawRangeElements = qglDrawRangeElementsEXT;
}

void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}

void IN_PreMove(void)
{
}

void CL_AdjustAngles(void);
void IN_PostMove(void)
{
	// clamp after the move as well to prevent messed up rendering angles
	CL_AdjustAngles();
}

void IN_Mouse(usercmd_t *cmd, float mx, float my)
{
	int mouselook = (in_mlook.state & 1) || freelook.integer;
	float mouse_x, mouse_y;
	static float old_mouse_x = 0, old_mouse_y = 0;

	if (m_filter.integer)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	// LordHavoc: viewzoom affects mouse sensitivity for sniping
	mouse_x *= sensitivity.value * cl.viewzoom;
	mouse_y *= sensitivity.value * cl.viewzoom;

	// Add mouse X/Y movement to cmd
	if ((in_strafe.state & 1) || (lookstrafe.integer && mouselook))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (mouselook)
		V_StopPitchDrift();

	if (mouselook && !(in_strafe.state & 1))
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

void VID_Shared_Init(void)
{
	Cvar_RegisterVariable(&vid_fullscreen);
	Cvar_RegisterVariable(&vid_width);
	Cvar_RegisterVariable(&vid_height);
	Cvar_RegisterVariable(&vid_bitsperpixel);
	Cvar_RegisterVariable(&vid_stencil);
	Cvar_RegisterVariable(&vid_mouse);
	Cvar_RegisterVariable(&gl_combine);
	Cvar_RegisterVariable(&in_pitch_min);
	Cvar_RegisterVariable(&in_pitch_max);
	Cvar_RegisterVariable(&m_filter);
	Cmd_AddCommand("force_centerview", Force_CenterView_f);
	Cmd_AddCommand("vid_restart", VID_Restart_f);
}

int current_vid_fullscreen;
int current_vid_width;
int current_vid_height;
int current_vid_bitsperpixel;
int current_vid_stencil;
extern int VID_InitMode (int fullscreen, int width, int height, int bpp, int stencil);
int VID_Mode(int fullscreen, int width, int height, int bpp, int stencil)
{
	Con_Printf("Video: %s %dx%dx%d %s\n", fullscreen ? "fullscreen" : "window", width, height, bpp, stencil ? "with stencil" : "without stencil");
	if (VID_InitMode(fullscreen, width, height, bpp, stencil))
	{
		current_vid_fullscreen = fullscreen;
		current_vid_width = width;
		current_vid_height = height;
		current_vid_bitsperpixel = bpp;
		current_vid_stencil = stencil;
		Cvar_SetValueQuick(&vid_fullscreen, fullscreen);
		Cvar_SetValueQuick(&vid_width, width);
		Cvar_SetValueQuick(&vid_height, height);
		Cvar_SetValueQuick(&vid_bitsperpixel, bpp);
		Cvar_SetValueQuick(&vid_stencil, stencil);
		return true;
	}
	else
		return false;
}

static void VID_OpenSystems(void)
{
	R_Modules_Start();
	S_Open();
	CDAudio_Open();
}

static void VID_CloseSystems(void)
{
	CDAudio_Close();
	S_Close();
	R_Modules_Shutdown();
}

void VID_Restart_f(void)
{
	Con_Printf("VID_Restart: changing from %s %dx%dx%dbpp %s, to %s %dx%dx%dbpp %s.\n",
		current_vid_fullscreen ? "fullscreen" : "window", current_vid_width, current_vid_height, current_vid_bitsperpixel, current_vid_stencil ? "with stencil" : "without stencil",
		vid_fullscreen.integer ? "fullscreen" : "window", vid_width.integer, vid_height.integer, vid_bitsperpixel.integer, vid_stencil.integer ? "with stencil" : "without stencil");
	VID_Close();
	if (!VID_Mode(vid_fullscreen.integer, vid_width.integer, vid_height.integer, vid_bitsperpixel.integer, vid_stencil.integer))
	{
		Con_Printf("Video mode change failed\n");
		if (!VID_Mode(current_vid_fullscreen, current_vid_width, current_vid_height, current_vid_bitsperpixel, current_vid_stencil))
			Sys_Error("Unable to restore to last working video mode\n");
	}
	VID_OpenSystems();
}

int vid_commandlinecheck = true;
void VID_Open(void)
{
	int i;
	if (vid_commandlinecheck)
	{
		// interpret command-line parameters
		vid_commandlinecheck = false;
		if ((i = COM_CheckParm("-window")) != 0)
			Cvar_SetValueQuick(&vid_fullscreen, false);
		if ((i = COM_CheckParm("-fullscreen")) != 0)
			Cvar_SetValueQuick(&vid_fullscreen, true);
		if ((i = COM_CheckParm("-width")) != 0)
			Cvar_SetQuick(&vid_width, com_argv[i+1]);
		if ((i = COM_CheckParm("-height")) != 0)
			Cvar_SetQuick(&vid_height, com_argv[i+1]);
		if ((i = COM_CheckParm("-bpp")) != 0)
			Cvar_SetQuick(&vid_bitsperpixel, com_argv[i+1]);
		if ((i = COM_CheckParm("-nostencil")) != 0)
			Cvar_SetValueQuick(&vid_stencil, 0);
		if ((i = COM_CheckParm("-stencil")) != 0)
			Cvar_SetValueQuick(&vid_stencil, 1);
	}

	Con_Printf("Starting video system\n");
	if (!VID_Mode(vid_fullscreen.integer, vid_width.integer, vid_height.integer, vid_bitsperpixel.integer, vid_stencil.integer))
	{
		Con_Printf("Desired video mode fail, trying fallbacks...\n");
		if (!vid_stencil.integer || !VID_Mode(vid_fullscreen.integer, vid_width.integer, vid_height.integer, vid_bitsperpixel.integer, false))
		{
			if (vid_fullscreen.integer)
			{
				if (!VID_Mode(true, 640, 480, 16, false))
					if (!VID_Mode(false, 640, 480, 16, false))
						Sys_Error("Video modes failed\n");
			}
			else
				Sys_Error("Windowed video failed\n");
		}
	}
	VID_OpenSystems();
}

void VID_Close(void)
{
	VID_CloseSystems();
	VID_Shutdown();
}
