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

		std::vector<int> position;
		int height;
		int width;
		float heightMultiplier;
		float mapScale;
		std::vector<float> noiseMap;
		glm::vec3* vertices;
		GLfloat* normals;
		int* triangles; // Contains indices of vertices
		GLfloat* finalArr;

		terrain(std::vector<int> pos, int h, int w, float hm, float ms, std::vector<float> nm){
			position = pos;
			height = h;
			width = w;
			heightMultiplier = hm;
			mapScale = ms;
			noiseMap = nm;
		}

		int getVerticesCount(int height, int width){
			return width * height; 
		}

		int getTriangleVerticesCount(int height, int width){
			return (width-1) * (height-1) * 6; //Number of squares(width-1 * height - 1) * number of triangles(2) * 3(number of vertices in triangle) 
		}

		//Get array of vertices (vec3)
  		void getVertices(int height, int width) {

		    vertices = new glm::vec3[getVerticesCount(height, width)];
		    triangles = new int[getTriangleVerticesCount(height, width)];
		    int vertexIndex = 0; // Keep track of vertex
		    int triangleIndex = 0; //keep track of triangle
		    
		    float topLeftX = position[0]*(width-1) - ((width-1) / 2.0f);
		    float topLeftZ = position[1]*(height-1) + ((height-1) / 2.0f); //Both used for centering the mesh

		    for ( int row=0; row<height; row++ ) {
		        for ( int col=0; col<width; col++ ) {
		          	//Multiply height by height multiplier and cube of noise map
		            glm::vec3 currVertex = glm::vec3(mapScale * (topLeftX + (float) col), heightMultiplier * pow(noiseMap[(row*width) + col], 3), mapScale*(topLeftZ - (float) row));
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

		//Call after getTrianglePoints makes updated to vec3 normals
		//Seg Fault
		GLfloat* getNormals(int height, int width){

			normals = new GLfloat[getTriangleVerticesCount(height, width) * 3];
			int normalIndex = 0; // Keep track of vertex
			int triangleIndex = 0; //keep track of triangle
			glm::vec3 normal;
			
			for ( int row=0; row<height; row++ ) {
				for ( int col=0; col<width; col++ ) {

					if(col<width-1 && row<height-1){

						//Get normal for triangle 1
						glm::vec3 side1 = vertices[triangles[triangleIndex + 1]] - vertices[triangles[triangleIndex]];
						glm::vec3 side2 = vertices[triangles[triangleIndex + 2]] - vertices[triangles[triangleIndex]];
						normal = glm::normalize(-glm::cross(side1, side2));
						
						normals[normalIndex  ] = normal[0];
						normals[normalIndex+1] = normal[1];
						normals[normalIndex+2] = normal[2];

						normals[normalIndex+3] = normal[0];
						normals[normalIndex+4] = normal[1];
						normals[normalIndex+5] = normal[2];

						normals[normalIndex+6] = normal[0];
						normals[normalIndex+7] = normal[1];
						normals[normalIndex+8] = normal[2];


						triangleIndex = triangleIndex + 3;
						normalIndex = normalIndex + 9;
						// std::cout << "first triangle" <<std::endl;

						//Get normal for triangle 1
						side1 = vertices[triangles[triangleIndex + 1]] - vertices[triangles[triangleIndex]];
						side2 = vertices[triangles[triangleIndex + 2]] - vertices[triangles[triangleIndex]];
						normal = glm::normalize(-glm::cross(side1, side2));

						normals[normalIndex  ] = normal[0];
						normals[normalIndex+1] = normal[1];
						normals[normalIndex+2] = normal[2];

						normals[normalIndex+3] = normal[0];
						normals[normalIndex+4] = normal[1];
						normals[normalIndex+5] = normal[2];

						normals[normalIndex+6] = normal[0];
						normals[normalIndex+7] = normal[1];
						normals[normalIndex+8] = normal[2];
						

						triangleIndex = triangleIndex + 3;
						normalIndex = normalIndex + 9;
						// std::cout << "second triangle" <<std::endl;

					}



				}
			}

			return normals;

		}




};

//Class for different types of terrains
class terrainType{

	public:

		char* name;
		float height;
		float red;
		float green;
		float blue;

		terrainType(char* n, float h, float r, float g, float b){
			name = n;
			height = h;
			red = r;
			green = g;
			blue = b;
		}

};

#endif