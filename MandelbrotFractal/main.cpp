#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <cmath>
#include <complex.h>
#include <list>

#define W 500
#define H 500

int windowW = 1, windowH = 1;
int X0 = (3*W/4);
int Y0 = (H/2);
double L = 320.0;
bool computed;
bool computing;
int iterations = 4095;

class Palette{
public:
    unsigned int r;
    unsigned int g;
    unsigned int b;
    Palette(int r, int g, int b){
        this->r = static_cast<unsigned int>(r);
        this->g = static_cast<unsigned int>(g);
        this->b = static_cast<unsigned int>(b);
    }
    Palette(){
    }
};
Palette *palette = new Palette[iterations];

int get_gray_color(_Complex double z0)
{
    _Complex double z = z0;
    for (int gray = iterations; gray; gray--) {
        if (cabs(z) > 2)
            return gray;
        /*_Complex double _z = creal(z) - I * cimag(z);
        z = _z  * _z  +  z0;
        */
        z = z * z + z0;
    }
    return 0;
}

void compute(sf::Image *buffer){
        computing = true;
        std::cout << "Start computing... ";

    auto start = std::chrono::steady_clock::now();
        #pragma omp parallel for
        for (int i = 0; i < windowW; i++) {
            for (int j = 0; j < windowH; j++) {
                double x = (i - X0) / L;
                double y = (j - Y0) / L;
                double cardio=sqrt((x-0.25)*(x-0.25)+y*y);
                if (x < (cardio-2*cardio*cardio+0.25)||(x+1)*(x+1)+y*y<0.0625){
                    buffer->setPixel(i, j, sf::Color(palette[0].r, palette[0].g, palette[0].b));
                }
                else {
                    _Complex double z = x + I * y;
                    int gray = get_gray_color(z);
                    buffer->setPixel(i, j, sf::Color(palette[gray].r, palette[gray].g, palette[gray].b));
                }
            }
        }

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);


        std::cout << "Computed for " << elapsed.count()/1000.0 << " seconds! Iterations = " << iterations <<  std::endl;
        computing = false;
        computed = true;
}

void draw(sf::Image *buffer){

}

void setPalette(){
    unsigned char red[] = {0, 15, 36, 54, 73, 90, 100 ,109, 125, 146, 164, 182, 203, 219, 232, 255};
    unsigned char *green = red;
    unsigned char *blue = red;
    int m = 0;
    for (int i = 0; i < 16 && m < iterations; i++)
        for (int j = 0; j < 16 &&  m < iterations; j++)
            for (int k = 0; k < 16 && m < iterations; k++) {
                palette[m].r = red[i];
                palette[m].g = green[j];
                palette[m++].b = blue[k];
            }
}

void compute2(sf::Image *buffer){
    int colwidth=666;
    int coloffset=3200;
    int nmax=24000;
    computing = true;
    std::cout << "Start computing... ";
    auto start = std::chrono::steady_clock::now();

    #pragma omp parallel for
    for (int i=0;i<windowW;i++){
        for (int j=0;j<windowH;j++){
            double p, q, cardio;
            int n;
            p=(i-X0)/L;
            q=(j-Y0)/L;
            cardio=sqrt((p-0.25)*(p-0.25)+q*q);
            if (p<(cardio-2*cardio*cardio+0.25)||(p+1)*(p+1)+q*q<0.0625)
            {
                buffer->setPixel(i,j,sf::Color(0,0,0));
            }
            else

            {   double x, y, x2, y2, xtemp;
                x=y=x2=y2=n=0;
                while (x2+y2<4&&n<nmax)
                {
                    xtemp=x2-y2+p;
                    y=2*x*y+q;
                    x=xtemp;
                    x2=x*x;
                    y2=y*y;
                    n++;
                }
                if (n==nmax)
                {
                    buffer->setPixel(i,j,sf::Color(0,0,0));
                }
                else
                {
                    n=(n+coloffset)%(3*colwidth);
                    if (n/colwidth==0){
                        buffer->setPixel(i,j,sf::Color(237*n/colwidth,11+244*n/colwidth,116+139*n/colwidth));
                    }
                    else if (n/colwidth==1){
                        n-=colwidth;
                        buffer->setPixel(i,j,sf::Color(238,255-123*n/colwidth,255-253*n/colwidth));
                    }
                    else{
                        n-=2*colwidth;
                        buffer->setPixel(i,j,sf::Color(239-239*n/colwidth,132-121*n/colwidth,2+114*n/colwidth));
                    }
                }
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);


    std::cout << "Computed for " << elapsed.count()/1000.0 << " seconds! Iterations = " << nmax <<  std::endl;
    computing = false;
    computed = true;
}

int main()
{
    windowW *= static_cast<int>(W);
    windowH *= static_cast<int>(H);
    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(windowW, windowH), "Fractals");
    sf::Image buffer;
    buffer.create(windowW,windowH,sf::Color::Black);
    sf::Texture texture;
    texture.create(windowW, windowH);
    sf::Sprite bufferSprite;
    std::list<sf::Image> imagesList;
    imagesList.push_back(buffer);
    setPalette();
    bool pushed = false;
    computed = false;
    computing = false;
    sf::Thread computingThread(&compute2, &buffer);
    sf::Thread drawingThread(&draw, &buffer);

    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window->close();
            if (event.type == sf::Event::MouseButtonPressed){
                if (event.key.code == sf::Mouse::Left){
                    sf::Vector2f mousePosition = window->mapPixelToCoords(sf::Mouse::getPosition(*((sf::Window*)window)));
                    if (!computing) {
                        X0 += X0 - mousePosition.x;
                        Y0 += Y0 - mousePosition.y;
                        L = L * 2;

                        computed = false;
                    }
                }
                if (event.key.code == sf::Mouse::Right){
                    if(!computing) {
                        //L = L / 2;
                        //computed = false;
                        if (imagesList.size() > 2) {
                            imagesList.pop_back();
                        }
                    }
                }
            }
        }
        if (!computing && !computed) {
            computingThread.launch();
            pushed = false;

        }
        else if (computing && !computed){
            texture.update(buffer);
            bufferSprite.setTexture(texture);
            window->draw(bufferSprite);
        }
        else if (computed && !pushed) {
            imagesList.push_back(buffer);
            pushed = true;
        }
        else if (computed && pushed){
            texture.update(imagesList.back());
            bufferSprite.setTexture(texture);
            window->draw(bufferSprite);
        }

            //iterations++;
            //computingThread.launch();
        //}

        window->display();
    }
    computingThread.terminate();
    return 0;
}