#include <stdio.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#include <cuda.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/transform.h>
#include <thrust/tuple.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/sequence.h>
#include <thrust/fill.h>

#define WIDTH 1280
#define HEIGHT 1024
#define VOLUME 3932160
#define ITERATIONS 24000

#define COLWIDTH 666
#define COLOFFSET 3200
#define BLACK 0

#define QCENTER 0.131825890901
#define PCENTER -0.743643900055

typedef thrust::tuple<char, char, char, int> tup;

//Хидер bmp файла
char head[]={0x42, 0x4D, 0x36, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct speed_functor{
    double range;
    speed_functor(double _range):range(_range){}
    __host__ __device__ tup operator()(tup point)
    {
        int tupleIndex = thrust::get<3>(point);
        char col1 = 0, col2 = 0, col3 = 0;

        double Pmax = PCENTER + range/2;
        double Pmin = PCENTER - range/2;
        double Qmin = QCENTER - range/2.5;
        double dp = range/(WIDTH-1);
        double q = Qmin + tupleIndex/WIDTH * dp;
        double p = Pmin + tupleIndex%WIDTH * dp;
        double x, y, x2, y2, xtemp;
        int n;
        double cardio=sqrt((double)((p-0.25)*(p-0.25)+q*q));
        if (p<(cardio-2*cardio*cardio+0.25) || (p+1)*(p+1)+q*q<0.0625){
            col1 = BLACK;
            col2 = BLACK;
            col3 = BLACK;
        }
        else{
            n = x = y = x2 = y2 = 0;
            while(n < ITERATIONS){
            if (x2 + y2 > 4)
                break;
                xtemp=x2-y2+p;
                y=2*x*y+q;
                x=xtemp;
                x2=x*x;
                y2=y*y;
                n++;
            }
            if (n==ITERATIONS) 
            {
              col1 = BLACK;
              col2 = BLACK;
              col3 = BLACK;
            }
            else 
            { 
              n=(n+COLOFFSET)%(3*COLWIDTH);
              if (n/COLWIDTH==0)
              { // Голубой 116.11.0          
                col1 = 116+139*n/COLWIDTH;
                col2 = 11+244*n/COLWIDTH;
                col3 = 237*n/COLWIDTH;
              }
              else if (n/COLWIDTH==1)
              { // Белый 255.255.237
                n-=COLWIDTH;
                col1 = 255-253*n/COLWIDTH;
                col2 = 255-123*n/COLWIDTH;
                col3 = 238;
              }
              else 
              { // Рыжий 2.132.239
                n-=2*COLWIDTH;
                col1 = 2+114*n/COLWIDTH;
                col2 = 132-121*n/COLWIDTH;
                col3 = 239-239*n/COLWIDTH;
              }
            }
        }
        return thrust::make_tuple(col1, col2, col3, tupleIndex);
    }
};

bool fileSaving = false;


void saveData(char fileName[],char *results){
    printf("Creating file %s\n", fileName);
    FILE *file = fopen(fileName, "w");
    
    if(file == NULL){
        return;
    }
    fwrite(head, 1, 54, file);
    fwrite(results, 1, VOLUME, file);
    fclose(file);
}

__global__ void setSpeed(char *results, double range){
    int deviceIndex = threadIdx.x + blockDim.x * blockIdx.x;
    double Pmax = PCENTER + range/2;
    double Pmin = PCENTER - range/2;
    double Qmin = QCENTER - range/2.5;
    double dp = range/(WIDTH-1);
    double q = Qmin + deviceIndex/WIDTH * dp;
    double p = Pmin + deviceIndex%WIDTH * dp;
    double x, y, x2, y2, xtemp;
    int n;
    double cardio=sqrt((double)((p-0.25)*(p-0.25)+q*q));
    if (p<(cardio-2*cardio*cardio+0.25) || (p+1)*(p+1)+q*q<0.0625){
        results[deviceIndex*3] = BLACK;
        results[deviceIndex*3+1] = BLACK;
        results[deviceIndex*3+2] = BLACK;
    }
    else{
        n = x = y = x2 = y2 = 0;
        while(n < ITERATIONS){
        if (x2 + y2 > 4)
            break;
            xtemp=x2-y2+p;
            y=2*x*y+q;
            x=xtemp;
            x2=x*x;
            y2=y*y;
            n++;
        }
        if (n==ITERATIONS) 
        {
          results[deviceIndex*3] = BLACK;
          results[deviceIndex*3+1] = BLACK;
          results[deviceIndex*3+2] = BLACK;
        }
        else 
        { 
          n=(n+COLOFFSET)%(3*COLWIDTH);
          if (n/COLWIDTH==0)
          { // Голубой 116.11.0          
            results[deviceIndex*3] = 116+139*n/COLWIDTH;
            results[deviceIndex*3+1] = 11+244*n/COLWIDTH;
            results[deviceIndex*3+2] = 237*n/COLWIDTH;
          }
          else if (n/COLWIDTH==1)
          { // Белый 255.255.237
            n-=COLWIDTH;
            results[deviceIndex*3] = 255-253*n/COLWIDTH;
            results[deviceIndex*3+1] = 255-123*n/COLWIDTH;
            results[deviceIndex*3+2] = 238;
          }
          else 
          { // Рыжий 2.132.239
            n-=2*COLWIDTH;
            results[deviceIndex*3] = 2+114*n/COLWIDTH;
            results[deviceIndex*3+1] = 132-121*n/COLWIDTH;
            results[deviceIndex*3+2] = 239-239*n/COLWIDTH;
          }
        }
    }
    return;
}


double pureCUDA(double range)
{
	cudaEvent_t start, stop;
	float elapsedTime, allTime;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	
    char *results_h;
    char *results_d;
    
    int setSize = 0;
    
    printf("Start pureCUDA fractal calculating...\n");
    
    cudaMalloc((void**)&results_d, sizeof(char)*(VOLUME));
    cudaMallocHost((void**)&results_h, sizeof(char)*(VOLUME));
    
    int threadsPerBlock = 32;
    int numOfBlocks = WIDTH * HEIGHT / threadsPerBlock;
    
    while(range > 0.00000005){
        setSize++;
        cudaEventRecord(start, 0);
        setSpeed <<< dim3(numOfBlocks), dim3(threadsPerBlock) >>> (results_d, range);
        cudaDeviceSynchronize();
        
        
        cudaMemcpy(results_h, results_d, (VOLUME)*sizeof(char), cudaMemcpyDeviceToHost);
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
	    cudaEventElapsedTime(&elapsedTime, start, stop);
        
        printf("Fractal calculated! Time = %f ms.\n", elapsedTime);
        char numberInString[100];
        snprintf(numberInString, 100, "pureCUDA/Output%d", setSize);
        if (fileSaving) saveData(numberInString,results_h);
        allTime += elapsedTime;
        range /= 2;
    }
    
    
    cudaFree(results_d);
    cudaFree(results_h);
    return allTime/setSize;
}


double thrustCUDA(double range)
{
	cudaEvent_t start, stop;
	float elapsedTime, allTime;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	
	thrust::host_vector<char> color1_h(WIDTH*HEIGHT);
	thrust::host_vector<char> color2_h(WIDTH*HEIGHT);
	thrust::host_vector<char> color3_h(WIDTH*HEIGHT);
	thrust::host_vector<int> index_h(WIDTH*HEIGHT);
	
	thrust::device_vector<char> color1_d(WIDTH*HEIGHT);
	thrust::device_vector<char> color2_d(WIDTH*HEIGHT);
	thrust::device_vector<char> color3_d(WIDTH*HEIGHT);
	thrust::device_vector<int> index_d(WIDTH*HEIGHT);
	
	thrust::fill(color1_d.begin(), color1_d.end(), 0);
	thrust::fill(color2_d.begin(), color2_d.end(), 0);
	thrust::fill(color3_d.begin(), color3_d.end(), 0);
	thrust::sequence(index_d.begin(), index_d.end());
	
    char *results = (char*)calloc(sizeof(char), VOLUME);
    int setSize = 0;
    printf("Start thrustCUDA fractal calculating...\n");
     
    while(range > 0.00000005){
    speed_functor speedFunctor(range);
        setSize++;
        cudaEventRecord(start, 0);
        thrust::transform(
            thrust::make_zip_iterator(
                thrust::make_tuple(color1_d.begin(), color2_d.begin(), color3_d.begin(), index_d.begin())
            ),
            thrust::make_zip_iterator(
                thrust::make_tuple(color1_d.end(), color2_d.end(), color3_d.end(),
                    index_d.end())
            ),
            thrust::make_zip_iterator(
                thrust::make_tuple(color1_d.begin(), color2_d.begin(), color3_d.begin(), index_d.begin())
            ),
            speedFunctor
        );
        
        //cudaDeviceSynchronize();
        
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
	    cudaEventElapsedTime(&elapsedTime, start, stop);
	    
        //results_h = results_d;
        
        color1_h = color1_d;
        color2_h = color2_d;
        color3_h = color3_d;
        for(int i = 0; i < WIDTH*HEIGHT; i++){
            results[i*3] = color1_h[i];
            results[i*3+1] = color2_h[i];
            results[i*3+2] = color3_h[i];
        }
        
        printf("Fractal calculated! Time = %f ms.\n", elapsedTime);
        char numberInString[100];
        snprintf(numberInString, 100, "thrustCUDA/Output%d", setSize);
        if (fileSaving) saveData(numberInString,results);
        range /= 2;
        
        allTime+= elapsedTime;
    }
    
    
    //cudaFree(results_d);
    //cudaFree(results_h);
    return allTime/setSize;
}

int main()
{
    printf("Start comparing program\n");
    fileSaving = false;
    double range = 0.000005;
    double pureCudaTime = 0;
    double thrustCudaTime = 0;
    
    thrustCudaTime = thrustCUDA(range);
    pureCudaTime = pureCUDA(range);
    
    printf("Time:\nPure CUDA:%f\nThrust:%f\n", pureCudaTime, thrustCudaTime);
    return 0;
}

