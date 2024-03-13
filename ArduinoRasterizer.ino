#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128 // OLED width, pixels
#define SCREEN_HEIGHT 64 // OLED height, pixels

// Useful color hex codes
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

void RenderObject(float V[][3], int T[][4], int sizeV, int sizeT, float position[], float scale, float angle);
void RenderTriangle(int t[], float projected[][2]);
float* ProjectVertex(float v[]);

// Display connected to I2C (SDA, SCL Pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Unit cube := set of vertices, set of tris
struct Cube{
  float V[8][3] =
  {
    {1.0, 1.0, 1.0},
    {-1.0, 1.0, 1.0},
    {-1.0, -1.0, 1.0},
    {1.0, -1.0, 1.0},
    {1.0, 1.0, -1.0},
    {-1.0, 1.0, -1.0},
    {-1.0, -1.0, -1.0},
    {1.0, -1.0, -1.0}
  };
  int T[12][4] =
  {
    {0, 1, 2, BLUE},
    {0, 2, 3, BLUE},
    {4, 0, 3, RED},
    {4, 3, 7, RED},
    {5, 4, 7, GREEN},
    {5, 7, 6, GREEN},
    {1, 5, 6, CYAN},
    {1, 6, 2, CYAN},
    {4, 5, 1, MAGENTA},
    {4, 1, 0, MAGENTA},
    {2, 6, 7, YELLOW},
    {2, 7, 3, YELLOW}
  };
};

float pos[3] = {3, 1.25, 7};
Cube cubeModel;
float angle = 0.0;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // 0x3C = display address
  display.fillScreen(0);
  RenderObject(cubeModel.V, cubeModel.T, 8, 12, pos, 1.0, angle);
}

void loop() {
  delay(10);
  int stickX = analogRead(A1);
  if (stickX > 600){
    angle -= 0.25;
    display.clearDisplay();
    RenderObject(cubeModel.V, cubeModel.T, 8, 12, pos, 1.0, angle);
  }
  if (stickX < 200){
    angle += 0.25;
    display.clearDisplay();
    RenderObject(cubeModel.V, cubeModel.T, 8, 12, pos, 1.0, angle);
  }
}

void RenderObject(float V[][3], int T[][4], int sizeV, int sizeT, float position[], float scale, float angle){
  float projected[sizeV][2];
  //Rotation matrix (rotation around y-axis)
   float rotationMatrix[3][3] = {
    {cos(angle), 0, sin(angle)},
    {0, 1, 0},
    {-sin(angle), 0, cos(angle)}
  };
  for (int v = 0; v < sizeV; v++){
    float rotated[3];
    //Apply rotation to each vertex
    for (int i = 0; i < 3; i++) {
      rotated[i] = 0;
      for (int j = 0; j < 3; j++) {
        rotated[i] += V[v][j] * rotationMatrix[j][i];
      }
    }
    float transformed[3];
    //Apply transform
    for (int i = 0; i < 3; i++){
      transformed[i] = rotated[i] + position[i];
    }
    float* result = ProjectVertex(transformed);
    projected[v][0] = result[0];
    projected[v][1] = result[1];
    delete[] result;
  }
  for (int t = 0; t < sizeT; t++){
    RenderTriangle(T[t], projected);
  }
}

//Draws triangle from projected vertices
void RenderTriangle(int t[], float projected[][2]){
  display.drawTriangle(
    (uint16_t)projected[t[0]][0], 
    (uint16_t)projected[t[0]][1], 
    (uint16_t)projected[t[1]][0], 
    (uint16_t)projected[t[1]][1], 
    (uint16_t)projected[t[2]][0], 
    (uint16_t)projected[t[2]][1], 1); //t[3] = color
    display.display();
}

//Takes in world space x.y.z, returns canvas projection x,y
//Viewport := Vw = 2, Vh = d = 1; FOV = 58
float* ProjectVertex(float v[]){
  // x/z * ScreenW, y/z * ScreenH
  float* result = new float[2];
  result[0] = ((v[0] / v[2]) * SCREEN_WIDTH);
  result[1] = ((v[1] / v[2]) * SCREEN_HEIGHT * 2);
  return result;
}