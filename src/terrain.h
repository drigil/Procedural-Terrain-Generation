#ifndef TINYTRIANGLES_TERRAIN_H
#define TINYTRIANGLES_TERRAIN_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include "utils.h"

class terrain{

	public:

		int height;
		int width;
		std::vector<float> noiseMap;
		glm::vec3* vertices;
		int* triangles; // Contains indices of vertices
		GLfloat* finalArr;

		terrain(int h, int w, std::vector<float> nm){
			height = h;
			width = w;
			noiseMap = nm;
		}

		int getVerticesCount(int height, int width){
			return width * height; 
		}

		int getTriangleVerticesCount(int height, int width){
			return (width-1) * (height-1) * 6; //Number of squares * number of triangles(2) * 3(number of vertices in triangle) 
		}

		//Get array of vertices (vec3)
  		void getVertices(int height, int width) {

		    vertices = new glm::vec3[getVerticesCount(height, width)];
		    triangles = new int[getTriangleVerticesCount(height, width)];
		    int vertexIndex = 0; // Keep track of vertex
		    int triangleIndex = 0; //keep track of triangle
		    
		    float topLeftX = (width-1) / (-2.0f);
		    float topLeftZ = (height-1) / (2.0f); //Both used for centering the mesh

		    for ( int row=0; row<height; row++ ) {
		        for ( int col=0; col<width; col++ ) {
		          
		            glm::vec3 currVertex = glm::vec3(topLeftX + (float) col, 100 * noiseMap[(row*width) + col], topLeftZ - (float) row);
		            vertices[vertexIndex] = currVertex;

		            if(col<width-1 && row<height-1){
		            	
		            	//Add triangle 1
		            	triangles[triangleIndex] = vertexIndex;
		            	triangles[triangleIndex + 1] = vertexIndex + width + 1;
		            	triangles[triangleIndex + 2] = vertexIndex + width;

		            	triangleIndex = triangleIndex + 3;

		            	//Add triangle 2
		            	triangles[triangleIndex] = vertexIndex + width + 1;
		            	triangles[triangleIndex + 1] = vertexIndex;
		            	triangles[triangleIndex + 2] = vertexIndex + 1;

		            	triangleIndex = triangleIndex + 3;

		            }

		            vertexIndex++;

		        }
		    }

		}

		GLfloat* getTrianglePoints(){

			getVertices(height, width);

			finalArr = new GLfloat[getTriangleVerticesCount(height, width) * 3];
			int triangleIndex = 0;
			int pointIndex = 0;

			for ( int row=0; row<height-1; row++ ) {
		        for ( int col=0; col<width-1; col++ ) {
		        	for(int i = 0; i<6; i++){
		        		for(int j = 0; j<3; j++){
		        			
		        			finalArr[pointIndex] = vertices[triangles[triangleIndex]][j];
		        			pointIndex++;
		        		}
		        		triangleIndex++;
		        	}
		        			
		        }
		    }

		    return finalArr;
		}

};

#endif