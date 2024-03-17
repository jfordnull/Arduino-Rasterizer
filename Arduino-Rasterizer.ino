#include <TFT_HX8357.h>

TFT_HX8357 display = TFT_HX8357();

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF
#define CUBE_V   8
#define CUBE_T   12

struct Cube {
  float V[8][3] = {
    {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0},
    {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}
  };
  int T[12][4] = {
    {0, 1, 2, GREEN}, {0, 2, 3, GREEN}, {4, 0, 3, GREEN}, {4, 3, 7, GREEN},
    {5, 4, 7, GREEN}, {5, 7, 6, GREEN}, {1, 5, 6, GREEN}, {1, 6, 2, GREEN},
    {4, 5, 1, GREEN}, {4, 1, 0, GREEN}, {2, 6, 7, GREEN}, {2, 7, 3, GREEN}
  };
}; 

int renderedTris[12][6];
float pos[3] = {0,5,10};
float scale[3] = {2,2,2};
float eulerAngles[3] = {0,0,0};
Cube cubeModel;

void setup() {
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
  RenderObject(cubeModel.V, cubeModel.T, pos, scale, eulerAngles);
}

void loop() {
  int l_stickY = analogRead(A0);
  int l_stickX = analogRead(A1);
  int r_stickY = analogRead(A2);
  int r_stickX = analogRead(A3);
  if (l_stickX > 600 || l_stickX < 200 || l_stickY > 600 || l_stickY < 200 ||
      r_stickX > 600 || r_stickX < 200 || r_stickY > 600 || r_stickY < 200){
        eulerAngles[0] += (r_stickY < 200) ? 0.05 : (r_stickY > 600) ? -0.05 : 0.0;
        eulerAngles[1] += (r_stickX < 200) ? 0.05 : (r_stickX > 600) ? -0.05 : 0.0;
        pos[0] += (l_stickX < 200) ? -0.15 : (l_stickX > 600) ? 0.15 : 0.0;
        pos[2] += (l_stickY < 200) ? -0.15 : (l_stickY > 600) ? 0.15 : 0.0;
        EraseRenderedTriangles();
        RenderObject(cubeModel.V, cubeModel.T, pos, scale, eulerAngles);
      }
}

void RenderObject(float V[][3], int T[][4], float transform[], float scale[], float eulerAngles[]){
  float projectionMap[CUBE_V][2];
  float u = eulerAngles[0];
  float v = eulerAngles[1];
  float w = eulerAngles[2];
  float rotationMatrix[3][3] = {
    {cos(v)*cos(w),sin(u)*sin(v)*cos(w)-cos(u)*sin(w),sin(u)*sin(w)+cos(u)*sin(v)*cos(w)},
    {cos(v)*sin(w),cos(u)*cos(w)+sin(u)*sin(v)*sin(w),cos(u)*sin(v)*sin(w)-sin(u)*cos(w)},
    {-sin(v),sin(u)*cos(v),cos(u)*cos(v)}};
  for (int v = 0; v < CUBE_V; v++){
    float transformed[3] = {0,0,0};
    for (int i = 0; i < 3; i++){
      for (int j = 0; j < 3; j++){
        transformed[i] += (V[v][j] * scale[j]) * rotationMatrix[j][i];
      }
      transformed[i] += transform[i];
    }
    float* result = ProjectVertex(transformed);
    projectionMap[v][0] = result[0];
    projectionMap[v][1] = result[1];
    delete[] result;
  }
  for (int t = 0; t < CUBE_T; t++){
    int k = 0;
    for (int i = 0; i < 3; i++){
      for (int j = 0; j < 2; j++){
        renderedTris[t][k] = projectionMap[T[t][i]][j];
        k++;
      }
    }
    RenderTriangle(renderedTris[t], T[t][3]);
  }
}

void RenderTriangle(int t[], int color) {
  display.drawTriangle(t[0],t[1],t[2],t[3],t[4],t[5], color);
}

float* ProjectVertex(float v[]) {
  float* result = new float[2];
  result[0] = (v[0] / v[2]) * (display.width() / 2) + display.width() / 2;
  result[1] = (-v[1] / v[2]) * (display.height() / 2) - display.width() / 2;
  return result;
}

void EraseRenderedTriangles(){
  for (int t = 0; t < CUBE_T; t++){
    RenderTriangle(renderedTris[t], TFT_BLACK);
  }
}