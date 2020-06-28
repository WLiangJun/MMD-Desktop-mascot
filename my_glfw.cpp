#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>  // 打印glm变量

#define	STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>  // 加载图片的库

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDFile.h>
#include <Saba/Model/MMD/VMDAnimation.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>

#include <math.h>
#include <Saba\GL\GLObject.h>

#include <string.h>
#include <cstdlib>

#include <time.h>
#include <windows.h>//用于函数SetConsoleOutputCP(65001);更改cmd编码为utf8
#include <fstream>

//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )  // 不显示控制台

// cpu运行核心数
int cpuNum = 1;
bool singleModeCpu = true;


// 专门存放失败vmd的路径列表
std::vector<std::string> vmdReadArr;

double PI = 3.1415926535898;
// 设置窗口大小和位置坐标
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 800;
int winXPos = 900;
int winYPos = 200;
// 执行程序路径
std::string exePath;
// model数组和vmd数组
int maxModelIndex = 0;
int maxVmdIndex = 0;
std::string modelArr[50];
std::string vmdArr[10000];

// 鼠标操作灵敏度设置
float Rsensitivity = 0.005f; // 右键拖放
float Lsensitivity = 0.01f; // 左键拖动
float Msensitivity = 10.0f;   // 窗口拖放
float MSsensitivity = 1.0f;   // 中键缩放放

// 设置结束标志  窗口透明标志
float frameMulti = 1.65;
bool transWindow = false;

// 屏幕分辨率
int SC_W = 1920;
int SC_H = 1080;

// 优化CPU占用率等待时间
int sleepTime = 30;

// 影子颜色
float rColor = 0.4f;
float gColor = 0.2f;
float bColor = 0.2f;
float aphaColor = 0.3f;

// 窗口前置
bool windowFloating = true;

// MSAA_NUM  设置多重采样层数
int MSAA_NUM = 1;
bool isMSAA = false;

// CurrentFrameBuffer
int		m_currentFrameBufferWidth;
int		m_currentFrameBufferHeight;
bool	m_currentMSAAEnable;
int		m_currentMSAACount;
saba::GLFramebufferObject		m_currentFrameBuffer;
saba::GLFramebufferObject		m_currentMSAAFrameBuffer;
saba::GLTextureObject			m_currentColorTarget;
saba::GLRenderbufferObject	m_currentMSAAColorTarget;
saba::GLRenderbufferObject	m_currentDepthTarget;
saba::GLFramebufferObject		m_captureFrameBuffer;

// vmd 时间相关变量
float maxAniTime = 0.0f;   // 最大时间
float frameNum = 0;   // 帧数计数
double frameTime = 0.022;  // 单帧时间
double maxTime;   // 最大vmd时间
float frameNumSec = 20.0; // 平均每秒的帧数

// 是否有拖放操作
bool drop = false;
bool dropConfigFile = false;  // 识别是否拖放的是配置文件

// 当前pmx，vmd及其队列
struct Input
{
	std::string					m_modelPath;
	std::vector<std::string>	m_vmdPaths;
};
Input currentInput;
std::vector<Input> inputModels;

// 摄像机跟踪位移偏置
bool cameraTrack = true;
float modelXPos = 0.0;
float modelYPos = 0.0;
float modelZPos = 0.0;


// 摄像机位置  glm::vec3(0, 10, 50), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0)
//glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, -40.0f);    // 控制摄像机位置glm::vec3(0.0f, 8.0f, 40.0f);
// 摄像机位置 球坐标
float radiusA = 40.0;
float radiusB = radiusA;
float seta = 0.0;
float fai = PI * 0.5;
float cameraY = 9.0f;
void cameraPosUpdate(float& x, float& y, float& z);
float x = radiusA * sin(fai) * sin(seta) + modelXPos;
float y = radiusB * cos(fai) + cameraY;
float z = radiusA * sin(fai) * cos(seta) + modelZPos;

// 暂停标志
bool stopPlay = false;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
// 避免跳帧
bool firstMouseL = true;
bool firstMouseM = true;
bool firstMouseR = true;
float cameraTargetX = 0.0f;
float cameraTargetY = cameraY;
glm::vec3 cameraPos(x, y, z);//摄像机位置坐标
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // 用于调节摄像机的远近
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);      // 用于摄像机视角的移动正负
glm::vec3 cameraTarget = glm::vec3(cameraTargetX, cameraTargetY, 0.0f);  // 用于控制摄像机的焦点坐标  9
// 摄像机方向  场景原点
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

// 记录鼠标按下事件
bool leftButton = false;
bool midButton = false;
bool rightButton = false;

// 记录鼠标位置
double cursorXPos = SCR_WIDTH / 2.0;
double cursorYPos = SCR_HEIGHT / 2.0;

// 记录触发次数
int rNum = 0;
int mNum = 0;
int lNum = 0;

// 初始化model标志
bool ini = false;

// 读取失败vmd文件
void readVmdFailPath()
{
    std::ifstream ifile;
    //char str1[256];
    std::string linestr;
    ifile.open(exePath + "\\" + "vmdFail.txt");
    while (ifile)
    {
        std::getline(ifile, linestr);
        //ifile.getline(str1, 256);
        //std::cout << str1 << std::endl;
        std::cout << linestr << std::endl;
        if (linestr != "")
        {
            vmdReadArr.push_back(linestr);
        }
    }
    ifile.close();
}

// 写入打开失败的路径
void writeVmdFailPath(std::string vmdTemp)
{
    bool vmdIsExist = false;
    std::cout << "write vmd path." << std::endl;
    for (std::string path : vmdReadArr)
    {
        if (vmdTemp == path)
        {
            std::cout << "failure vmdFile is exist aready!" << std::endl;
            vmdIsExist = true;
            return;
        }
    }
    if(!vmdIsExist)  // 该文件没有写入
    {
        std::ofstream writeVmd(exePath + "\\" + "vmdFail.txt");
        vmdReadArr.push_back(vmdTemp);
        for (std::string line : vmdReadArr)
        {
            std::cout << "path Lines:   " << line << std::endl;
            writeVmd << line << std::endl;
        }
        writeVmd.close();
    }
}

// 摄像机高度更新
void cameraHighUpdate(float cameY)
{
    // 摄像机位置
    cameraY = cameY;
    cameraPosUpdate(x, y, z);

    // 控制摄像机焦点位置
    cameraTargetY = cameY;
    cameraTarget = glm::vec3(cameraTargetX+modelXPos, cameraTargetY, 0.0f+modelZPos);  // 用于控制摄像机的视点  
}

// 十进制转二进制
int intTtransToBin(int x)
{
    int p = 1, y = 0, yushu, num = x;
    while (1)
    {
        yushu = x % 2;
        x /= 2;
        y += yushu * p;
        p *= 10;
        if (x < 2)
        {
            y += x * p;
            break;
        }
    }
    std::cout << "int = " << num << "       bin = " << y << std::endl;
    return y;
}

// 设置图标

// 读取配置文件，如窗口透明，结束时间设置
void config(std::string conFile)
{
    std::string File = exePath + "\\" + conFile;
    std::ifstream in(File);
    std::string configArr[100];
    std::string line;
    int i = 0;
    if (in)
    {
        while (getline(in, line)) // line中不包括每行的换行符  
        {
            std::cout << line << std::endl;
            configArr[i] = line;
            i++;
        }
    }
    transWindow = atoi(configArr[0].c_str());
    frameMulti = atof(configArr[1].c_str());
    SCR_WIDTH = atoi(configArr[2].c_str());
    SCR_HEIGHT = atoi(configArr[3].c_str());
    winXPos = atoi(configArr[4].c_str());
    winYPos = atoi(configArr[5].c_str());
    /*右键拖放 左键拖动 中键拖放 中键缩放放 灵敏度*/
    Rsensitivity = atof(configArr[6].c_str());
    Lsensitivity = atof(configArr[7].c_str());
    Msensitivity = atof(configArr[8].c_str());
    MSsensitivity = atof(configArr[9].c_str());
    isMSAA = atoi(configArr[10].c_str());
    MSAA_NUM = atoi(configArr[11].c_str());
    frameNumSec = atof(configArr[12].c_str());
    SC_W = atoi(configArr[13].c_str());
    SC_H = atoi(configArr[14].c_str());
    radiusA = atof(configArr[15].c_str());
    rColor = atof(configArr[16].c_str());
    gColor = atof(configArr[17].c_str());
    bColor = atof(configArr[18].c_str());
    aphaColor = atof(configArr[19].c_str());
    windowFloating = atoi(configArr[20].c_str());
    sleepTime = atoi(configArr[21].c_str());
    int singleMode = atoi(configArr[22].c_str());
    cpuNum = atoi(configArr[23].c_str());
    cameraY = atof(configArr[24].c_str());
    cameraTrack = (atoi(configArr[25].c_str()) == 1) ? true : false;

    cameraHighUpdate(cameraY);
    std::cout << "cpu mode:" << singleMode << std::endl;
    if (singleMode)
    {
        singleModeCpu = true;
    }
    else
    {
        singleModeCpu = false;
    }
	std::cout << "transWindow:" << transWindow << "    frameMulti:" << frameMulti << "  宽高：" << SCR_WIDTH << " " << SCR_HEIGHT << " 位置:" << winXPos << " " << winYPos << std::endl;
	std::cout << "isMSAA:" << isMSAA << "    MSAA_NUM:" << MSAA_NUM << std::endl;
	std::cout << "frameNumSec:" << frameNumSec << std::endl;
}

// 获取当前路径
std::string GetExePath()
{
	char exeFullPath[MAX_PATH]; // Full path 
	std::string strPath = "";

	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
	strPath = (std::string)exeFullPath;    // Get full path of the file 

	int pos = strPath.find_last_of('\\', strPath.length());
	return strPath.substr(0, pos);  // Return the directory without the file name

}

// 读取文件初始化
void modelVmdIni(std::string fileName)
{
	std::string iniFile = exePath + "\\" + fileName;
	std::ifstream in(iniFile);
	std::string filename;
	std::string line;
	bool isModels = false;
	bool isVmds = false;
	if (fileName.find("model") != std::string::npos)
	{
		isModels = true;
	}
	else if (fileName.find("vmd") != std::string::npos)
	{
		isVmds = true;
	}
	if (in)
	{
		if (isModels)
		{
			ini = true;
			int i = 0;
			while (getline(in, line)) // line中不包括每行的换行符  
			{
				std::cout << line << std::endl;
				modelArr[i] = line;
				i++;
			}
			maxModelIndex = i;
		}
		else if (isVmds)
		{
			int i = 0;
			while (getline(in, line)) // line中不包括每行的换行符  
			{
				std::cout << line << std::endl;
				vmdArr[i] = line;
				i++;
			}
			maxVmdIndex = i;
		}
	}
	else
	{

		std::cout << "ini.ini 文件打开失败..." << std::endl;
	}
}

// 更新摄像机位置
void cameraPosUpdate(float& x, float& y, float& z)
{
    // 自带镜头追踪
    x = radiusA * sin(fai) * sin(seta) + modelXPos;
    y = radiusB * cos(fai) + cameraY;
    z = radiusA * sin(fai) * cos(seta) + modelZPos;
}

void cameraTargetUpdate(float cameraTargetX, float cameraTargetY)
{
    // 镜头追踪
    cameraTarget = glm::vec3(cameraTargetX + modelXPos, cameraTargetY, 0.0f + modelZPos); 
}

// 模型平移  OpenGL的glTranslatef平移变换函数详解  处理鼠标点击
void curse_poscallback(GLFWwindow* window, double x, double y)
{
	if (midButton)
	{
		cursorXPos = x;
		cursorYPos = y;
		std::cout << "鼠标坐标：：" << "(pos:" << x << "," << y << ")" << std::endl;
	}
	//std::cout << "鼠标坐标1：：" << "(pos:" << x << "," << y << ")" << std::endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		midButton = true;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		leftButton = true;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		rightButton = true;
		break;
	default:
		break;
	}
	else if (action == GLFW_RELEASE) switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		midButton = false;
		firstMouseM = true;
		mNum = 0;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		leftButton = false;
		firstMouseL = true;
		lNum = 0;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		// std::cout << "GLFW_MOUSE_BUTTON_RIGHT 弹起" << std::endl;
		rightButton = false;
		firstMouseR = true;
		rNum = 0;
		break;
	case GLFW_MOUSE_BUTTON_4:
		stopPlay = !(stopPlay);
		//std::cout << "stopPlay:::::::::::" << stopPlay << std::endl;
	}
	return;
}

// 模型旋转  处理鼠标拖动

// 模型缩放
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	radiusA -= MSsensitivity * yoffset;
}

// OpenGL-旋转平移与缩放  https://blog.csdn.net/charles_neil/article/details/78664228

//摄像机视角
float yaw = -90.0f;	// 偏航被初始化为-90.0度，因为0.0的偏航会导致指向右边的方向向量，所以我们最初会向左旋转一点。
float pitch = 0.0f;  // 俯仰角
float lastXL = 800.0f / 2.0;
float lastYL = 600.0 / 2.0;
float lastXPos = SCR_WIDTH / 2.0;
float lastYPos = SCR_HEIGHT / 2.0;
float lastXR = 800.0f / 2.0;
float lastYR = 600.0 / 2.0;
float fov = 45.0f;

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{  // 消息滞后窗口移动：http://www.voidcn.com/article/p-xzflhcme-byu.html
    // OpenGL之鼠标控制坐标系旋转、缩放、移动：https://blog.csdn.net/Cracent/article/details/51049471
    /*std::cout << "mouse_callback OUT --> Xpos:" << xpos << " Ypos:" << ypos << std::endl;
    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);
    std::cout << " CursorPos::OUT --> Xpos:" << cx << " Ypos:" << cy << std::endl;*/
    if (rightButton)   // 旋转
	{
		if (firstMouseR)
		{
			lastXR = lastXR;
			lastYR = lastYR;
			firstMouseR = false;
		}
		else
		{
			rNum++;
		}
		float xoffset = xpos - lastXR;
		float yoffset = lastYR - ypos; // reversed since y-coordinates go from bottom to top
		lastXR = xpos;
		lastYR = ypos;
		if (!firstMouseR && rNum > 1)
		{
			xoffset *= Rsensitivity;
			yoffset *= Rsensitivity;
			seta -= xoffset;
			fai += yoffset;
		}
	}

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (leftButton)  // 模型移动
	{
		if (firstMouseL)
		{
			lastXL = lastXL;
			lastYL = lastYL;
			firstMouseL = false;
		}
		else
		{
			lNum++;
		}
		float xoffset = xpos - lastXL;
		float yoffset = lastYL - ypos; // reversed since y-coordinates go from bottom to top
		lastXL = xpos;
		lastYL = ypos;
		if (!firstMouseL && lNum > 1)
		{
			xoffset *= Lsensitivity;
			yoffset *= Lsensitivity;
			cameraTargetX -= xoffset;
			cameraTargetY -= yoffset;
			//seta -= xoffset;
			//std::cout << " Xpos:" << xpos << " Ypos:" << xpos << " xoffset:" << xoffset << " yoffset:" << yoffset << std::endl;
		}
		//std::cout << " OUT --> Xpos:" << xpos << " Ypos:" << xpos << " xoffset:" << xoffset << " yoffset:" << yoffset << std::endl;
	}

	if (seta >= 2 * PI)
		seta = 2 * PI;
	if (seta <= -2 * PI)
		seta = -2 * PI;
	if (fai >= 2 * PI)
		fai = 2 * PI;
	if (fai <= -2 * PI)
		fai = -2 * PI;
	if (midButton)  // 窗口移动
	{
		if (firstMouseM)
		{

			/*lastXPos = lastXPos;
			lastYPos = lastYPos;*/
            lastXPos = xpos;
            lastYPos = ypos;
			firstMouseM = false;
		}
		else
		{
			mNum++;
		}
		// 控制窗口的移动
		float xoffset = xpos - lastXPos;
		float yoffset = lastYPos - ypos; // reversed since y-coordinates go from bottom to top
		//lastXPos = xpos;
		//lastYPos = ypos;
		
		if (!firstMouseM && mNum > 1)
		{
			//std::cout << "lastXpos:" << lastXPos << " lastYpos:" << lastYPos << " Xpos:" << xpos << " Ypos:" << ypos << " xoffset:" << xoffset << " yoffset:" << yoffset << std::endl;
			// 灵敏度设置
			//float sensitivity = Msensitivity * deltaTime; // change this value to your liking
			float sensitivity = Msensitivity; // change this value to your liking
			glfwGetWindowPos(window, &winXPos, &winYPos);
			winXPos += xoffset * sensitivity;
			winYPos -= yoffset * sensitivity;
			// 更新窗口位置
			glfwSetWindowPos(window, winXPos, winYPos);
		}
		glfwGetWindowPos(window, &winXPos, &winYPos);
		//lastXPos = xpos + winXPos;
		//lastYPos = ypos + winYPos;
        //std::cout << "lastXpos:" << lastXPos << " lastYpos:" << lastYPos << " Xpos:" << xpos << " Ypos:" << ypos << " xoffset:" << xoffset << " yoffset:" << yoffset << std::endl;
	}
}

// 读取拖拽文件
void drop_callback(GLFWwindow* window, int count, const char** paths)
{
	int i;
	for (i = 0; i < count; i++)
	{
		/*std::cout << paths[i] << std::endl;*/
		bool isModels = false;
		bool isVmds = false;
		bool isConfig = false;
		bool isCamera = false;
		std::string newPath = std::string(paths[i]);
		if (newPath.find(".pmx") != std::string::npos)
		{
			isModels = true;
		}
		else if (newPath.find(".pmd") != std::string::npos)
		{
			isModels = true;
		}
		else if (newPath.find(".vmd") != std::string::npos)
		{
			isVmds = true;
			if (newPath.find("camera") != std::string::npos)
			{
				std::cout << "镜头文件" << paths[i] << std::endl;
				isCamera = true;
			}
			else
			{
				std::cout << "动作文件" << paths[i] << std::endl;
				isCamera = false;
			}
		}
		else if (newPath.find("config.ini") != std::string::npos)
		{
			std::cout << "已识别拖放文件config.ini:" << paths[i] << std::endl;
			config("config.ini");
			isConfig = true;
		}
		if (isModels)
		{
			std::cout << "model:" << paths[i] << std::endl;
			modelArr[maxModelIndex] = newPath;
			maxModelIndex++;
			currentInput.m_modelPath = newPath;
			drop = true;
			dropConfigFile = true;
		}
		else if (isVmds)
		{
			if (isCamera)
			{
				currentInput.m_vmdPaths.push_back(newPath);
				dropConfigFile = true;
			}
			else
			{
				std::cout << "vmd:" << paths[i] << std::endl;
				vmdArr[maxVmdIndex] = newPath;
				maxVmdIndex++;
				currentInput.m_vmdPaths.clear();
				currentInput.m_vmdPaths.push_back(newPath);
			}

			drop = true;
		}
		else if (isConfig)
		{
			drop = true;
			dropConfigFile = true;
		}
		else
		{
			drop = false;
		}
		inputModels.clear();
		inputModels.emplace_back(currentInput);
	}
}

// 摄像机控制
void processInput(GLFWwindow* window, float& seta, float& fai)
{
	/*if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);*/

	float cameraSpeed = 2.5 * deltaTime;
	//float cameraSpeed = 0.2;
	// 摄像机远近
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		seta += cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		seta -= cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		fai += cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		fai -= cameraSpeed;
}

GLuint CreateShader(GLenum shaderType, const std::string code)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader == 0)
	{
		std::cout << "Failed to create shader.\n";
		return 0;
	}
	const char* codes[] = { code.c_str() };
	GLint codesLen[] = { (GLint)code.size() };
	glShaderSource(shader, 1, codes, codesLen);
	glCompileShader(shader);

	GLint infoLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
	if (infoLength != 0)
	{
		std::vector<char> info;
		info.reserve(infoLength + 1);
		info.resize(infoLength);

		GLsizei len;
		glGetShaderInfoLog(shader, infoLength, &len, &info[0]);
		if (info[infoLength - 1] != '\0')
		{
			info.push_back('\0');
		}

		std::cout << &info[0] << "\n";
	}

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus != GL_TRUE)
	{
		glDeleteShader(shader);
		std::cout << "Failed to compile shader.\n";
		return 0;
	}

	return shader;
}

GLuint CreateShaderProgram(const std::string vsFile, const std::string fsFile)
{
	saba::TextFileReader vsFileText;
	if (!vsFileText.Open(vsFile))
	{
		std::cout << "Failed to open shader file. [" << vsFile << "].\n";
		return 0;
	}
	std::string vsCode = vsFileText.ReadAll();
	vsFileText.Close();

	saba::TextFileReader fsFileText;
	if (!fsFileText.Open(fsFile))
	{
		std::cout << "Failed to open shader file. [" << fsFile << "].\n";
		return 0;
	}
	std::string fsCode = fsFileText.ReadAll();
	fsFileText.Close();

	GLuint vs = CreateShader(GL_VERTEX_SHADER, vsCode);
	GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fsCode);
	if (vs == 0 || fs == 0)
	{
		if (vs != 0) { glDeleteShader(vs); }
		if (fs != 0) { glDeleteShader(fs); }
		return 0;
	}

	GLuint prog = glCreateProgram();
	if (prog == 0)
	{
		glDeleteShader(vs);
		glDeleteShader(fs);
		std::cout << "Failed to create program.\n";
		return 0;
	}
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	GLint infoLength;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLength);
	if (infoLength != 0)
	{
		std::vector<char> info;
		info.reserve(infoLength + 1);
		info.resize(infoLength);

		GLsizei len;
		glGetProgramInfoLog(prog, infoLength, &len, &info[0]);
		if (info[infoLength - 1] != '\0')
		{
			info.push_back('\0');
		}

		std::cout << &info[0] << "\n";
	}

	GLint linkStatus;
	glGetProgramiv(prog, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		glDeleteShader(vs);
		glDeleteShader(fs);
		std::cout << "Failed to link shader.\n";
		return 0;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}

struct AppContext;

struct MMDShader
{
	~MMDShader()
	{
		Clear();
	}

	GLuint	m_prog = 0;

	// attribute
	GLint	m_inPos = -1;
	GLint	m_inNor = -1;
	GLint	m_inUV = -1;

	// uniform
	GLint	m_uWV = -1;
	GLint	m_uWVP = -1;

	GLint	m_uAmbinet = -1;
	GLint	m_uDiffuse = -1;
	GLint	m_uSpecular = -1;
	GLint	m_uSpecularPower = -1;
	GLint	m_uAlpha = -1;

	GLint	m_uTexMode = -1;
	GLint	m_uTex = -1;
	GLint	m_uTexMulFactor = -1;
	GLint	m_uTexAddFactor = -1;

	GLint	m_uSphereTexMode = -1;
	GLint	m_uSphereTex = -1;
	GLint	m_uSphereTexMulFactor = -1;
	GLint	m_uSphereTexAddFactor = -1;

	GLint	m_uToonTexMode = -1;
	GLint	m_uToonTex = -1;
	GLint	m_uToonTexMulFactor = -1;
	GLint	m_uToonTexAddFactor = -1;

	GLint	m_uLightColor = -1;
	GLint	m_uLightDir = -1;

	GLint	m_uLightVP = -1;
	GLint	m_uShadowMapSplitPositions = -1;
	GLint	m_uShadowMap0 = -1;
	GLint	m_uShadowMap1 = -1;
	GLint	m_uShadowMap2 = -1;
	GLint	m_uShadowMap3 = -1;
	GLint	m_uShadowMapEnabled = -1;

	bool Setup(const AppContext& appContext);
	void Clear();
};

struct MMDEdgeShader
{
	~MMDEdgeShader()
	{
		Clear();
	}

	GLuint	m_prog = 0;

	// attribute
	GLint	m_inPos = -1;
	GLint	m_inNor = -1;

	// uniform
	GLint	m_uWV = -1;
	GLint	m_uWVP = -1;
	GLint	m_uScreenSize = -1;
	GLint	m_uEdgeSize = -1;

	GLint	m_uEdgeColor = -1;

	bool Setup(const AppContext& appContext);
	void Clear();
};

struct MMDGroundShadowShader
{
	~MMDGroundShadowShader()
	{
		Clear();
	}

	GLuint	m_prog = 0;

	// attribute
	GLint	m_inPos = -1;

	// uniform
	GLint	m_uWVP = -1;
	GLint	m_uShadowColor = -1;

	bool Setup(const AppContext& appContext);
	void Clear();
};

struct Texture
{
	GLuint	m_texture;
	bool	m_hasAlpha;
};

struct AppContext
{
	~AppContext()
	{
		Clear();
	}

	std::string m_resourceDir;
	std::string	m_shaderDir;
	std::string	m_mmdDir;

	std::unique_ptr<MMDShader>				m_mmdShader;
	std::unique_ptr<MMDEdgeShader>			m_mmdEdgeShader;
	std::unique_ptr<MMDGroundShadowShader>	m_mmdGroundShadowShader;

	glm::mat4	m_viewMat;
	glm::mat4	m_projMat;
	int			m_screenWidth = 0;
	int			m_screenHeight = 0;

	glm::vec3	m_lightColor = glm::vec3(1, 1, 1);
	glm::vec3	m_lightDir = glm::vec3(-0.5f, -1.0f, -0.5f);

	std::map<std::string, Texture>	m_textures;
	GLuint	m_dummyColorTex = 0;
	GLuint	m_dummyShadowDepthTex = 0;

	// 透明窗口渲染添加文件 +1
	const int m_msaaSamples = MSAA_NUM;

	bool		m_enableTransparentWindow = false;
	uint32_t	m_transparentFboWidth = 0;
	uint32_t	m_transparentFboHeight = 0;
	GLuint	m_transparentFboColorTex = 0;
	GLuint	m_transparentFbo = 0;
	GLuint	m_transparentFboMSAAColorRB = 0;
	GLuint	m_transparentFboMSAADepthRB = 0;
	GLuint	m_transparentMSAAFbo = 0;
	GLuint	m_copyTransparentWindowShader = 0;
	GLint	m_copyTransparentWindowShaderTex = -1;
	GLuint	m_copyTransparentWindowVAO = 0;
	// 透明窗口渲染添加文件

	float	m_elapsed = 0.0f;
	float	m_animTime = 0.0f;
	std::unique_ptr<saba::VMDCameraAnimation>	m_vmdCameraAnim;

	bool Setup();
	void Clear();

	// 透明窗口渲染添加文件
	void SetupTransparentFBO();
	// 透明窗口渲染添加文件

	Texture GetTexture(const std::string& texturePath);

	// 暂停 VMDplay
	void SetAnimationTime(double animTime) { m_animTime = animTime; }
	/*
	case ViewerContext::PlayMode::Play:
			m_context.SetAnimationTime(animTime + m_context.GetElapsed());
			break;
		case ViewerContext::PlayMode::Stop:
			m_context.SetAnimationTime(animTime);
	*/
	double GetElapsed() const { return m_elapsed; }
	double GetAnimationTime() const { return m_animTime; }
};

struct Material
{
	explicit Material(const saba::MMDMaterial& mat)
		: m_mmdMat(mat)
	{}

	const saba::MMDMaterial& m_mmdMat;
	GLuint	m_texture = 0;
	bool	m_textureHasAlpha = false;
	GLuint	m_spTexture = 0;
	GLuint	m_toonTexture = 0;
};

struct Model
{
	std::shared_ptr<saba::MMDModel>	m_mmdModel;
	std::unique_ptr<saba::VMDAnimation>	m_vmdAnim;

	GLuint	m_posVBO = 0;
	GLuint	m_norVBO = 0;
	GLuint	m_uvVBO = 0;
	GLuint	m_ibo = 0;
	GLenum	m_indexType;

	GLuint	m_mmdVAO = 0;
	GLuint	m_mmdEdgeVAO = 0;
	GLuint	m_mmdGroundShadowVAO = 0;

	std::vector<Material>	m_materials;

	bool Setup(AppContext& appContext);
	void Clear();

	void UpdateAnimation(const AppContext& appContext);
	void Update(const AppContext& appContext);
	void Draw(const AppContext& appContext);
	//void DrawBegin();
	//void DrawEnd();
};

/*
	AppContext
*/

bool AppContext::Setup()
{
	// Setup resource directory.
	m_resourceDir = saba::PathUtil::GetExecutablePath();
	m_resourceDir = saba::PathUtil::GetDirectoryName(m_resourceDir);
	m_resourceDir = saba::PathUtil::Combine(m_resourceDir, "resource");
	m_shaderDir = saba::PathUtil::Combine(m_resourceDir, "shader");
	m_mmdDir = saba::PathUtil::Combine(m_resourceDir, "mmd");

	m_mmdShader = std::make_unique<MMDShader>();
	if (!m_mmdShader->Setup(*this))
	{
		return false;
	}

	m_mmdEdgeShader = std::make_unique<MMDEdgeShader>();
	if (!m_mmdEdgeShader->Setup(*this))
	{
		return false;
	}

	m_mmdGroundShadowShader = std::make_unique<MMDGroundShadowShader>();
	if (!m_mmdGroundShadowShader->Setup(*this))
	{
		return false;
	}

	glGenTextures(1, &m_dummyColorTex);
	glBindTexture(GL_TEXTURE_2D, m_dummyColorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// MSAA 003 多重采样附件
	/*glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_dummyColorTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_NUM, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_dummyColorTex, 0);*/
	// MSAA 003 多重采样附件

	// MSAA 003 多重采样附件
	glGenTextures(1, &m_dummyShadowDepthTex);
	glBindTexture(GL_TEXTURE_2D, m_dummyShadowDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	/*glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_dummyShadowDepthTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_NUM, GL_DEPTH_COMPONENT16, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_dummyShadowDepthTex, 1);*/
	// MSAA 003 多重采样附件

	// 透明窗口渲染添加文件
	m_copyTransparentWindowShader = CreateShaderProgram(
		saba::PathUtil::Combine(m_shaderDir, "quad.vert"),
		saba::PathUtil::Combine(m_shaderDir, "copy_transparent_window.frag")
	);

	m_copyTransparentWindowShaderTex = glGetUniformLocation(m_copyTransparentWindowShader, "u_Tex");

	glGenVertexArrays(1, &m_copyTransparentWindowVAO);
	// 透明窗口渲染添加文件

	return true;
}

void AppContext::Clear()
{
	m_mmdShader.reset();
	m_mmdEdgeShader.reset();
	m_mmdGroundShadowShader.reset();

	for (auto& tex : m_textures)
	{
		glDeleteTextures(1, &tex.second.m_texture);
	}
	m_textures.clear();

	if (m_dummyColorTex != 0) { glDeleteTextures(1, &m_dummyColorTex); }
	if (m_dummyShadowDepthTex != 0) { glDeleteTextures(1, &m_dummyShadowDepthTex); }
	m_dummyColorTex = 0;
	m_dummyShadowDepthTex = 0;

	// 透明窗口渲染添加文件
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (m_transparentFbo != 0) { glDeleteFramebuffers(1, &m_transparentFbo); }
	if (m_transparentMSAAFbo != 0) { glDeleteFramebuffers(1, &m_transparentMSAAFbo); }
	if (m_transparentFboColorTex != 0) { glDeleteTextures(1, &m_transparentFboColorTex); }
	if (m_transparentFboMSAAColorRB != 0) { glDeleteRenderbuffers(1, &m_transparentFboMSAAColorRB); }
	if (m_transparentFboMSAADepthRB != 0) { glDeleteRenderbuffers(1, &m_transparentFboMSAADepthRB); }
	if (m_copyTransparentWindowShader != 0) { glDeleteProgram(m_copyTransparentWindowShader); }
	if (m_copyTransparentWindowVAO != 0) { glDeleteVertexArrays(1, &m_copyTransparentWindowVAO); }
	// 透明窗口渲染添加文件

	m_vmdCameraAnim.reset();
}

// 透明窗口渲染添加文件
void AppContext::SetupTransparentFBO()
{
	// Setup FBO
	if (m_transparentFbo == 0)
	{
		glGenFramebuffers(1, &m_transparentFbo);
		glGenTextures(1, &m_transparentFboColorTex);
		glGenFramebuffers(1, &m_transparentMSAAFbo);
		glGenRenderbuffers(1, &m_transparentFboMSAAColorRB);
		glGenRenderbuffers(1, &m_transparentFboMSAADepthRB);
	}

	if ((m_screenWidth != m_transparentFboWidth) || (m_screenHeight != m_transparentFboHeight))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//// MSAA 004
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		//// MSAA 004
		glBindTexture(GL_TEXTURE_2D, m_transparentFboColorTex);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA,
			m_screenWidth,
			m_screenHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_transparentFbo);
		//// MSAA 004
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_transparentFbo);
		//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		//// MSAA 004
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_transparentFboColorTex, 0);
		// 多重采样部分代码
		if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
		{
			std::cout << "Faile to bind framebuffer.\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_transparentFboMSAAColorRB);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screenWidth, m_screenHeight);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaaSamples, GL_RGBA, m_screenWidth, m_screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_transparentFboMSAADepthRB);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screenWidth, m_screenHeight);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaaSamples, GL_DEPTH24_STENCIL8, m_screenWidth, m_screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_transparentMSAAFbo);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_transparentFboColorTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_transparentFboMSAAColorRB);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_transparentFboMSAADepthRB);
		// 多重采样部分代码
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status)
		{
			std::cout << "Faile to bind framebuffer.\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_transparentFboWidth = m_screenWidth;
		m_transparentFboHeight = m_screenHeight;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_transparentMSAAFbo);
	glEnable(GL_MULTISAMPLE);
}
// 透明窗口渲染添加文件


Texture AppContext::GetTexture(const std::string& texturePath)
{
	auto it = m_textures.find(texturePath);
	if (it == m_textures.end())
	{
		stbi_set_flip_vertically_on_load(true);
		saba::File file;
		if (!file.Open(texturePath))
		{
			return Texture{ 0, false };
		}
		int x, y, comp;
		int ret = stbi_info_from_file(file.GetFilePointer(), &x, &y, &comp);
		if (ret == 0)
		{
			return Texture{ 0, false };
		}

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		int reqComp = 0;
		bool hasAlpha = false;
		if (comp != 4)
		{
			uint8_t* image = stbi_load_from_file(file.GetFilePointer(), &x, &y, &comp, STBI_rgb);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			stbi_image_free(image);
			hasAlpha = false;
		}
		else
		{
			uint8_t* image = stbi_load_from_file(file.GetFilePointer(), &x, &y, &comp, STBI_rgb_alpha);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			stbi_image_free(image);
			hasAlpha = true;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		m_textures[texturePath] = Texture{ tex, hasAlpha };

		return m_textures[texturePath];
	}
	else
	{
		return (*it).second;
	}
}

/*
	MMDShader
*/

bool MMDShader::Setup(const AppContext& appContext)
{
	m_prog = CreateShaderProgram(
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd.vert"),
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd.frag")
	);
	if (m_prog == 0)
	{
		return false;
	}

	// attribute
	m_inPos = glGetAttribLocation(m_prog, "in_Pos");
	m_inNor = glGetAttribLocation(m_prog, "in_Nor");
	m_inUV = glGetAttribLocation(m_prog, "in_UV");

	// uniform
	m_uWV = glGetUniformLocation(m_prog, "u_WV");
	m_uWVP = glGetUniformLocation(m_prog, "u_WVP");

	m_uAmbinet = glGetUniformLocation(m_prog, "u_Ambient");
	m_uDiffuse = glGetUniformLocation(m_prog, "u_Diffuse");
	m_uSpecular = glGetUniformLocation(m_prog, "u_Specular");
	m_uSpecularPower = glGetUniformLocation(m_prog, "u_SpecularPower");
	m_uAlpha = glGetUniformLocation(m_prog, "u_Alpha");

	m_uTexMode = glGetUniformLocation(m_prog, "u_TexMode");
	m_uTex = glGetUniformLocation(m_prog, "u_Tex");
	m_uTexMulFactor = glGetUniformLocation(m_prog, "u_TexMulFactor");
	m_uTexAddFactor = glGetUniformLocation(m_prog, "u_TexAddFactor");

	m_uSphereTexMode = glGetUniformLocation(m_prog, "u_SphereTexMode");
	m_uSphereTex = glGetUniformLocation(m_prog, "u_SphereTex");
	m_uSphereTexMulFactor = glGetUniformLocation(m_prog, "u_SphereTexMulFactor");
	m_uSphereTexAddFactor = glGetUniformLocation(m_prog, "u_SphereTexAddFactor");

	m_uToonTexMode = glGetUniformLocation(m_prog, "u_ToonTexMode");
	m_uToonTex = glGetUniformLocation(m_prog, "u_ToonTex");
	m_uToonTexMulFactor = glGetUniformLocation(m_prog, "u_ToonTexMulFactor");
	m_uToonTexAddFactor = glGetUniformLocation(m_prog, "u_ToonTexAddFactor");

	m_uLightColor = glGetUniformLocation(m_prog, "u_LightColor");
	m_uLightDir = glGetUniformLocation(m_prog, "u_LightDir");

	m_uLightVP = glGetUniformLocation(m_prog, "u_LightWVP");
	m_uShadowMapSplitPositions = glGetUniformLocation(m_prog, "u_ShadowMapSplitPositions");
	m_uShadowMap0 = glGetUniformLocation(m_prog, "u_ShadowMap0");
	m_uShadowMap1 = glGetUniformLocation(m_prog, "u_ShadowMap1");
	m_uShadowMap2 = glGetUniformLocation(m_prog, "u_ShadowMap2");
	m_uShadowMap3 = glGetUniformLocation(m_prog, "u_ShadowMap3");
	m_uShadowMapEnabled = glGetUniformLocation(m_prog, "u_ShadowMapEnabled");

	return true;
}

void MMDShader::Clear()
{
	if (m_prog != 0) { glDeleteProgram(m_prog); }
	m_prog = 0;
}

/*
	MMDEdgeShader
*/

bool MMDEdgeShader::Setup(const AppContext& appContext)
{
	m_prog = CreateShaderProgram(
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd_edge.vert"),
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd_edge.frag")
	);
	if (m_prog == 0)
	{
		return false;
	}

	// attribute
	m_inPos = glGetAttribLocation(m_prog, "in_Pos");
	m_inNor = glGetAttribLocation(m_prog, "in_Nor");

	// uniform
	m_uWV = glGetUniformLocation(m_prog, "u_WV");
	m_uWVP = glGetUniformLocation(m_prog, "u_WVP");
	m_uScreenSize = glGetUniformLocation(m_prog, "u_ScreenSize");
	m_uEdgeSize = glGetUniformLocation(m_prog, "u_EdgeSize");
	m_uEdgeColor = glGetUniformLocation(m_prog, "u_EdgeColor");

	return true;
}

void MMDEdgeShader::Clear()
{
	if (m_prog != 0) { glDeleteProgram(m_prog); }
	m_prog = 0;
}

/*
	MMDGroundShadowShader
*/

bool MMDGroundShadowShader::Setup(const AppContext& appContext)
{
	m_prog = CreateShaderProgram(
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd_ground_shadow.vert"),
		saba::PathUtil::Combine(appContext.m_shaderDir, "mmd_ground_shadow.frag")
	);
	if (m_prog == 0)
	{
		return false;
	}

	// attribute
	m_inPos = glGetAttribLocation(m_prog, "in_Pos");

	// uniform
	m_uWVP = glGetUniformLocation(m_prog, "u_WVP");
	m_uShadowColor = glGetUniformLocation(m_prog, "u_ShadowColor");

	return true;
}

void MMDGroundShadowShader::Clear()
{
	if (m_prog != 0) { glDeleteProgram(m_prog); }
	m_prog = 0;
}

/*
	Model
*/

bool Model::Setup(AppContext& appContext)
{
	if (m_mmdModel == nullptr)
	{
		return false;
	}

	// Setup vertices
	size_t vtxCount = m_mmdModel->GetVertexCount();
	glGenBuffers(1, &m_posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_norVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_uvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	size_t idxSize = m_mmdModel->GetIndexElementSize();
	size_t idxCount = m_mmdModel->GetIndexCount();
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize * idxCount, m_mmdModel->GetIndices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (idxSize == 1)
	{
		m_indexType = GL_UNSIGNED_BYTE;
	}
	else if (idxSize == 2)
	{
		m_indexType = GL_UNSIGNED_SHORT;
	}
	else if (idxSize == 4)
	{
		m_indexType = GL_UNSIGNED_INT;
	}
	else
	{
		return false;
	}

	// Setup MMD VAO
	glGenVertexArrays(1, &m_mmdVAO);
	glBindVertexArray(m_mmdVAO);

	const auto& mmdShader = appContext.m_mmdShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(mmdShader->m_inPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glVertexAttribPointer(mmdShader->m_inNor, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(mmdShader->m_inNor);

	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glVertexAttribPointer(mmdShader->m_inUV, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);
	glEnableVertexAttribArray(mmdShader->m_inUV);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup MMD Edge VAO
	glGenVertexArrays(1, &m_mmdEdgeVAO);
	glBindVertexArray(m_mmdEdgeVAO);

	const auto& mmdEdgeShader = appContext.m_mmdEdgeShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdEdgeShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(mmdEdgeShader->m_inPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glVertexAttribPointer(mmdEdgeShader->m_inNor, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(mmdEdgeShader->m_inNor);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup MMD Ground Shadow VAO
	glGenVertexArrays(1, &m_mmdGroundShadowVAO);
	glBindVertexArray(m_mmdGroundShadowVAO);

	const auto& mmdGroundShadowShader = appContext.m_mmdGroundShadowShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdGroundShadowShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(mmdGroundShadowShader->m_inPos);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup materials
	for (size_t i = 0; i < m_mmdModel->GetMaterialCount(); i++)
	{
		const auto& mmdMat = m_mmdModel->GetMaterials()[i];
		Material mat(mmdMat);
		if (!mmdMat.m_texture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_texture);
			mat.m_texture = tex.m_texture;
			mat.m_textureHasAlpha = tex.m_hasAlpha;
		}
		if (!mmdMat.m_spTexture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_spTexture);
			mat.m_spTexture = tex.m_texture;
		}
		if (!mmdMat.m_toonTexture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_toonTexture);
			mat.m_toonTexture = tex.m_texture;
		}
		m_materials.emplace_back(std::move(mat));
	}

	return true;
}

void Model::Clear()
{
	if (m_posVBO != 0) { glDeleteBuffers(1, &m_posVBO); }
	if (m_norVBO != 0) { glDeleteBuffers(1, &m_norVBO); }
	if (m_uvVBO != 0) { glDeleteBuffers(1, &m_uvVBO); }
	if (m_ibo != 0) { glDeleteBuffers(1, &m_ibo); }
	m_posVBO = 0;
	m_norVBO = 0;
	m_uvVBO = 0;
	m_ibo = 0;

	if (m_mmdVAO != 0) { glDeleteVertexArrays(1, &m_mmdVAO); }
	if (m_mmdEdgeVAO != 0) { glDeleteVertexArrays(1, &m_mmdEdgeVAO); }
	if (m_mmdGroundShadowVAO != 0) { glDeleteVertexArrays(1, &m_mmdGroundShadowVAO); }
	m_mmdVAO = 0;
	m_mmdEdgeVAO = 0;
	m_mmdGroundShadowVAO = 0;
}

void Model::UpdateAnimation(const AppContext& appContext)
{
	double animTime = appContext.GetAnimationTime();
	m_mmdModel->BeginAnimation();
	m_mmdModel->UpdateAllAnimation(m_vmdAnim.get(), appContext.m_animTime * 30.0f, appContext.m_elapsed);
	m_mmdModel->EndAnimation();
}

void Model::Update(const AppContext& appContext)
{
	m_mmdModel->Update();

	size_t vtxCount = m_mmdModel->GetVertexCount();
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vtxCount, m_mmdModel->GetUpdatePositions());
	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vtxCount, m_mmdModel->GetUpdateNormals());
	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * vtxCount, m_mmdModel->GetUpdateUVs());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (cameraTrack)
    {
        modelXPos = m_mmdModel->GetUpdatePositions()->x;
        modelYPos = m_mmdModel->GetUpdatePositions()->y;
        modelZPos = (m_mmdModel->GetUpdatePositions()->z);
        //std::cout << "m_mmdModel->GetUpdatePositions():::   " << "    " << modelXPos << "    " << modelYPos << "    " << modelZPos << std::endl;
    }
}

void Model::Draw(const AppContext& appContext)
{
	const auto& view = appContext.m_viewMat;
	const auto& proj = appContext.m_projMat;
    //std::cout << "view Position::" << glm::to_string(view) << std::endl;

	auto world = glm::mat4(1.0f);
    //std::cout << "world Position::" << glm::to_string(world) << std::endl;
	auto wv = view * world;
	auto wvp = proj * view * world;
	auto wvit = glm::mat3(view * world);
	wvit = glm::inverse(wvit);
	wvit = glm::transpose(wvit);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);

	// 抗锯齿
	if (isMSAA)
	{
		//glBindRenderbuffer(GL_RENDERBUFFER, m_currentMSAAColorTarget);
		//glBindFramebuffer(GL_FRAMEBUFFER, m_currentMSAAFrameBuffer);
		glEnable(GL_MULTISAMPLE);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_NUM, GL_RGBA, SCR_WIDTH, SCR_HEIGHT);
	}
	// 抗锯齿

	glEnable(GL_DEPTH_TEST);

	// Draw model
	size_t subMeshCount = m_mmdModel->GetSubMeshCount();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto& subMesh = m_mmdModel->GetSubMeshes()[i];
		const auto& shader = appContext.m_mmdShader;
		const auto& mat = m_materials[subMesh.m_materialID];
		const auto& mmdMat = mat.m_mmdMat;

		if (mat.m_mmdMat.m_alpha == 0)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdVAO);

		glUniformMatrix4fv(shader->m_uWV, 1, GL_FALSE, &wv[0][0]);
		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wvp[0][0]);

		bool alphaBlend = true;

		glUniform3fv(shader->m_uAmbinet, 1, &mmdMat.m_ambient[0]);
		glUniform3fv(shader->m_uDiffuse, 1, &mmdMat.m_diffuse[0]);
		glUniform3fv(shader->m_uSpecular, 1, &mmdMat.m_specular[0]);
		glUniform1f(shader->m_uSpecularPower, mmdMat.m_specularPower);
		glUniform1f(shader->m_uAlpha, mmdMat.m_alpha);

		glActiveTexture(GL_TEXTURE0 + 0);
		glUniform1i(shader->m_uTex, 0);
		if (mat.m_texture != 0)
		{
			if (!mat.m_textureHasAlpha)
			{
				// Use Material Alpha
				glUniform1i(shader->m_uTexMode, 1);
			}
			else
			{
				// Use Material Alpha * Texture Alpha
				glUniform1i(shader->m_uTexMode, 2);
			}
			glUniform4fv(shader->m_uTexMulFactor, 1, &mmdMat.m_textureMulFactor[0]);
			glUniform4fv(shader->m_uTexAddFactor, 1, &mmdMat.m_textureAddFactor[0]);
			glBindTexture(GL_TEXTURE_2D, mat.m_texture);
		}
		else
		{
			glUniform1i(shader->m_uTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glActiveTexture(GL_TEXTURE0 + 1);
		glUniform1i(shader->m_uSphereTex, 1);
		if (mat.m_spTexture != 0)
		{
			if (mmdMat.m_spTextureMode == saba::MMDMaterial::SphereTextureMode::Mul)
			{
				glUniform1i(shader->m_uSphereTexMode, 1);
			}
			else if (mmdMat.m_spTextureMode == saba::MMDMaterial::SphereTextureMode::Add)
			{
				glUniform1i(shader->m_uSphereTexMode, 2);
			}
			glUniform4fv(shader->m_uSphereTexMulFactor, 1, &mmdMat.m_spTextureMulFactor[0]);
			glUniform4fv(shader->m_uSphereTexAddFactor, 1, &mmdMat.m_spTextureAddFactor[0]);
			glBindTexture(GL_TEXTURE_2D, mat.m_spTexture);
		}
		else
		{
			glUniform1i(shader->m_uSphereTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glActiveTexture(GL_TEXTURE0 + 2);
		glUniform1i(shader->m_uToonTex, 2);
		if (mat.m_toonTexture != 0)
		{
			glUniform4fv(shader->m_uToonTexMulFactor, 1, &mmdMat.m_toonTextureMulFactor[0]);
			glUniform4fv(shader->m_uToonTexAddFactor, 1, &mmdMat.m_toonTextureAddFactor[0]);
			glUniform1i(shader->m_uToonTexMode, 1);
			glBindTexture(GL_TEXTURE_2D, mat.m_toonTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			glUniform1i(shader->m_uToonTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glm::vec3 lightColor = appContext.m_lightColor;
		glm::vec3 lightDir = appContext.m_lightDir;
		glm::mat3 viewMat = glm::mat3(appContext.m_viewMat);
		lightDir = viewMat * lightDir;
		glUniform3fv(shader->m_uLightDir, 1, &lightDir[0]);
		glUniform3fv(shader->m_uLightColor, 1, &lightColor[0]);

		if (mmdMat.m_bothFace)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}

		// 透明窗口渲染部分代码
		// 删除if (alphaBlend)改为下面if
		if (appContext.m_enableTransparentWindow)
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			// 删除glDisable(GL_BLEND);改为下面条件
			if (alphaBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
		// 透明窗口渲染部分代码
		glUniform1i(shader->m_uShadowMapEnabled, 0);
		glUniform1i(shader->m_uShadowMap0, 3);
		glUniform1i(shader->m_uShadowMap1, 4);
		glUniform1i(shader->m_uShadowMap2, 5);
		glUniform1i(shader->m_uShadowMap3, 6);

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid*)offset);

		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw edge
	glm::vec2 screenSize(appContext.m_screenWidth, appContext.m_screenHeight);
	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto& subMesh = m_mmdModel->GetSubMeshes()[i];
		int matID = subMesh.m_materialID;
		const auto& shader = appContext.m_mmdEdgeShader;
		const auto& mat = m_materials[subMesh.m_materialID];
		const auto& mmdMat = mat.m_mmdMat;

		if (!mmdMat.m_edgeFlag)
		{
			continue;
		}
		if (mmdMat.m_alpha == 0.0f)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdEdgeVAO);

		glUniformMatrix4fv(shader->m_uWV, 1, GL_FALSE, &wv[0][0]);
		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wvp[0][0]);
		glUniform2fv(shader->m_uScreenSize, 1, &screenSize[0]);
		glUniform1f(shader->m_uEdgeSize, mmdMat.m_edgeSize);
		glUniform4fv(shader->m_uEdgeColor, 1, &mmdMat.m_edgeColor[0]);

		bool alphaBlend = false;
		if (transWindow)  // 设置窗口透明
		{
			alphaBlend = true;
		}
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// 透明窗口渲染部分代码
		// 删除if (alphaBlend)
		if (appContext.m_enableTransparentWindow)
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			// 删除glDisable(GL_BLEND);
			if (alphaBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
		// 透明窗口渲染部分代码

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid*)offset);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	// Draw ground shadow
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1, -1);
	auto plane = glm::vec4(0, 1, 0, 0);
	auto light = -appContext.m_lightDir;
	auto shadow = glm::mat4(1);

	shadow[0][0] = plane.y * light.y + plane.z * light.z;
	shadow[0][1] = -plane.x * light.y;
	shadow[0][2] = -plane.x * light.z;
	shadow[0][3] = 0;

	shadow[1][0] = -plane.y * light.x;
	shadow[1][1] = plane.x * light.x + plane.z * light.z;
	shadow[1][2] = -plane.y * light.z;
	shadow[1][3] = 0;

	shadow[2][0] = -plane.z * light.x;
	shadow[2][1] = -plane.z * light.y;
	shadow[2][2] = plane.x * light.x + plane.y * light.y;
	shadow[2][3] = 0;

	shadow[3][0] = -plane.w * light.x;
	shadow[3][1] = -plane.w * light.y;
	shadow[3][2] = -plane.w * light.z;
	shadow[3][3] = plane.x * light.x + plane.y * light.y + plane.z * light.z;

	auto wsvp = proj * view * shadow * world;

	auto shadowColor = glm::vec4(rColor, gColor, bColor, aphaColor);

	// 透明窗口渲染部分代码
	// 删除if (shadowColor.a < 1.0f)
	if (appContext.m_enableTransparentWindow)
	{
		glEnable(GL_BLEND);
		// 删除glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);改下面语句
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		// 透明窗口渲染部分代码

		glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_NOTEQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_STENCIL_TEST);
	}
	else
	{
		// 透明窗口渲染部分代码
		// 删除glDisable(GL_BLEND);
		if (shadowColor.a < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_NOTEQUAL, 1, 1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
	// 透明窗口渲染部分代码
	glDisable(GL_CULL_FACE);

	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto& subMesh = m_mmdModel->GetSubMeshes()[i];
		int matID = subMesh.m_materialID;
		const auto& mat = m_materials[subMesh.m_materialID];
		const auto& mmdMat = mat.m_mmdMat;
		const auto& shader = appContext.m_mmdGroundShadowShader;

		if (!mmdMat.m_groundShadow)
		{
			continue;
		}
		if (mmdMat.m_alpha == 0.0f)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdGroundShadowVAO);

		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wsvp[0][0]);
		glUniform4fv(shader->m_uShadowColor, 1, &shadowColor[0]);

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid*)offset);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
}

void Usage()
{
	std::cout << "app [-model <pmd|pmx file path>] [-vmd <vmd file path>]\n";
	std::cout << "e.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n";
}

bool SampleMain(std::vector<std::string>& args)
{

	// 透明窗口渲染部分代码
	bool enableTransparentWindow = false;
	if (transWindow)  // 设置窗口透明
	{
		enableTransparentWindow = true;
	}
	// 透明窗口渲染部分代码

	// 读取model和vmd
	currentInput.m_modelPath = modelArr[0];
    int Num = 0;
    saba::VMDFile vmdFileStart;
    std::string vmdPath;
    while (true)
    {
        Num = rand() % maxVmdIndex;
        vmdPath = vmdArr[Num];
        if (saba::ReadVMDFile(&vmdFileStart, vmdPath.c_str()))
        {
            currentInput.m_vmdPaths.push_back(vmdPath);
            std::cout << "Successed to read VMD file." << vmdPath.c_str() << std::endl;
            break;
        }
        else
        {
            writeVmdFailPath(vmdPath);  // 写入加载失败的vmd文件
        }
        Sleep(1);
    }
    //writeVmdFailPath(vmdPath);  // 写入加载失败的vmd文件
	// 读取model和vmd

	if (!currentInput.m_modelPath.empty())
	{
		inputModels.emplace_back(currentInput);
	}

	// Initialize glfw
	if (!glfwInit())
	{
		return false;
	}
	AppContext appContext;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (isMSAA)
	{  // 启用抗锯齿
		// MSAA  001
		glfwWindowHint(GLFW_SAMPLES, MSAA_NUM);
	}
	// 窗口透明参考：： https://www.glfw.org/docs/3.3/window_guide.html#window_transparency
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);  // 窗口透明  OIT渲染  OpenGL

	// 透明窗口渲染部分代码
	if (enableTransparentWindow)
	{
		//glfwWindowHint(GLFW_SAMPLES, 0);
		// 窗口前置
		glfwWindowHint(GLFW_FLOATING, GL_TRUE);
		// 设置窗口透明
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
		// 无边框窗口设置 成功
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);	//没有边框和标题栏
		// 无边框窗口设置
	}

	// 窗口前置
	if (windowFloating)
	{
		glfwWindowHint(GLFW_FLOATING, GL_TRUE);
	}
	else
	{
		glfwWindowHint(GLFW_FLOATING, GL_FALSE);
	}

	// 透明窗口渲染部分代码
	//auto window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AnimSpriteBy星纪弥航kotori[B站]", nullptr, nullptr);
	auto window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "myFairy", nullptr, nullptr);

	// 设置初始窗口位置
	glfwSetWindowPos(window, winXPos, winYPos);

	// 设置图标
	GLFWimage icons[1];
	icons[0].pixels = stbi_load("myicon.png", &icons[0].width, &icons[0].height, 0, 0);
	glfwSetWindowIcon(window, 1, icons);
	stbi_image_free(icons[0].pixels);
	// 设置图标

	// 用于设置摄像机视角控制， 注册回调函数

	// glfwSetScrollCallback(window, scroll_callback);
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 摄像机视角
	// 注册鼠标回调函数

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, curse_poscallback);

	if (window == nullptr)
	{
		return false;
	}

	glfwMakeContextCurrent(window);

	if (gl3wInit() != 0)
	{
		return false;
	}

	glfwSwapInterval(0);
	// 抗锯齿
	// MSAA  002
	glEnable(GL_MULTISAMPLE);
	////glDisable(GL_MULTISAMPLE);
	//glEnable(GL_LINE_SMOOTH);//对线进行抗锯齿处理
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_POLYGON_SMOOTH);//对多边形进行抗锯齿处理
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// Initialize application
	if (!appContext.Setup())
	{
		std::cout << "Failed to setup AppContext.\n";
		return false;
	}

	// 透明窗口渲染部分代码
	appContext.m_enableTransparentWindow = enableTransparentWindow;
	// 透明窗口渲染部分代码

	// 动作计数
	int vmdNum = 0;
	//std::cout << "maxModelIndex:" << maxModelIndex << "    maxVmdIndex:" << maxVmdIndex << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		//models.clear();
		//model.Clear();
		//vmdAnim->Destroy();
		// Load MMD model
        modelXPos = 0.0;
        modelZPos = 0.0;
		std::vector<Model> models;
		//Load VMD animation
		auto vmdAnim = std::make_unique<saba::VMDAnimation>();
		Model model;
		for (const auto& input : inputModels)
		{
			// Load MMD model
			//Model model;
			auto ext = saba::PathUtil::GetExt(input.m_modelPath);
			if (ext == "pmd")
			{
				auto pmdModel = std::make_unique<saba::PMDModel>();
				if (!pmdModel->Load(input.m_modelPath, appContext.m_mmdDir))
				{
					std::cout << "Failed to load pmd file.\n";
					return false;
				}
				model.m_mmdModel = std::move(pmdModel);
			}
			else if (ext == "pmx")
			{
				auto pmxModel = std::make_unique<saba::PMXModel>();
				if (!pmxModel->Load(input.m_modelPath, appContext.m_mmdDir))
				{
					std::cout << "Failed to load pmx file.\n";
					return false;
				}
				model.m_mmdModel = std::move(pmxModel);
			}
			else
			{
				std::cout << "Unknown file type. [" << ext << "]\n";
				return false;
			}
			model.m_mmdModel->InitializeAnimation();

			//// Load VMD animation
			//auto vmdAnim = std::make_unique<saba::VMDAnimation>();
			if (!vmdAnim->Create(model.m_mmdModel))
			{
				std::cout << "Failed to create VMDAnimation.\n";
				return false;
			}
			for (const auto& vmdPath : input.m_vmdPaths)
			{
				saba::VMDFile vmdFile;
				if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str()))
				{
					std::cout << "Failed to read VMD file." << vmdPath.c_str() << std::endl;
					return false;
				}
				if (!vmdAnim->Add(vmdFile))
				{
					std::cout << "Failed to add VMDAnimation.\n";
					return false;
				}
				if (!vmdFile.m_cameras.empty())
				{
					auto vmdCamAnim = std::make_unique<saba::VMDCameraAnimation>();
					if (!vmdCamAnim->Create(vmdFile))
					{
						std::cout << "Failed to create VMDCameraAnimation.\n";
					}
					appContext.m_vmdCameraAnim = std::move(vmdCamAnim);
				}
			}

			vmdAnim->SyncPhysics(appContext.m_animTime);  // 0.0f

			maxAniTime = vmdAnim->GetMaxKeyTime();

			//vmdAnim->Evaluate(fmod(maxTime, float(maxAniTime)));
			maxTime = float(maxAniTime) / float(frameNumSec);

			std::cout << "maxTime:" << maxTime << std::endl;

			model.m_vmdAnim = std::move(vmdAnim);

			model.Setup(appContext);

			models.emplace_back(std::move(model));
		}
		double fpsTime = saba::GetTime();
		int fpsFrame = 0;
		double saveTime = saba::GetTime();
		//double maxTime = frameTime * maxAniTime;
		while (!glfwWindowShouldClose(window))
		{
			// 拖放操作检测
			if (drop)
			{
				drop = !drop;
				std::cout << "m_animTime:" << appContext.m_animTime << "   frameNum:" << frameNum << "          maxAniTime:" << maxAniTime << std::endl;
				appContext.m_vmdCameraAnim.reset();
				if (dropConfigFile == false)
				{
					appContext.m_animTime = 0.0f;
					frameNum = 0;
				}
				else
				{
					std::cout << "拖放配置文件 " << std::endl;
					dropConfigFile = !dropConfigFile;
					glfwSetWindowPos(window, winXPos, winYPos);
					glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);
					if (transWindow)
					{
						enableTransparentWindow = true;
						glfwWindowHint(GLFW_FLOATING, GL_TRUE);
						glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
						glfwWindowHint(GLFW_DECORATED, GL_FALSE);	//没有边框和标题栏	
					}
					else
					{
						enableTransparentWindow = false;
						glfwWindowHint(GLFW_FLOATING, GL_FALSE);
						glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_FALSE);
						glfwWindowHint(GLFW_DECORATED, GL_TRUE);	//没有边框和标题栏
					}
				}
				break;
			}
			// 循环播放设置
			//if (frameNum > frameMulti*maxAniTime)   // 帧数控制结束
			if (appContext.m_animTime > maxTime)  // 30帧每秒控制结束
			{
				//vmdNum++;
                while (true)
                {
                    vmdNum = rand() % maxVmdIndex;
                    vmdPath = vmdArr[vmdNum];
                    if (saba::ReadVMDFile(&vmdFileStart, vmdPath.c_str()))
                    {
                        std::cout << "Successed to read VMD file." << vmdPath.c_str() << std::endl;
                        break;
                    }
                    else
                    {
                        writeVmdFailPath(vmdPath);   // 写入打开失败的vmd文件
                    }
                    Sleep(1);
                }  
                
				std::cout << "m_animTime:" << appContext.m_animTime << "   frameNum:" << frameNum << "          maxAniTime:" << maxAniTime << std::endl;
				appContext.m_animTime = 0.0f;
				frameNum = 0;
				// 清空动作
				currentInput.m_vmdPaths.clear();
				inputModels.clear();
				currentInput.m_vmdPaths.push_back(vmdArr[vmdNum]);
				std::cout << "vmdpush_back:" << vmdArr[vmdNum] << std::endl;
				for (auto vmd : currentInput.m_vmdPaths)
				{
					std::cout << "next vmd:" << vmd << std::endl;
				}
				if (!currentInput.m_modelPath.empty())
				{
					inputModels.emplace_back(currentInput);
				}
				break;
			}
			// 循环播放设置
			// std::cout << "m_animTime:" << appContext.m_animTime << "   frameNum:" << frameNum << "          maxAniTime:" << maxAniTime << std::endl;
			// per-frame time logic  用于摄像机移动速度
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			double time = saba::GetTime();
			double elapsed = time - saveTime;
			if (elapsed > 1.0 / 30.0)
			{
				elapsed = 1.0 / 30.0;
			}
			saveTime = time;
			appContext.m_elapsed = float(elapsed);

			// 测试暂停
			if (!stopPlay)
			{
				appContext.m_animTime += float(elapsed);
				frameNum++;
			}
			else
			{
				std::cout << "m_animTime:" << appContext.m_animTime << "   frameNum:" << frameNum << "          maxAniTime:" << maxAniTime << "   maxTime:" << maxTime << std::endl;
			}
			// 测试暂停
			/*appContext.m_animTime += float(elapsed);
			frameNum++;*/
			// appContext.m_animTime += float(elapsed);

			glfwSetMouseButtonCallback(window, mouse_button_callback);
			glfwSetScrollCallback(window, scroll_callback);
			glfwSetCursorPosCallback(window, curse_poscallback);
			// 获取模型视角变换
			glfwSetCursorPosCallback(window, mouse_callback);
			// 获取摄像机移动数据
			/*cameraPosUpdate(x, y, z);
			cameraTargetUpdate(cameraTargetX, cameraTargetY);*/

			// 获取拖拽文件路径回调函数
			glfwSetDropCallback(window, drop_callback);

			/*glClearColor(1.0f, 0.8f, 0.75f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			删除*/
			// 透明窗口渲染部分代码
			if (enableTransparentWindow)
			{
				appContext.SetupTransparentFBO();
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
			else
			{
				glClearColor(1.0f, 0.8f, 0.75f, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
			// 透明窗口渲染部分代码

	//		// 设置窗体背景颜色
	//		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//		//glClear(GL_COLOR_BUFFER_BIT);
	//
	//		// 清除窗口
	//		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			appContext.m_screenWidth = width;
			appContext.m_screenHeight = height;

			glViewport(0, 0, width, height);

			
            // update MMD
			for (auto& model : models)
			{
				// Update Animation
				model.UpdateAnimation(appContext);

				// Update Vertices
				model.Update(appContext);

                // update camera
                cameraPosUpdate(x, y, z);
                cameraTargetUpdate(cameraTargetX, cameraTargetY);

				// Draw
				model.Draw(appContext);
			}

            // Setup camera
			if (appContext.m_vmdCameraAnim)
			{
				appContext.m_vmdCameraAnim->Evaluate(appContext.m_animTime * 30.0f);
				const auto mmdCam = appContext.m_vmdCameraAnim->GetCamera();
				saba::MMDLookAtCamera lookAtCam(mmdCam);
				appContext.m_viewMat = glm::lookAt(lookAtCam.m_eye, lookAtCam.m_center, lookAtCam.m_up);
				appContext.m_projMat = glm::perspectiveFovRH(mmdCam.m_fov, float(width), float(height), 1.0f, 10000.0f);
			}
			else
			{
				// appContext.m_viewMat = glm::lookAt(glm::vec3(0, 10, 50), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));  // 摄像机
				// LookAt函数  camera/view transformation  位置  目标  上向量
				// appContext.m_viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
				float radius = 10.0f;
				float camX = sin(glfwGetTime()) * radius;
				float camZ = cos(glfwGetTime()) * radius;
				//std::cout << "(x,y,z) -->" << "(" << x << "," << y << "," << z << ")" << std::endl;
                // 摄像机位置向量，目标位置向量， 上向量
				appContext.m_viewMat = glm::lookAt(glm::vec3(x, y, z), cameraTarget, cameraUp);
				// 透视投影
				appContext.m_projMat = glm::perspectiveFovRH(glm::radians(30.0f), float(width), float(height), 1.0f, 10000.0f);  // 视角
				// appContext.m_projMat = glm::perspectiveFovRH(glm::radians(fov), float(width), float(height), 1.0f, 10000.0f);  // 视角 SCR_WIDTH SCR_HEIGHT
			}

			// 透明窗口渲染部分代码
			if (enableTransparentWindow)
			{
				glDisable(GL_MULTISAMPLE);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, appContext.m_transparentFbo);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, appContext.m_transparentMSAAFbo);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

				glDisable(GL_DEPTH_TEST);
				glBindVertexArray(appContext.m_copyTransparentWindowVAO);
				glUseProgram(appContext.m_copyTransparentWindowShader);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, appContext.m_transparentFboColorTex);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindVertexArray(0);
				glUseProgram(0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			// 透明窗口渲染部分代码

			glfwSwapBuffers(window);
			glfwPollEvents();

			//FPS
			{
				fpsFrame++;
				double time = saba::GetTime();
				double deltaTime = time - fpsTime;
				if (deltaTime > 1.0)
				{
					double fps = double(fpsFrame) / deltaTime;
					//std::cout << fps << " fps\n";
					fpsFrame = 0;
					fpsTime = time;
				}
			}
			Sleep(sleepTime);
		}
        Sleep(1);  // 外层循环
	}

	appContext.Clear();

	glfwTerminate();

	return true;
}

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

int main(int argc, char** argv)
{
    SetConsoleOutputCP(65001);//更改cmd编码为utf8
	// 设置cpu运行核心
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	int corenum = info.dwNumberOfProcessors;              // 计算核心数
	std::cout << "cpu:" << corenum << std::endl;
	
    srand((unsigned)time(NULL));

    // 获取程序运行路径
	exePath = GetExePath();
	std::cout << "程序路径：" << exePath << std::endl;
	// 获取程序运行路径

    // 读取vmd失败列表
    readVmdFailPath();
    // 读取vmd失败列表
	// 读取初始配置文件
	std::string vmdFile = "vmd_ini.ini";
	std::string modelFile = "model_ini.ini";
	modelVmdIni(vmdFile);
	modelVmdIni(modelFile);
    config("config.ini");
	// 读取初始配置文件
        

    // 设置cpu亲缘性，相关性
    if (cpuNum >= corenum)
    {
        cpuNum = corenum;
    }

    if (singleModeCpu == true)      // 单核心运行模式
    {
        SetProcessAffinityMask(GetCurrentProcess(), pow(2, cpuNum));
        std::cout << "single cpu run mode..." << std::endl;
    }
    else          // 多核心运行模式
    {
        SetProcessAffinityMask(GetCurrentProcess(), pow(2, cpuNum) - 1);
        std::cout << "Multi cpu run mode..." << std::endl;
    }
    
    // 设置cpu亲缘性，相关性

	if (argc < 2 && !ini)
	{
		Usage();
		return 1;
	}

	std::vector<std::string> args(argc);
#if _WIN32
	{
		WCHAR* cmdline = GetCommandLineW();
		int wArgc;
		WCHAR** wArgs = CommandLineToArgvW(cmdline, &wArgc);
		for (int i = 0; i < argc; i++)
		{
			args[i] = saba::ToUtf8String(wArgs[i]);
		}
	}
#else // _WIN32
	for (int i = 0; i < argc; i++)
	{
		args[i] = argv[i];
	}
#endif

	if (!SampleMain(args))
	{
		std::cout << "Failed to run.\n";
		return 1;
	}

	//system("pause");
	return 0;
}