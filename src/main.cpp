#define TSF_IMPLEMENTATION
#include "tsf.h"
#include "minisdl_audio.h"

#include <stdio.h>
#include <chrono>
#include "chip8.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GL/gl3w.h"    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>


// From here: https://github.com/schellingb/TinySoundFont/blob/master/examples/example1.c
//This is a minimal SoundFont with a single loopin saw-wave sample/instrument/preset (484 bytes)
const static unsigned char MinimalSoundFont[] =
{
	#define TEN0 0,0,0,0,0,0,0,0,0,0
	'R','I','F','F',220,1,0,0,'s','f','b','k',
	'L','I','S','T',88,1,0,0,'p','d','t','a',
	'p','h','d','r',76,TEN0,TEN0,TEN0,TEN0,0,0,0,0,TEN0,0,0,0,0,0,0,0,255,0,255,0,1,TEN0,0,0,0,
	'p','b','a','g',8,0,0,0,0,0,0,0,1,0,0,0,'p','m','o','d',10,TEN0,0,0,0,'p','g','e','n',8,0,0,0,41,0,0,0,0,0,0,0,
	'i','n','s','t',44,TEN0,TEN0,0,0,0,0,0,0,0,0,TEN0,0,0,0,0,0,0,0,1,0,
	'i','b','a','g',8,0,0,0,0,0,0,0,2,0,0,0,'i','m','o','d',10,TEN0,0,0,0,
	'i','g','e','n',12,0,0,0,54,0,1,0,53,0,0,0,0,0,0,0,
	's','h','d','r',92,TEN0,TEN0,0,0,0,0,0,0,0,50,0,0,0,0,0,0,0,49,0,0,0,34,86,0,0,60,0,0,0,1,TEN0,TEN0,TEN0,TEN0,0,0,0,0,0,0,0,
	'L','I','S','T',112,0,0,0,'s','d','t','a','s','m','p','l',100,0,0,0,86,0,119,3,31,7,147,10,43,14,169,17,58,21,189,24,73,28,204,31,73,35,249,38,46,42,71,46,250,48,150,53,242,55,126,60,151,63,108,66,126,72,207,
		70,86,83,100,72,74,100,163,39,241,163,59,175,59,179,9,179,134,187,6,186,2,194,5,194,15,200,6,202,96,206,159,209,35,213,213,216,45,220,221,223,76,227,221,230,91,234,242,237,105,241,8,245,118,248,32,252
};

// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Callback function called by the audio thread
static void AudioCallback(void* data, Uint8 *stream, int len)
{
	// Note we don't do any thread concurrency control here because in this
	// example all notes are started before the audio playback begins.
	// If you do play notes while the audio thread renders output you
	// will need a mutex of some sort.
	int SampleCount = (len / (2 * sizeof(short))); //2 output channels
	tsf_render_short(g_TinySoundFont, (short*)stream, SampleCount, 0);
}


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLuint CreateTexture() {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return textureID;
}

void TextureFromMat(unsigned char* buffer, int width, int height) {
    //use fast 4-byte alignment (default anyway) if possible
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //set length of one complete row in data (doesn't need to equal im.cols)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer);
}


void inputKeys(Chip8& chip8, ImGuiIO& io, int keyboardKey, int keyId) {
    if (io.KeysDownDuration[keyboardKey] >= 0.0f) {
        chip8.keys[keyId] = 1;
    } else {
        chip8.keys[keyId] = 0;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./chip8 path/to/game/awesomegame\n");
        return 0;
    }

    unsigned char image_buffer[32][64*3];
    Chip8 chip8;
    if (chip8.loadGame(argv[1]) == false)
    {
        printf("Problem loading the provided game: %s\n", argv[1]);
        return 1;
    }
    float im_scale = 10.0;


    // Audio stuff
	// Define the desired audio output format we request
	SDL_AudioSpec OutputAudioSpec;
	OutputAudioSpec.freq = 44100;
	OutputAudioSpec.format = AUDIO_S16;
	OutputAudioSpec.channels = 2;
	OutputAudioSpec.samples = 4096;
	OutputAudioSpec.callback = AudioCallback;

	// Initialize the audio system
	if (SDL_AudioInit(NULL) < 0)
	{
		fprintf(stderr, "Could not initialize audio hardware or driver\n");
		return 1;
	}

	// Load the SoundFont from the memory block
	g_TinySoundFont = tsf_load_memory(MinimalSoundFont, sizeof(MinimalSoundFont));
	if (!g_TinySoundFont)
	{
		fprintf(stderr, "Could not load soundfont\n");
		return 1;
	}

	// Set the rendering output mode to 44.1khz and -10 decibel gain
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, -10);

	// Start two notes before starting the audio playback
	tsf_note_on(g_TinySoundFont, 0, 32, 1.0f);

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, NULL) < 0)
	{
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
		return 1;
	}


    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;


    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Chip8 Emulator", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    GLuint textureID = CreateTexture(); // Just using one texture. Avoiding texture memory leak.

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // we must run this until 1/60 of the cpu cicles have finished, and since
        // the timers are updated on the same frequency I'm using them to keep track  
        auto begin = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> ellapsed_time;
        do {
            chip8.runStep();
            if (chip8.sound_timer > 1) {
                SDL_PauseAudio(0);
            } else {
                SDL_PauseAudio(1);
            }
            ellapsed_time = std::chrono::high_resolution_clock::now()-begin;
        } while ((float)ellapsed_time.count() < 1000000/60);
        if (chip8.display_updated) {
            for (int i=0; i<32; i++) {
                for (int j=0; j<64; j++) {
                    image_buffer[i][j*3+0] = 255*chip8.display[i][j];
                    image_buffer[i][j*3+1] = 255*chip8.display[i][j];
                    image_buffer[i][j*3+2] = 255*chip8.display[i][j];
                }
            }
            TextureFromMat(image_buffer[0], 64, 32); 
            chip8.display_updated = false;
        }
        
        {
            ImGui::Begin("Chip8");//, NULL, ImGuiWindowFlags_MenuBar );   
            ImGui::Image((GLuint*)textureID, ImVec2(64*im_scale,32*im_scale));        
            ImGui::End();
        }


        
        inputKeys(chip8, io, 49, 1);
        inputKeys(chip8, io, 50, 2);
        inputKeys(chip8, io, 51, 3);
        inputKeys(chip8, io, 52, 12);
        
        inputKeys(chip8, io, 81, 4);
        inputKeys(chip8, io, 87, 5);
        inputKeys(chip8, io, 69, 6);
        inputKeys(chip8, io, 82, 13);
        
        inputKeys(chip8, io, 65, 7);
        inputKeys(chip8, io, 83, 8);
        inputKeys(chip8, io, 68, 9);
        inputKeys(chip8, io, 70, 14);
        
        inputKeys(chip8, io, 90, 10);
        inputKeys(chip8, io, 88, 0);
        inputKeys(chip8, io, 67, 11);
        inputKeys(chip8, io, 86, 15);


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
