#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <memory>
#include <strings.h>
#include <vector>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include <opencv2/opencv.hpp>

const int IMG_WIDTH = 1200;


struct Shared {
    int iteration = 5;
    float scale = 0.9f;

    bool isChanged(Shared sh){
        return sh.iteration != iteration || sh.scale != scale;
    }
};


// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT){
            if (key == GLFW_KEY_LEFT){
                if (sh.iteration > 0)
                    sh.iteration--;
            }
            if (key == GLFW_KEY_RIGHT){
                if (sh.iteration < 10)
                    sh.iteration++;
            }
            if (key == GLFW_KEY_UP){
                if (sh.scale < 5.0f)
                    sh.scale += 0.1;
            }
            if (key == GLFW_KEY_DOWN){
                if (sh.scale > 0.5f)
                    sh.scale -= 0.1;
            }
            if (key == GLFW_KEY_R ){
                shader.recompile();
            }
        }
	}
    
    Shared get_shared(){
        return sh;
    }

private:
	ShaderProgram& shader;
    Shared sh;
};

void print_vec3(glm::vec3 vec){
    std::cerr<<"{ ";
    std::cerr<< vec[0] << ", " << vec[1] << ", " << vec[2];
    std::cerr<<" }, ";
}

void print_vec(std::vector<glm::vec3> vec){
    for (auto i : vec){
        std::cerr<<"{ ";
        print_vec3(i);
        std::cerr<<" }\n";
    }
}

glm::vec3 center(std::vector<glm::vec3> points, int start, int end){
    float sumx = 0;
    float sumy = 0;
    float sumz = 0;

    for (int i = start; i < end; i++){
        sumx += points[i][0];
        sumy += points[i][1];
        sumz += points[i][2];
    }

    return glm::vec3{
        sumx / (end - start),
        sumy / (end - start),
        sumz / (end - start),
    };
}

std::vector<glm::vec3> get_next_hilbert(Shared sh, std::vector<glm::vec3> points){
    std::vector<glm::vec3> result;
    
    glm::vec3 stepx = (points[3] - points[0]) / 3.0f;
    glm::vec3 stepy = (points[1] - points[0]) / 3.0f;

    for (int i = 0; i < points.size(); i+=4){
        glm::vec3 stepx = (points[i+3] - points[i+0]) / 3.0f;
        glm::vec3 stepy = (points[i+1] - points[i+0]) / 3.0f;
        result.insert(result.end(), {
                points[i+0],
                points[i+0] + stepx,
                points[i+0] + stepx + stepy,
                points[i+0] + stepy,
                points[i+1] - stepy,
                points[i+1],
                points[i+1] + stepx,
                points[i+1] + stepx - stepy,
                points[i+2] - stepy - stepx,
                points[i+2] - stepx,
                points[i+2],
                points[i+2] - stepy,
                points[i+3] + stepy,
                points[i+3] + stepy - stepx,
                points[i+3] - stepx,
                points[i+3]
                });
    }
        for (int i = 15; i < result.size() - 4; i+=16){
        if (fabs(result[i+1][0] - result[i][0]) + fabs(result[i+1][1] - result[i][1]) > fabs(stepy[0]) + fabs(stepy[1])){
            glm::vec3 step = (result[i+1] - result[i]) / 3.0f ;
            for (int j = i+1; j < result.size(); j++){
                result[j] -= step * 2.0f;
            }
        }
    }
    // normalization
    float rate = 2 * sh.scale / (result[result.size()-1][0] - result[0][0]);
    for (auto &point : result)
        point *= rate;
    glm::vec3 distance = glm::vec3{-sh.scale, -sh.scale, 0.0f} - result[0];
    for (auto &point : result)
        point += distance;
    return result;
}

std::vector<glm::vec3> get_next_hilbert_colors(Shared sh, std::vector<glm::vec3> points, std::vector<std::vector<glm::vec3>> image){
    std::vector<glm::vec3> result;
    for (auto point : points){
        int x = (int)((((point[0] + sh.scale) / (2 * sh.scale)) * (IMG_WIDTH -1)));
        int y = (int)((((point[1] + sh.scale)) / (2 * sh.scale)) * (IMG_WIDTH -1));
        result.push_back(image[x][y]);
        
    }
    return result;
}

CPU_Geometry generate_hilbert_cpugeom(Shared sh, std::vector<std::vector<glm::vec3>> image){
    std::cerr<<"hilbert iteration: "<<sh.iteration<<", "<<sh.scale<<std::endl;
	CPU_Geometry cpuGeom;
    cpuGeom.verts.push_back(glm::vec3(-1 * sh.scale, -1 * sh.scale, 0.f));
	cpuGeom.verts.push_back(glm::vec3(-1 * sh.scale, sh.scale, 0.f));
	cpuGeom.verts.push_back(glm::vec3(sh.scale, sh.scale, 0.f));
	cpuGeom.verts.push_back(glm::vec3(sh.scale, -1 * sh.scale, 0.f));
    for (int i=0; i<sh.iteration; i++){
        cpuGeom.verts = get_next_hilbert(sh, cpuGeom.verts);
    }
    cpuGeom.cols = get_next_hilbert_colors(sh, cpuGeom.verts, image);

    return cpuGeom;
}

std::vector<std::vector<glm::vec3>> load_image(char* path){
    std::vector<std::vector<glm::vec3>> reslut;
    cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
    if (image.empty())
        std::cout<<"Image not found\n";
    cv::resize(image, image, cv::Size(IMG_WIDTH,IMG_WIDTH));
    cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
    for (int i = 0; i< IMG_WIDTH; i++){
        std::vector<glm::vec3> inner_result;
        for (int j = 0; j < IMG_WIDTH; j++){
            inner_result.push_back(glm::vec3{
                    (float)image.at<cv::Vec3b>(i,j)[2] / 256,
                    (float)image.at<cv::Vec3b>(i,j)[1] / 256,
                    (float)image.at<cv::Vec3b>(i,j)[0] / 256,
                    });
        }
        reslut.push_back(inner_result);
    }
    return reslut;
}

int main(int argc, char** argv) {
	Log::debug("Starting main");
    
    char* image_file_path;

    // check args
    if (argc != 2){
        std::cout << "usage: ./hilbert {image_file_path}\n\
            ex: ./hilbert test.png\n";
        return 0;
    }
    else{
        std::cout << "loading image: " << argv[1] << "\n";
        image_file_path = argv[1];
    }

    auto image = load_image(image_file_path);
	// WINDOW
	glfwInit();
    // can set callbacks at construction if desired
	Window window(IMG_WIDTH, IMG_WIDTH, "CPSC 453");

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
    auto callbacks = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;
    
    Shared sh;
    cpuGeom = generate_hilbert_cpugeom(sh, image);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();
        
        if (sh.isChanged(callbacks->get_shared())){
            sh = callbacks->get_shared();
            cpuGeom = generate_hilbert_cpugeom(sh, image);
            
            gpuGeom.setVerts(cpuGeom.verts);
            gpuGeom.setCols(cpuGeom.cols);
        }
		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_LINE_STRIP, 0, cpuGeom.verts.size());
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
