// References - 
// https://www.firespark.de/resources/downloads/implementation%20of%20a%20methode%20for%20hydraulic%20erosion.pdf
// https://github.com/SebLague/Hydraulic-Erosion/blob/ac6b3f04d106674945305970650426ef3d405360/Assets/Scripts/Erosion.cs#L130

#ifndef TINYTRIANGLES_EROSION_H
#define TINYTRIANGLES_EROSION_H

int seed;
float erosionRadius = 3.0f; //originally 3
float inertia = 0.05f; // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
float sedimentCapacityFactor = 4.0f; // Multiplier for how much sediment a droplet can carry
float minSedimentCapacity = 0.01f; // Used to prevent carry capacity getting too close to zero on flatter terrain
float erodeSpeed = 0.3f;
float depositSpeed = 0.3f;
float evaporateSpeed = 0.01f;
float gravity = 4.0f;
int maxDropletLifetime = 30;

float initialWaterVolume = 1.0f;
float initialSpeed = 1.0f;
float borderSize = 10.0f;

// Indices and weights of erosion brush precomputed for every node
std::vector<std::vector<int>> erosionBrushIndices; 
std::vector<std::vector<float>> erosionBrushWeights;

int currentSeed;
int currentErosionRadius;
int currWidth;
int currHeight;

//Get Height and Slpoe using nearby values
std::vector<float> getHeightAndGradient(std::vector<float> &noiseMap, int height, int width, float posX, float posY){
	std::cout<< "14 "<<std::endl;
	int nodeX = (int)posX;
	int nodeY = (int)posY;

	//Values used for bilinear interpolation
	float x = posX - nodeX;
	float y = posY - nodeY;

	int dropIndex = (nodeY * width) + nodeX; //Index of drop in the noiseMap
	//Calculate heights of the four nodes of the droplet's cell
	

	float heightTopLeft = noiseMap[dropIndex];
	float heightTopRight = noiseMap[dropIndex + 1];
	float heightBottomLeft = noiseMap[dropIndex + width];
	float heightBottomRight = noiseMap[dropIndex + width + 1];

	//Calculate drop's direction with bilinear interpolation of height difference along edges
	float gradientX = (heightTopRight - heightTopLeft)*(1-y) + (heightBottomRight - heightBottomLeft) * y;
	float gradientY = (heightBottomLeft - heightTopLeft)*(1-x) + (heightBottomRight - heightTopRight) * x;
	
	float newHeight = heightTopLeft*(1-x)*(1-y) + heightTopRight*(x)*(1-y) + heightBottomLeft*(1-x)*y + heightBottomRight*(x)*(y);
	std::vector<float> result{newHeight, gradientX, gradientY};
	return result;	
		
}

//Used to perform erosion
void initializeBrushIndices(int height, int width, int radius){
	
	erosionBrushIndices.clear();
	erosionBrushWeights.clear();

	std::vector<int> xOffsets(radius*radius*4, 0);
	std::vector<int> yOffsets(radius*radius*4, 0);
	std::vector<float> weights(radius*radius*4, 0);
	
	float weightSum = 0;
	int addIndex = 0;

	for(int i = 0; i<height*width; i++){ 
		int centreX = i%width;
		int centreY = i/height;

		if((centreY<=radius) || (centreY>=height - radius) || (centreX<=radius+1) || (centreX>=width - radius)){
			weightSum = 0;
			addIndex = 0;

			for (int y = -radius; y <= radius; y++) {
                for (int x = -radius; x <= radius; x++) {
                    float sqrDst = x * x + y * y;
                    if (sqrDst < radius * radius) {
                        int coordX = centreX + x;
                        int coordY = centreY + y;

                        if (coordX >= 0 && coordX < width && coordY >= 0 && coordY < height) {
                            float weight = 1 - sqrt (sqrDst) / radius;
                            weightSum += weight;
                            weights[addIndex] = weight;
                            xOffsets[addIndex] = x;
                            yOffsets[addIndex] = y;
                            addIndex++;
                        }
                    }
                }
            }

		}

		int numEntries = addIndex;
		
		erosionBrushIndices.push_back(std::vector<int>(numEntries, 0));
		erosionBrushWeights.push_back(std::vector<float>(numEntries, 0));
		
		

		for(int j = 0; j<numEntries; j++){
			erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * width + xOffsets[j] + centreX;
			erosionBrushWeights[i][j] = weights[j] / weightSum;
				
		}
	}			
}


void erode(std::vector<float> &noiseMap, int height, int width, int numIterations){ //height and width are of noise map
	for(int i = 0; i<numIterations; i++){ //Each randomly placed water droplet
		
		if (erosionBrushIndices.empty() || currentErosionRadius != erosionRadius || currHeight != height || currWidth!=width) {
        	initializeBrushIndices (height, width, erosionRadius);
        	currentErosionRadius = erosionRadius;
        	currHeight = height;
        	currWidth = width;
    	}

		float posX = (float)(rand() % (100 * width)) / 100.0f;  //Get random X position
		float posY = (float)(rand() % (100 * height)) / 100.0f;	//Get random Y position

		float dirX = 0;
		float dirY = 0;
		float speed = initialSpeed;
		float water = initialWaterVolume;
		float sediment = 0;

		for(int j = 0; j<maxDropletLifetime; j++){ //A drop's lifetime
			
			int nodeX = (int)posX;
			int nodeY = (int)posY;

			std::cout<< posX << " " << posY <<std::endl;

			int dropletIndex = (nodeY * width) + nodeX; //Index in noiseMap
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posY - nodeY;
			
			//Order of values in vector - Height, gradientX and gradientY
			std::vector<float> heightAndGradient = getHeightAndGradient(noiseMap, height, width, posX, posY);

			std::cout<< "9 "<<std::endl;
			//Update drop's direction and direction
			dirX = dirX * inertia - heightAndGradient[1] * (1-inertia);
			dirY = dirY * inertia - heightAndGradient[2] * (1-inertia);

			std::cout<< "25 "<< dirX << std::endl;
			std::cout<< "26 "<< dirY << std::endl;
			
			//Normalize direction
			float dirLen = sqrt(dirX*dirX + dirY*dirY);

			//Check for divide by 0 exception
			if(dirLen!=0){
				dirX = dirX / dirLen;
				dirY = dirY / dirLen;
			}

			std::cout<< "23 "<< posX << std::endl;
			std::cout<< "24 "<< posY << std::endl;

			posX = posX + dirX;
			posY = posY + dirY;

			std::cout<< "21 "<< posX << std::endl;
			std::cout<< "22 "<< posY << std::endl;
			
			//Check if drop should stop moving or has flowed over the edge of the map
			if((dirX == 0 && dirY == 0) || (posX < borderSize) || (posY <borderSize) || (posX >= width-borderSize) || (posY >= height-borderSize)){
				break;
			}

			std::cout<< "10 "<<std::endl;
			//Else find drop's new height and calculate the difference
			float newHeight = getHeightAndGradient(noiseMap, height, width, posX, posY)[0];
			std::cout<< "12 "<<std::endl;
			float deltaHeight = newHeight - heightAndGradient[0];
			
			std::cout<< "11 "<<std::endl;
			//Calculate drop's sediment capacity - Check more about this
			float sedimentCapacity = std::max(-deltaHeight*speed*water*sedimentCapacityFactor, minSedimentCapacity);
			std::cout << "speed " << speed <<std::endl;
			std::cout << "sedimentCapacity " << sedimentCapacity << " " << deltaHeight << " " << speed << " " << water << " " << sedimentCapacityFactor << " " << minSedimentCapacity <<std::endl;
			std::cout<< "7 "<<std::endl;
			//If carrying more sediment than capacity or if flowing uphill
			if((sediment > sedimentCapacity) || (deltaHeight > 0)){
				//If moving uphill, fill up current height or deposit fraction of excess of sediment
				float amountToDeposit;
				if(deltaHeight > 0){
					amountToDeposit = std::min(deltaHeight, sediment);
				}
				else{
					amountToDeposit = (sediment - sedimentCapacity) * depositSpeed;
				}
				sediment = sediment - amountToDeposit;

				//Add sediment to four nodes using bilinear interpolation
				noiseMap[dropletIndex] = noiseMap[dropletIndex] + (amountToDeposit*(1- cellOffsetX)*(1-cellOffsetY));
				noiseMap[dropletIndex + 1] = noiseMap[dropletIndex + 1] + (amountToDeposit*(cellOffsetX)*(1-cellOffsetY));
				noiseMap[dropletIndex + width] = noiseMap[dropletIndex + width] + (amountToDeposit*(1- cellOffsetX)*(cellOffsetY));
				noiseMap[dropletIndex + width + 1] = noiseMap[dropletIndex + width + 1] + (amountToDeposit*(cellOffsetX)*(cellOffsetY));
				std::cout << "noiseMap " << noiseMap[dropletIndex] << " " << noiseMap[dropletIndex + 1] << " " << noiseMap[dropletIndex + width] << " " << noiseMap[dropletIndex + width + 1] << std::endl;
					
			}

			else{
				//Erode a fraction of the droplet's current carry capacity
				//Clamp erosion to change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::min((sedimentCapacity - sediment)*erodeSpeed, -deltaHeight); 
				std::cout << "amountToErode " << amountToErode << " " << sedimentCapacity << " " <<  sediment << " " << erodeSpeed << " " << deltaHeight << std::endl;
				//float amountToErode = 1000.0f;


				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
                for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndices[dropletIndex].size(); brushPointIndex++) {
                    int nodeIndex = erosionBrushIndices[dropletIndex][brushPointIndex];
                    float weighedErodeAmount = amountToErode * erosionBrushWeights[dropletIndex][brushPointIndex];
                    float deltaSediment = 0.0f;
                    
                    std::cout<< "nodeIndex " << noiseMap[nodeIndex] <<std::endl;
                    if(noiseMap[nodeIndex] < weighedErodeAmount){
                    	deltaSediment = noiseMap[nodeIndex];
                    	std::cout<< "1 " << deltaSediment<<std::endl;
                    }
                    else{
                    	deltaSediment = weighedErodeAmount;
                    	std::cout<< "2 " << deltaSediment<<std::endl;
                    	std::cout<< "32 " << amountToErode<<std::endl;
                    	std::cout<< "33 " << erosionBrushWeights[dropletIndex][brushPointIndex]<<std::endl;
                    }

                    //std::cout<<deltaSediment<<std::endl;
                    noiseMap[nodeIndex] = noiseMap[nodeIndex] - deltaSediment;
                    sediment = sediment + deltaSediment;
                    std::cout<< "3 " <<std::endl;
                }
			}

			// Update droplet's speed and water content
			std::cout<< "deltaHeight "<< deltaHeight << std::endl;
			std::cout<< "speed "<< speed << std::endl;
            std::cout<< "speed*speed "<< speed*speed << std::endl;
            std::cout<< "gravity "<< gravity << std::endl;
            
            speed = std::max(0.0f, sqrt (speed * speed + deltaHeight * gravity));
            std::cout<< "speed2 "<< speed << std::endl;
            std::cout<< "speed*speed2 "<< speed*speed << std::endl;
            
            water = water * (1 - evaporateSpeed);
            std::cout<< "5 "<<std::endl;
		}
	}
}


#endif