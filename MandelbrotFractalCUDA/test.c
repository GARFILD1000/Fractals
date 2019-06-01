#include <stdio.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define WIDTH 1280
#define HEIGHT 1024
#define VOLUME 3932160
#define ITERATIONS 24000

#define COLWIDTH 666
#define COLOFFSET 3200
#define BLACK 0

#define QCENTER 0.131825890901
#define PCENTER -0.743643900055

//Хидер bmp файла
char head[]={0x42, 0x4D, 0x36, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


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

void setSpeed(char *results, double range, int deviceIndex){
    int i = deviceIndex%WIDTH;
    int j = deviceIndex/WIDTH;
    double Pmax=PCENTER+range/2;
    double Pmin=PCENTER-range/2;
    double Qmin=QCENTER-range/2.5;
    double dp=range/(WIDTH-1);
    double q=Qmin+j*dp;
    double p=Pmin+i*dp;
    double x, y, x2, y2, xtemp;
    int n;
    
    printf("i = %d, j = %d | p = %f, q = %f\n", i, j, p, q);
    
    double cardio=sqrt((double)((p-0.25)*(p-0.25)+q*q));
    if (p<(cardio-2*cardio*cardio+0.25)||(p+1)*(p+1)+q*q<0.0625){
        results[(j*WIDTH + i)*3] = BLACK;
        results[(j*WIDTH + i)*3+1] = BLACK;
        results[(j*WIDTH + i)*3+2] = BLACK;
        printf("Cardio!\n");
    }
    else{
        printf("Calculating speed...");
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
        printf("n = %d\n",n);
        if (n==ITERATIONS) 
        {
          results[(j*WIDTH + i)*3] = BLACK;
          results[(j*WIDTH + i)*3+1] = BLACK;
          results[(j*WIDTH + i)*3+2] = BLACK;
        }
        else 
        { 
          n=(n+COLOFFSET)%(3*COLWIDTH);
          if (n/COLWIDTH==0)
          { // Голубой 116.11.0          
            results[(j*WIDTH + i)*3] = 116+139*n/COLWIDTH;
            results[(j*WIDTH + i)*3+1] = 11+244*n/COLWIDTH;
            results[(j*WIDTH + i)*3+2] = 237*n/COLWIDTH;
          }
          else if (n/COLWIDTH==1)
          { // Белый 255.255.237
            n-=COLWIDTH;
            results[(j*WIDTH + i)*3] = 255-253*n/COLWIDTH;
            results[(j*WIDTH + i)*3+1] = 255-123*n/COLWIDTH;
            results[(j*WIDTH + i)*3+2] = 238;
          }
          else 
          { // Рыжий 2.132.239
            n-=2*COLWIDTH;
            results[(j*WIDTH + i)*3] = 2+114*n/COLWIDTH;
            results[(j*WIDTH + i)*3+1] = 132-121*n/COLWIDTH;
            results[(j*WIDTH + i)*3+2] = 239-239*n/COLWIDTH;
          }
        }
    }
    return;
}


void frame(double range)
{
    char *results = calloc(sizeof(char), VOLUME);
    for(int i = 0; i < WIDTH * HEIGHT; i++){
        setSpeed(results, range, i);
    }
    
    
    char numberInString[100];
    snprintf(numberInString, 100, "Output %f", range);
    saveData(numberInString,results);
    return;
}


int main()
{
    printf("Init array\n");
    double range = 0.00000005;
    int num = 0;
    //while(range > 0.00000005){
        num++;
        frame(range);
        //range /= 2;
    //}
  // FPS 
  return 0;
}

