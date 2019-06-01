#include <stdio.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <complex.h>
//Размеры и объем изображения

#define WIDTH 1280
#define HEIGHT 1024
#define VOLUME 3932160


//size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream);
clock_t clock(void);
double sqrt(double num);
void frame(double pcenter, double qcenter, double range, char *results);
void saveData(char fileName[], char *results);

FILE *fopen(const char *fname, const char *mode);
double Pmax, Pmin, Qmin;
//Хидер bmp файла
char head[]={0x42, 0x4D, 0x36, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int main()
{
  printf("Init array\n");
  char *results = calloc(sizeof(char), VOLUME);
  double range, xcenter, ycenter;
  
  //range = 2;
    range = 0.00000006;
    xcenter=-0.743643900055;
    ycenter=0.131825890901;
    int num = 0;
    char numberInString[100];
    while(range > 0.00000005){
        num++;
        frame(xcenter, ycenter, range, results);
        snprintf(numberInString, 100, "Output %d", num);
        saveData(numberInString,results);
        range /= 2;
    }
  // FPS 
  printf("clock %.4f\n", 1.0*CLOCKS_PER_SEC/(1.0*(clock())));
  return 0;
}


void saveData(char fileName[],char *results){
    printf("Creating file %s\n", fileName);
    FILE *file = fopen(fileName, "w");
    
    if(file == NULL){
        return;
    }
    double intensity;
    fwrite(head, 1, 54, file);
    fwrite(results, 1, VOLUME, file);
    fclose(file);
}

int getSpeed(_Complex z0, int iterations)
{
    _Complex double z = z0;
    //static double n, x, y, x2, y2, xtemp;
    //n = x = y = x2 = y2 = 0;
    int speed = 0;
    while(speed < iterations){
        if (cabs(z) > 2){
            break;
        }
        z = z  * z  +  z0;
    }
    return 0;
}


void frame(double pcenter, double qcenter, double range, char *results)
{
    double dp, p, q, cardio;
    register double x, y, x2, y2, xtemp;
    register int i,j,n;
    int colwidth=666;
    int coloffset=3200;
    int nmax=24000;
    long int black=0;
  
    Pmax=pcenter+range/2;
    Pmin=pcenter-range/2;
    Qmin=qcenter-range/2.5;
    dp=range/(WIDTH-1);
    for (j=0;j<HEIGHT;j++)
    {
  	 q=Qmin+j*dp;
        for (i=0;i<WIDTH;i++){
      p=Pmin+i*dp;
      cardio=sqrt((double)((p-0.25)*(p-0.25)+q*q));
      if (p<(cardio-2*cardio*cardio+0.25)||(p+1)*(p+1)+q*q<0.0625)
        {
            results[(j*WIDTH + i)*3] = black;
            results[(j*WIDTH + i)*3+1] = black;
            results[(j*WIDTH + i)*3+2] = black;
        }
      else
      {
        _Complex z0 = p + I * q;
        n = getSpeed(z0 ,nmax);
        /*n = x = y = x2 = y2 = 0;
        while(n < nmax){
        if (x2 + y2 > 4)
            break;
            xtemp=x2-y2+p;
            y=2*x*y+q;
            x=xtemp;
            x2=x*x;
            y2=y*y;
            n++;
        }
        */
        if (n==nmax) 
        {
          results[(j*WIDTH + i)*3] = black;
          results[(j*WIDTH + i)*3+1] = black;
          results[(j*WIDTH + i)*3+2] = black;
        }
        else 
        { 
          n=(n+coloffset)%(3*colwidth);
          if (n/colwidth==0)
          { // Голубой 116.11.0          
            results[(j*WIDTH + i)*3] = 116+139*n/colwidth;
            results[(j*WIDTH + i)*3+1] = 11+244*n/colwidth;
            results[(j*WIDTH + i)*3+2] = 237*n/colwidth;
          }
          else if (n/colwidth==1)
          { // Белый 255.255.237
            n-=colwidth;
            results[(j*WIDTH + i)*3] = 255-253*n/colwidth;
            results[(j*WIDTH + i)*3+1] = 255-123*n/colwidth;
            results[(j*WIDTH + i)*3+2] = 238;
          }
          else 
          { // Рыжий 2.132.239
            n-=2*colwidth;
            results[(j*WIDTH + i)*3] = 2+114*n/colwidth;
            results[(j*WIDTH + i)*3+1] = 132-121*n/colwidth;
            results[(j*WIDTH + i)*3+2] = 239-239*n/colwidth;
          }
        }
        
    }
  }
  }  
  return;
}
