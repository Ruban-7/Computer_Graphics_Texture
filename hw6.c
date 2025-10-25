#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float cameraX = 0.0f;
float cameraY =0.0f;
float cameraZ = 50.0f;
float rotationX= 20.0f;  
float rotationY = -1.0f;   
float fpCameraX = -4.0f;   
float fpCameraY =3.8f;    
float fpCameraZ = 36.9f;   
float fpLookAtX=-3.9f;   
float fpLookAtY =3.9f;   
float fpLookAtZ= 35.9f;  
float fpUpX= 0.0f;       
float fpUpY = 1.0f;       
float fpUpZ = 0.0f;       
float fpYaw = -187.5f;       
float fpPitch = 6.0f;     
int lastMouseX = 400;     
int lastMouseY =300;     
int mouseLookActive= 0;  
float fpMoveSpeed= 0.5f;     
float fpMouseSensitivity =0.3f;  
float fpRotationSpeed=5.0f;     
float zoomScale = 1.0f;  
int obj = 0;   

// Orbital light variables
int orbitalLight= 1;       
int orbitalDistance = 50;   
float orbitalZh = 0.0f;         
float orbitalYlight=10.0f; 
int orbitalMove = 1;      
float orbitalSpeed= 15.0f;
int stadiumAnimationPaused = 0;  
int ticketBoothRoofColor = 0;    
float skyscraperTextureRepeat = 1.0f;  
float treeTrunkHeight = 4.0f;         
float treeLeafScale = 1.0f;               
int currentEffectiveDistance = 50;
int orbitalAmbient = 15;     
int orbitalDiffuse = 50;    
int orbitalSpecular = 40;    
int orbitalShininess = 6;  
float shiny = 64.0f;  
int orbitalSmooth = 1;     

// Texture variables
unsigned int texture[15];  
int mode = 0;
float stadiumTextureOffset = 0.0f;  


// Load texture from BMP file
static unsigned int LoadTexBMP(const char* file)
{
   
   FILE* f =fopen(file,"rb");
   if (!f) return 0;
   
   unsigned short magic;
   if (fread(&magic, 2, 1, f) != 1) 
   {
      fclose(f);
      return 0;
   }
   if (magic != 0x4D42) 
   {
      fclose(f);
      return 0;
   }
   
   unsigned int dx,dy,off,k; 
   unsigned short nbp, bpp;     
   if (fseek(f, 8,SEEK_CUR)||fread(&off, 4, 1, f) != 1 ||
       fseek(f, 4,SEEK_CUR) || fread(&dx, 4, 1, f) != 1 || fread(&dy, 4, 1, f)!= 1||
       fread(&nbp,2, 1,f)!= 1 ||fread(&bpp,2,1, f) !=1 ||fread(&k, 4, 1, f)!= 1)
   {
      fclose(f);
      return 0;
   }
  
   if (dx <1||dy< 1||nbp!=1||bpp!=24||k!= 0)
   {
      fclose(f);
      return 0;
   }

   unsigned int size =3 *dx*dy;
   unsigned char* image =(unsigned char*) malloc(size);
   if (!image) 
   {
      fclose(f);
      return 0;
   }
   
   if (fseek(f,off,SEEK_SET) ||fread(image,size, 1,f)!= 1) 
   {
      free(image);
      fclose(f);
      return 0;
   }
   fclose(f);
   
   for (k =0;k<size;k+= 3)
   {
      unsigned char temp =image[k];
      image[k]=image[k+2];
      image[k+2]=temp;
   }
   
   unsigned int texture;
   glGenTextures(1,&texture);
   glBindTexture(GL_TEXTURE_2D,texture);
   glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, dx,dy, 0,GL_RGB,GL_UNSIGNED_BYTE,image);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
   free(image);
   return texture;
}

// Reset function for orbital light
void resetOrbitalLight(void)
{
    orbitalDistance = 50;    
    orbitalYlight =10.0f;   
    orbitalZh = 0.0f;           
    orbitalMove = 1;         
    orbitalAmbient=15;     
    orbitalDiffuse = 50;     
    orbitalSpecular = 40;    
    orbitalShininess = 6;    
    shiny = 64.0f;   
    orbitalSmooth = 1;       
} 

typedef enum 
{
    VIEW_ORTHOGONAL = 0,
    VIEW_PERSPECTIVE=1,
    VIEW_FIRST_PERSON = 2
} ViewMode;

ViewMode currentViewMode= VIEW_ORTHOGONAL;

void reshape(int width, int height);
void perspective(float fovy, float aspect, float zNear, float zFar);
void lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);
void updateFirstPersonLookAt(void);

const float PI =3.14159265359f;
const int RING_SEGMENTS=32;
const int TORUS_SEGMENTS=16;
const int STADIUM_SEGMENTS = 64;

void perspective(float fovy, float aspect,float zNear, float zFar) 
{
    float fH = tan(fovy/360.0f*PI)*zNear;
    float fW= fH *aspect;
    glFrustum(-fW,fW,-fH, fH, zNear,zFar);
}

void lookAt(float eyeX, float eyeY, float eyeZ,float centerX,float centerY, float centerZ,float upX,float upY, float upZ) {
    float forwardX = centerX - eyeX;
    float forwardY =centerY- eyeY;
    float forwardZ= centerZ -eyeZ;
    float length = sqrt(forwardX *forwardX + forwardY * forwardY+ forwardZ * forwardZ);
    forwardX /= length;
    forwardY /= length;
    forwardZ /=length;
    float rightX = forwardY *upZ - forwardZ * upY;
    float rightY =forwardZ * upX - forwardX * upZ;
    float rightZ= forwardX * upY - forwardY * upX;
    length = sqrt(rightX * rightX +rightY * rightY+ rightZ * rightZ);
    rightX /= length;
    rightY/= length;
    rightZ /= length;
    float actualUpX=rightY *forwardZ -rightZ *forwardY;
    float actualUpY= rightZ *forwardX-rightX* forwardZ;
    float actualUpZ = rightX* forwardY-rightY * forwardX;
    float matrix[16] = {
        rightX,actualUpX,-forwardX,0,
        rightY,actualUpY,-forwardY,0,
        rightZ,actualUpZ,-forwardZ, 0,
        0, 0, 0, 1
    };
    glMultMatrixf(matrix);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

void updateFirstPersonLookAt(void) {
    
    float yawRad = fpYaw*PI/180.0f;
    float pitchRad = fpPitch * PI / 180.0f;

    fpLookAtX = fpCameraX+cos(pitchRad)*sin(yawRad);
    fpLookAtY=fpCameraY + sin(pitchRad);
    fpLookAtZ=fpCameraZ+cos(pitchRad) * cos(yawRad);
}

//simple sphere for light visualization
void drawLightSphere(float x,float y,float z,float radius) {
    glPushMatrix();
    glTranslatef(x,y,z);
    glScalef(radius,radius,radius);
    for (int ph = -90; ph < 90; ph += 10) {
        glBegin(GL_QUAD_STRIP);
        for (int th = 0; th <= 360; th += 20) {
            float x1 = cos(th*PI/180.0f) * cos(ph * PI / 180.0f);
            float y1 = sin(ph *PI/180.0f);
            float z1 = sin(th * PI/ 180.0f)*cos(ph*PI/180.0f);
            float x2 = cos(th * PI/180.0f) * cos((ph + 10) * PI / 180.0f);
            float y2 = sin((ph+10) * PI/180.0f);
            float z2 = sin(th*PI / 180.0f) * cos((ph + 10) * PI / 180.0f);
            
            glNormal3f(x1,y1, z1);
            glVertex3f(x1,y1,z1);
            glNormal3f(x2, y2,z2);
            glVertex3f(x2,y2, z2);
        }
        glEnd();
    }
    glPopMatrix();
}

void drawAxes(float length) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_LINES);

    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);
    
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);
    
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, length);
    
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(length + 1.0f, 0.0f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'X');
    glRasterPos3f(0.0f, length + 1.0f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Y');
    glRasterPos3f(0.0f, 0.0f, length + 1.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Z');
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void drawSoccerGoalPost(float x, float z, float height, float width) {
    glPushMatrix();
    
    // Set material properties for lighting
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[]= {0.0f, 0.0f, 0.0f, 1.0f};
    float goalWhite[] ={1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, goalWhite);
    
    // Left post (vertical)
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x-0.05f, 0.0f, z - width/2.0f);
    glVertex3f(x+0.05f,0.0f, z - width/2.0f);
    glVertex3f(x+0.05f, height, z - width/2.0f);
    glVertex3f(x-0.05f, height, z - width/2.0f);
    // Back face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x-0.05f, 0.0f, z - width/2.0f);
    glVertex3f(x-0.05f,height, z - width/2.0f);
    glVertex3f(x+0.05f, height, z - width/2.0f);
    glVertex3f(x+0.05f,0.0f, z - width/2.0f);
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x-0.05f, 0.0f, z - width/2.0f);
    glVertex3f(x-0.05f, 0.0f, z - width/2.0f);
    glVertex3f(x-0.05f, height, z - width/2.0f);
    glVertex3f(x-0.05f, height, z - width/2.0f);
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x+0.05f, 0.0f, z - width/2.0f);
    glVertex3f(x+0.05f,height, z - width/2.0f);
    glVertex3f(x+0.05f, height, z - width/2.0f);
    glVertex3f(x+0.05f,0.0f, z - width/2.0f);
    glEnd();
    
    // Right post (vertical)
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x-0.05f, 0.0f, z + width/2.0f);
    glVertex3f(x+0.05f,0.0f, z + width/2.0f);
    glVertex3f(x+0.05f, height, z + width/2.0f);
    glVertex3f(x-0.05f,height, z + width/2.0f);
    // Back face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x-0.05f, 0.0f, z + width/2.0f);
    glVertex3f(x-0.05f,height, z + width/2.0f);
    glVertex3f(x+0.05f, height, z + width/2.0f);
    glVertex3f(x+0.05f,0.0f, z + width/2.0f);
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x-0.05f, 0.0f, z + width/2.0f);
    glVertex3f(x-0.05f,height, z + width/2.0f);
    glVertex3f(x-0.05f, height, z + width/2.0f);
    glVertex3f(x-0.05f,0.0f, z + width/2.0f);
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x+0.05f, 0.0f, z+ width/2.0f);
    glVertex3f(x+0.05f,height, z + width/2.0f);
    glVertex3f(x+0.05f, height, z + width/2.0f);
    glVertex3f(x+0.05f,0.0f, z + width/2.0f);
    glEnd();
    
    // Crossbar (horizontal)
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x-0.05f,height, z - width/2.0f);
    glVertex3f(x+0.05f,height, z - width/2.0f);
    glVertex3f(x+0.05f, height, z + width/2.0f);
    glVertex3f(x-0.05f,height, z + width/2.0f);
    // Back face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x-0.05f,height, z - width/2.0f);
    glVertex3f(x-0.05f,height,z+width/2.0f);
    glVertex3f(x+0.05f, height, z+ width/2.0f);
    glVertex3f(x+0.05f,height, z - width/2.0f);
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x-0.05f, height, z-width/2.0f);
    glVertex3f(x+0.05f,height, z-width/2.0f);
    glVertex3f(x+0.05f,height, z+width/2.0f);
    glVertex3f(x-0.05f,height, z + width/2.0f);
    // Bottom face
    glNormal3f(0.0f,-1.0f,0.0f);
    glVertex3f(x-0.05f,height, z-width/2.0f);
    glVertex3f(x-0.05f, height,z+width/2.0f);
    glVertex3f(x+0.05f,height,z+ width/2.0f);
    glVertex3f(x+0.05f, height, z-width/2.0f);
    glEnd();
    // Goal net 
    glColor3f(0.8f,0.8f,0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i<=8;i++) 
    {
        float y =(height*i)/8.0f;
        glVertex3f(x,y,z-width/2.0f);
        glVertex3f(x,y,z+width/2.0f);
    }
    for (int i=0; i<=12;i++)
     {
        float zPos = z-width/2.0f +(width *i)/12.0f;
        glVertex3f(x,0.0f,zPos);
        glVertex3f(x, height,zPos);
    }
    glEnd();
    
    glPopMatrix();
}

void drawSoccerFieldMarkings(float fieldLength, float fieldWidth) {
    glColor3f(1.0f,1.0f, 1.0f);
    float halfLength = fieldLength / 2.0f;
    float halfWidth =fieldWidth /2.0f;
    
    glBegin(GL_LINES);
    glVertex3f(-halfLength, 0.01f, -halfWidth);
    glVertex3f(halfLength,0.01f, -halfWidth);
    glVertex3f(-halfLength, 0.01f, halfWidth);
    glVertex3f(halfLength, 0.01f, halfWidth);
    glVertex3f(-halfLength,0.01f, -halfWidth);
    glVertex3f(-halfLength, 0.01f,halfWidth);
    glVertex3f(halfLength, 0.01f, -halfWidth);
    glVertex3f(halfLength,0.01f, halfWidth);
    glVertex3f(0.0f,0.01f, -halfWidth);
    glVertex3f(0.0f, 0.01f,halfWidth);
    
    float centerCircleRadius = fieldWidth * 0.15f;
    for (int i = 0; i< 64; i++) {
        float angle1= 2.0f *PI *i/ 64;
        float angle2 =2.0f *PI *(i + 1)/64;
        glVertex3f(centerCircleRadius*cos(angle1), 0.01f, centerCircleRadius* sin(angle1));
        glVertex3f(centerCircleRadius *cos(angle2), 0.01f, centerCircleRadius *sin(angle2));
    }
    
    float goalAreaLength =fieldLength * 0.15f;
    float goalAreaWidth = fieldWidth* 0.25f;
    
    glVertex3f(-halfLength, 0.01f, -goalAreaWidth/2.0f);
    glVertex3f(-halfLength+ goalAreaLength,0.01f,-goalAreaWidth/2.0f);
    glVertex3f(-halfLength, 0.01f, goalAreaWidth/2.0f);
    glVertex3f(-halfLength + goalAreaLength, 0.01f,goalAreaWidth/2.0f);
    glVertex3f(-halfLength+goalAreaLength,0.01f, -goalAreaWidth/2.0f);
    glVertex3f(-halfLength+goalAreaLength,0.01f, goalAreaWidth/2.0f);
    glVertex3f(halfLength, 0.01f, -goalAreaWidth/2.0f);
    glVertex3f(halfLength - goalAreaLength,0.01f, -goalAreaWidth/2.0f);
    glVertex3f(halfLength, 0.01f, goalAreaWidth/2.0f);
    glVertex3f(halfLength -goalAreaLength, 0.01f,goalAreaWidth/2.0f);
    glVertex3f(halfLength- goalAreaLength,0.01f,-goalAreaWidth/2.0f);
    glVertex3f(halfLength -goalAreaLength, 0.01f, goalAreaWidth/2.0f);
    
    float penaltyAreaLength= fieldLength * 0.25f;
    float penaltyAreaWidth = fieldWidth *0.6f;
    glVertex3f(-halfLength, 0.01f,-penaltyAreaWidth/2.0f);
    glVertex3f(-halfLength + penaltyAreaLength, 0.01f,-penaltyAreaWidth/2.0f);
    glVertex3f(-halfLength, 0.01f,penaltyAreaWidth/2.0f);
    glVertex3f(-halfLength +penaltyAreaLength, 0.01f, penaltyAreaWidth/2.0f);
    glVertex3f(-halfLength + penaltyAreaLength, 0.01f, -penaltyAreaWidth/2.0f);
    glVertex3f(-halfLength+ penaltyAreaLength,0.01f, penaltyAreaWidth/2.0f);
    glVertex3f(halfLength, 0.01f, -penaltyAreaWidth/2.0f);
    glVertex3f(halfLength -penaltyAreaLength, 0.01f, -penaltyAreaWidth/2.0f);
    glVertex3f(halfLength, 0.01f, penaltyAreaWidth/2.0f);
    glVertex3f(halfLength -penaltyAreaLength, 0.01f,penaltyAreaWidth/2.0f);
    glVertex3f(halfLength- penaltyAreaLength, 0.01f, -penaltyAreaWidth/2.0f);
    glVertex3f(halfLength -penaltyAreaLength,0.01f,penaltyAreaWidth/2.0f);
    
    glEnd();
}
void drawStadiumField(float length, float width) {
    glPushMatrix(); 
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float grass[] = {0.2f, 0.8f, 0.2f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, grass);
    
    // Enable texture for football field
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[13]);
    
    // Set texture wrapping to repeat in both directions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Stadium field surface with football field texture
    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    
    // Calculate texture repetition based on field size
    float textureRepeatX = length / 2.0f;
    float textureRepeatZ = width / 2.0f;  
    
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-length/2.0f, -0.01f, -width/2.0f);
    glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(length/2.0f, -0.01f, -width/2.0f);
    glTexCoord2f(textureRepeatX, textureRepeatZ); glVertex3f(length/2.0f, -0.01f, width/2.0f);
    glTexCoord2f(0.0f, textureRepeatZ); glVertex3f(-length/2.0f, -0.01f, width/2.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
    drawSoccerFieldMarkings(length, width);
}

void drawStadiumTier(float centerX, float centerZ, float majorRadius, float minorRadius, int rows, int segments) {
    glPushMatrix();
    float white[] = {1.0f, 1.0f, 1.0f,1.0f};
    float black[] ={0.0f, 0.0f, 0.0f, 1.0f};
    float tierGray[]={0.6f,0.6f,0.6f,1.0f};
    float wallGray[]={0.5f,0.5f,0.5f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,black);
    
    // Enable texture for stadium tier
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[14]);
    
    // Set texture wrapping to repeat in both directions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    float tierThickness=0.2f; 
    
    for (int row =0;row<rows;row++) 
    {
        float currentHeight =  row * 0.3f;
        float currentMajorRadius =majorRadius+ row * 0.8f;
        float currentMinorRadius=minorRadius+row* 0.6f;
        glColor3f(0.6f,0.6f, 0.6f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, tierGray);
        
        // Top surface of tier
        glColor3f(1.0f, 1.0f, 1.0f); 
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, tierGray);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0;i <= segments;i++) {
            float angle = 2.0f *PI*i / segments;
            float x1= centerX+ currentMajorRadius* cos(angle);
            float z1 =centerZ +currentMinorRadius * sin(angle);
            float x2 = centerX +(currentMajorRadius- 0.3f) * cos(angle);
            float z2=centerZ+ (currentMinorRadius-0.3f) *sin(angle);
            
            
            float u =(float)i /(float)segments * 4.0f; 
            float v1 = 0.0f;
            float v2 = 1.0f;
            
            glNormal3f(0.0f,1.0f,0.0f);
            glTexCoord2f(u, v1);
            glVertex3f(x1, currentHeight,z1);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(u, v2);
            glVertex3f(x2, currentHeight,z2);
        }
        glEnd();
        
        // Bottom surface of tier
        glBegin(GL_QUAD_STRIP);
        for (int i = 0;i <=segments;i++) {
            float angle =2.0f* PI * i / segments;
            float x1= centerX+ currentMajorRadius* cos(angle);
            float z1 =centerZ +currentMinorRadius * sin(angle);
            float x2=centerX +(currentMajorRadius- 0.3f) * cos(angle);
            float z2 = centerZ+ (currentMinorRadius -0.3f) *sin(angle);
            float u = (float)i/(float)segments * 4.0f; 
            float v1 = 0.0f;
            float v2 = 1.0f;
            
            // Calculate normals for tier surface (horizontal)
            glNormal3f(0.0f, -1.0f,0.0f);
            glTexCoord2f(u, v1);
            glVertex3f(x1,currentHeight-tierThickness, z1);
            glNormal3f(0.0f, -1.0f, 0.0f);
            glTexCoord2f(u, v2);
            glVertex3f(x2, currentHeight-tierThickness,z2);
        }
        glEnd();
        // Outer edge of tier 
        glBegin(GL_QUAD_STRIP);
        for (int i = 0;i <= segments;i++) {
            float angle = 2.0f * PI * i / segments;
            float x = centerX+ currentMajorRadius* cos(angle);
            float z = centerZ +currentMinorRadius * sin(angle);
            // Calculate normals 
            float nx = cos(angle)/currentMajorRadius;
            float nz = sin(angle)/currentMinorRadius;  
            // Normalize the normal vector
            float length = sqrt(nx*nx+ nz*nz);
            if (length > 0) {
                nx /=length;
                nz/= length;
            }

            float u = (float)i / (float)segments * 4.0f; 
            float v1 = 0.0f;
            float v2 = 1.0f;
            
            glNormal3f(nx, 0.0f, nz);
            glTexCoord2f(u, v1);
            glVertex3f(x,currentHeight -tierThickness, z);
            glNormal3f(nx, 0.0f, nz);
            glTexCoord2f(u, v2);
            glVertex3f(x, currentHeight, z);
        }
        glEnd();
        
        // Inner edge of tier 
        glBegin(GL_QUAD_STRIP);
        for (int i = 0;i <= segments;i++) {
            float angle =2.0f* PI* i/segments;
            float x = centerX +(currentMajorRadius- 0.3f)* cos(angle);
            float z = centerZ+ (currentMinorRadius - 0.3f) *sin(angle);
            // Calculate normals 
            float nx= cos(angle)/(currentMajorRadius - 0.3f);
            float nz = sin(angle)/(currentMinorRadius - 0.3f);
            // Normalize the normal vector
            float length = sqrt(nx* nx +nz * nz);
            if (length > 0) 
            {
                nx /= length;
                nz /= length;
            }
            
            // Calculate texture coordinates for inner edge
            float u =(float)i/(float)segments * 4.0f; 
            float v1 = 0.0f;
            float v2 = 1.0f;
            
            glNormal3f(-nx, 0.0f, -nz);
            glTexCoord2f(u, v1);
            glVertex3f(x,currentHeight-tierThickness, z);
            glNormal3f(-nx, 0.0f, -nz);
            glTexCoord2f(u, v2);
            glVertex3f(x, currentHeight,z);
        }
        glEnd();

        // Vertical wall behind tier 
        glColor3f(1.0f,1.0f,1.0f); 
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE, wallGray);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture[14]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Outer surface of wall
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i<=segments; i++) {
            float angle =2.0f *PI * i/segments;
            float x = centerX +(currentMajorRadius- 0.3f) * cos(angle);
            float z = centerZ + (currentMinorRadius - 0.3f)* sin(angle);
            
            // Calculate normals 
            float nx =cos(angle)/(currentMajorRadius- 0.3f);
            float nz=sin(angle)/(currentMinorRadius-0.3f);
            
            // Normalize the normal vector
            float length = sqrt(nx*nx+ nz*nz);
            if (length > 0) {
                nx /=length;
                nz /=length;
            }
            
            // Calculate texture coordinates for vertical wall
            float u = (float)i / (float)segments * 4.0f; 
            float v1= 0.0f;
            float v2 =2.0f; 
            
            glNormal3f(nx,0.0f,nz);
            glTexCoord2f(u, v1);
            glVertex3f(x,currentHeight,z);
            glNormal3f(nx, 0.0f, nz);
            glTexCoord2f(u, v2);
            glVertex3f(x, currentHeight + 0.4f, z);
        }
        glEnd();
        
        // Inner surface of wall
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i<=segments; i++) {
            float angle = 2.0f *PI * i / segments;
            float x = centerX +(currentMajorRadius - 0.3f -tierThickness) * cos(angle);
            float z= centerZ + (currentMinorRadius -0.3f- tierThickness)* sin(angle);
            
            // Calculate normals 
            float nx =cos(angle)/(currentMajorRadius- 0.3f);
            float nz =sin(angle) /(currentMinorRadius-0.3f);
            
            // Normalize the normal vector
            float length=sqrt(nx*nx + nz*nz);
            if (length > 0)
             {
                nx /= length;
                nz /= length;
            }
            
            // Calculate texture coordinates for inner wall surface
            float u = (float)i/(float)segments *4.0f; 
            float v1 = 0.0f;
            float v2 = 2.0f;
            
            glNormal3f(-nx, 0.0f, -nz);
            glTexCoord2f(u, v1);
            glVertex3f(x,currentHeight, z);
            glNormal3f(-nx, 0.0f, -nz);
            glTexCoord2f(u, v2);
            glVertex3f(x, currentHeight + 0.4f, z);
        }
        glEnd();
        
        // Top surface of wall
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i<=segments; i++) {
            float angle = 2.0f *PI * i / segments;
            float xOuter = centerX +(currentMajorRadius - 0.3f) * cos(angle);
            float zOuter =centerZ + (currentMinorRadius - 0.3f)* sin(angle);
            float xInner= centerX +(currentMajorRadius - 0.3f -tierThickness) * cos(angle);
            float zInner =centerZ +(currentMinorRadius- 0.3f- tierThickness)* sin(angle);
            float u1 = (float)i / (float)segments * 4.0f; 
            float u2 = (float)i / (float)segments * 4.0f;
            float v = 1.0f;
            
            glNormal3f(0.0f,1.0f, 0.0f);
            glTexCoord2f(u1,v);
            glVertex3f(xOuter,currentHeight+0.4f, zOuter);
            glTexCoord2f(u2, v);
            glVertex3f(xInner, currentHeight+0.4f,zInner);
        }
        glEnd();
        
        // Bottom surface of wall
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i<=segments; i++) {
            float angle = 2.0f *PI * i / segments;
            float xOuter = centerX +(currentMajorRadius - 0.3f) * cos(angle);
            float zOuter = centerZ + (currentMinorRadius - 0.3f)* sin(angle);
            float xInner=centerX +(currentMajorRadius - 0.3f - tierThickness) * cos(angle);
            float zInner=centerZ + (currentMinorRadius- 0.3f - tierThickness)* sin(angle);
            
            float u1 = (float)i /(float)segments * 4.0f; 
            float u2 = (float)i / (float)segments*4.0f;
            float v = 0.0f;
            
            glNormal3f(0.0f,-1.0f, 0.0f);
            glTexCoord2f(u1, v);
            glVertex3f(xOuter,currentHeight,zOuter);
            glTexCoord2f(u2, v);
            glVertex3f(xInner,currentHeight,zInner);
        }
        glEnd();
    }
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawLightTower(float x,float z,float height) {
    glPushMatrix();
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float darkGray[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float lightGray[] = {0.4f, 0.4f, 0.4f, 1.0f};
    float platformGray[] = {0.8f, 0.8f, 0.8f, 1.0f};
    
    // Shininess for light tower
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny*1.8f);  
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,lightGray);     
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    
    // Base (dark gray)
    glColor3f(0.3f, 0.3f,0.3f);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,darkGray);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(x- 0.3f, 0.0f, z - 0.3f);
    glVertex3f(x + 0.3f, 0.0f, z- 0.3f);
    glVertex3f(x +0.3f,0.0f, z +0.3f);
    glVertex3f(x - 0.3f, 0.0f,z + 0.3f);
    glEnd();
    
    // Pole with light pole texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[8]);
    glColor3f(0.4f, 0.4f, 0.4f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,lightGray);
    glBegin(GL_QUAD_STRIP);
    
    // Front face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f,0.0f); glVertex3f(x - 0.1f, 0.0f, z - 0.1f);
    glTexCoord2f(0.0f,1.0f); glVertex3f(x- 0.1f,height, z - 0.1f);
    glTexCoord2f(0.25f,0.0f); glVertex3f(x + 0.1f, 0.0f, z - 0.1f);
    glTexCoord2f(0.25f, 1.0f); glVertex3f(x + 0.1f, height,z - 0.1f);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.5f,0.0f);glVertex3f(x + 0.1f, 0.0f, z + 0.1f);
    glTexCoord2f(0.5f,1.0f); glVertex3f(x+ 0.1f, height, z + 0.1f);
    
    // Back face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.75f, 0.0f); glVertex3f(x -0.1f,0.0f, z + 0.1f);
    glTexCoord2f(0.75f,1.0f); glVertex3f(x - 0.1f, height, z + 0.1f);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x - 0.1f, 0.0f, z- 0.1f);
    glTexCoord2f(1.0f,1.0f);glVertex3f(x -0.1f,height,z - 0.1f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    glColor3f(0.8f, 0.8f,0.8f);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,platformGray);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(x -0.4f,height,z - 0.4f);
    glVertex3f(x +0.4f, height, z- 0.4f);
    glVertex3f(x + 0.4f, height,z + 0.4f);
    glVertex3f(x- 0.4f, height, z + 0.4f);
    glEnd();
    
    // Light beams (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    float dirX = -x /sqrt(x*x + z*z);
    float dirZ = -z/sqrt(x*x + z*z);
    for (int i = 0; i < 5; i++) {
        float offset = (i - 2) * 0.2f;
        glVertex3f(x+ offset*dirZ, height,z+offset *dirX);
        glVertex3f(x +offset *dirZ + dirX * 3.0f, height- 1.0f, z+offset* dirX +dirZ *3.0f);
    }
    glEnd();
    glPopMatrix();
}

void drawPalmTree(void) {
    glPushMatrix();
    
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float brown[] = {0.6f, 0.4f, 0.2f, 1.0f};
    float green[] = {0.2f, 0.8f, 0.2f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION, black);
    
    // Trunk with tree bark texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    
    // Set texture wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glColor3f(0.6f,0.4f, 0.2f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, brown);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 16; i++) {
        float angle = 2.0f* PI * i / 16;
        float radius= 0.3f;
        float x1 =radius * cos(angle);
        float z1 = radius *sin(angle);
        float x2 = radius * cos(angle);
        float z2 = radius* sin(angle);

        // Calculate normals 
        float nx = x1 / radius;
        float nz = z1 /radius;
        
        
        float u = (float)i /16.0f*20.0f;
        glTexCoord2f(u, 0.0f);
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x1, 0.0f, z1);
        glTexCoord2f(u, 14.0f * (treeTrunkHeight / 4.0f));  
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x2, treeTrunkHeight, z2);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Leaves with leaf texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[7]);
    glColor3f(0.2f, 0.8f, 0.2f);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,green);
    
    // Leaf 1 (front) 
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.0f,1.0f);  
    glTexCoord2f(0.5f,0.0f); glVertex3f(0.0f,treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f, treeLeafScale);glVertex3f(-0.5f * treeLeafScale,treeTrunkHeight, 2.0f * treeLeafScale);
    glTexCoord2f(treeLeafScale, treeLeafScale); glVertex3f(0.5f * treeLeafScale, treeTrunkHeight,2.0f * treeLeafScale);
    glEnd();
    
    // Leaf 2 (back)
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glTexCoord2f(0.5f,0.0f); glVertex3f(0.0f,treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f, treeLeafScale);glVertex3f(-0.5f * treeLeafScale, treeTrunkHeight,-2.0f * treeLeafScale);
    glTexCoord2f(treeLeafScale,treeLeafScale); glVertex3f(0.5f * treeLeafScale,treeTrunkHeight, -2.0f * treeLeafScale);
    glEnd();
    
    // Leaf 3 (right)
    glBegin(GL_TRIANGLES);
    glNormal3f(1.0f, 0.0f, 0.0f);  
    glTexCoord2f(0.5f,0.0f);glVertex3f(0.0f, treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f, treeLeafScale); glVertex3f(2.0f * treeLeafScale,treeTrunkHeight, -0.5f * treeLeafScale);
    glTexCoord2f(treeLeafScale,treeLeafScale);glVertex3f(2.0f * treeLeafScale, treeTrunkHeight, 0.5f * treeLeafScale);
    glEnd();
    
    // Leaf 4 (left)
    glBegin(GL_TRIANGLES);
    glNormal3f(-1.0f,0.0f, 0.0f);  
    glTexCoord2f(0.5f,0.0f);glVertex3f(0.0f,treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f,treeLeafScale); glVertex3f(-2.0f * treeLeafScale, treeTrunkHeight, -0.5f * treeLeafScale);
    glTexCoord2f(treeLeafScale, treeLeafScale);glVertex3f(-2.0f * treeLeafScale, treeTrunkHeight,0.5f * treeLeafScale);
    glEnd();
    
    // Leaf 5 (front-right) 
    glBegin(GL_TRIANGLES);
    glNormal3f(0.707f, 0.0f, 0.707f);  
    glTexCoord2f(0.5f, 0.0f); glVertex3f(0.0f, treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f, treeLeafScale); glVertex3f(1.4f * treeLeafScale, treeTrunkHeight,1.4f * treeLeafScale);
    glTexCoord2f(treeLeafScale, treeLeafScale); glVertex3f(1.8f * treeLeafScale,treeTrunkHeight, 1.0f * treeLeafScale);
    glEnd();
    
    // Leaf 6 (front-left) 
    glBegin(GL_TRIANGLES);
    glNormal3f(-0.707f, 0.0f, 0.707f);  
    glTexCoord2f(0.5f,0.0f);glVertex3f(0.0f, treeTrunkHeight,0.0f);
    glTexCoord2f(0.0f, treeLeafScale); glVertex3f(-1.4f * treeLeafScale, treeTrunkHeight,1.4f * treeLeafScale);
    glTexCoord2f(treeLeafScale,treeLeafScale); glVertex3f(-1.8f * treeLeafScale,treeTrunkHeight, 1.0f * treeLeafScale);
    glEnd();
    
    // Leaf 7 (back-right) 
    glBegin(GL_TRIANGLES);
    glNormal3f(0.707f, 0.0f, -0.707f);  
    glTexCoord2f(0.5f, 0.0f); glVertex3f(0.0f, treeTrunkHeight, 0.0f);
    glTexCoord2f(0.0f, treeLeafScale);glVertex3f(1.4f * treeLeafScale,treeTrunkHeight, -1.4f * treeLeafScale);
    glTexCoord2f(treeLeafScale,treeLeafScale);glVertex3f(1.8f * treeLeafScale, treeTrunkHeight,-1.0f * treeLeafScale);
    glEnd();
    
    // Leaf 8 (back-left) 
    glBegin(GL_TRIANGLES);
    glNormal3f(-0.707f, 0.0f,-0.707f); 
    glTexCoord2f(0.5f, 0.0f);glVertex3f(0.0f,treeTrunkHeight,0.0f);
    glTexCoord2f(0.0f,treeLeafScale);glVertex3f(-1.4f * treeLeafScale,treeTrunkHeight, -1.4f * treeLeafScale);
    glTexCoord2f(treeLeafScale,treeLeafScale); glVertex3f(-1.8f * treeLeafScale,treeTrunkHeight, -1.0f * treeLeafScale);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void drawStadiumWalls(float centerX, float centerZ, float majorRadius, float minorRadius, float height, int segments) {
    glPushMatrix();
    
    float white[] = {1.0f,1.0f, 1.0f, 1.0f};
    float black[] = {0.0f,0.0f,0.0f,1.0f};
    float wallGray[] = {0.4f, 0.4f, 0.4f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE, wallGray);
    
    glColor3f(0.4f, 0.4f, 0.4f);
    
    float wallThickness = 0.5f; 
    
    // Enable texture for stadium walls
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,mode?GL_REPLACE:GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // Outer wall 
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i<=segments;i++)
     {
        float angle = 2.0f* PI* i /segments;        
        float x =centerX +majorRadius * cos(angle);
        float z =centerZ+minorRadius *sin(angle);
        
        // Calculate normals  
        float nx = cos(angle) /majorRadius;  
        float nz=sin(angle) / minorRadius;  
        
        // Normalize the normal vector
        float length =sqrt(nx *nx + nz * nz);
        if (length > 0)
         {
            nx /= length;
            nz /= length;
        }
    
        float u_ratio =(float)i / (float)segments;
        float u = 8.0f - (u_ratio *8.0f+stadiumTextureOffset);  
        glTexCoord2f(u, 0.0f);  
        glNormal3f(nx,0.0f,nz);
        glVertex3f(x,0.0f, z);
        
        glTexCoord2f(u, 1.0f); 
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x, height, z);
    }
    glEnd();
    
    // Inner wall 
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f* PI* i / segments;        
        float x = centerX +(majorRadius - wallThickness) * cos(angle);
        float z = centerZ + (minorRadius-wallThickness) *sin(angle);
        
        // Calculate normals 
        float nx = cos(angle)/ majorRadius;  
        float nz = sin(angle) /minorRadius;  
        
        // Normalize the normal vector
        float length =sqrt(nx * nx + nz * nz);
        if (length > 0)
         {
            nx/= length;
            nz /=length;
        }
        float u_ratio = (float)i / (float)segments;
        float u = 8.0f -(u_ratio*8.0f + stadiumTextureOffset);  
        glTexCoord2f(u, 0.0f);  
        glNormal3f(-nx, 0.0f, -nz);
        glVertex3f(x,0.0f,z);
        
        glTexCoord2f(u,1.0f);  
        glNormal3f(-nx, 0.0f, -nz);
        glVertex3f(x, height, z);
    }
    glEnd();
    
    // Top surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) 
    {
        float angle = 2.0f* PI* i / segments;        
        float xOuter = centerX + majorRadius * cos(angle);
        float zOuter =centerZ + minorRadius *sin(angle);
        float xInner = centerX + (majorRadius - wallThickness) * cos(angle);
        float zInner = centerZ +(minorRadius - wallThickness) *sin(angle);

        float u_ratio = (float)i/(float)segments;
        float u = 8.0f-(u_ratio *8.0f +stadiumTextureOffset);
        glTexCoord2f(u,0.0f);  
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glVertex3f(xOuter,height, zOuter);
        glTexCoord2f(u,1.0f);  
        glVertex3f(xInner,height,zInner);
    }
    glEnd();
    
    // Bottom surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) 
    {
        float angle = 2.0f* PI* i / segments;        
        float xOuter = centerX+ majorRadius * cos(angle);
        float zOuter = centerZ + minorRadius *sin(angle);
        float xInner= centerX +(majorRadius - wallThickness) * cos(angle);
        float zInner = centerZ + (minorRadius - wallThickness) *sin(angle);
        float u_ratio = (float)i / (float)segments;
        float u = 8.0f - (u_ratio * 8.0f + stadiumTextureOffset);
        glTexCoord2f(u, 0.0f);  
        glNormal3f(0.0f, -1.0f, 0.0f);  
        glVertex3f(xOuter,0.0f,zOuter);
        glTexCoord2f(u, 1.0f);  
        glVertex3f(xInner, 0.0f, zInner);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void drawStadiumGround(float stadiumRadius)
 {
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-stadiumRadius,-0.1f,-stadiumRadius);
    glVertex3f(stadiumRadius, -0.1f, -stadiumRadius);
    glVertex3f(stadiumRadius, -0.1f, stadiumRadius);
    glVertex3f(-stadiumRadius, -0.1f, stadiumRadius);
    glEnd();
}
void drawAthleticsStadium(void) 
{
    drawStadiumGround(80.0f);  
    drawStadiumField(8.0f, 6.0f);
    drawSoccerGoalPost(-4.0f,0.0f, 1.8f, 1.2f);
    drawSoccerGoalPost(4.0f, 0.0f, 1.8f, 1.2f);
    drawStadiumTier(0.0f, 0.0f, 6.0f,4.5f, 12, 64);
    drawStadiumWalls(0.0f, 0.0f, 16.0f, 12.0f, 4.0f, 64);
    drawLightTower(-13.0f,-10.0f, 8.0f);
    drawLightTower(13.0f,-10.0f, 8.0f);
    drawLightTower(-13.0f, 10.0f,8.0f);
    drawLightTower(13.0f, 10.0f, 8.0f); 
}
void drawRing(float innerRadius, float outerRadius, float r, float g, float b) {
    glPushMatrix();
    
    float white[] = {1.0f, 1.0f, 1.0f,1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float ringColor[] = {r, g, b, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ringColor);
    
    glColor3f(r, g, b);
    int i, j;
    for (i = 0; i < RING_SEGMENTS; i++) {
        float angle1 = 2.0f * PI* i / RING_SEGMENTS;
        float angle2 =2.0f* PI * (i + 1) / RING_SEGMENTS;       
        glBegin(GL_QUAD_STRIP);
        for (j = 0; j<= TORUS_SEGMENTS;j++) {
            float angle3 = 2.0f * PI * j / TORUS_SEGMENTS;
            float x1 = (outerRadius +innerRadius * cos(angle3)) * cos(angle1);
            float y1= (outerRadius + innerRadius * cos(angle3))* sin(angle1);
            float z1 = innerRadius * sin(angle3);
            float x2 = (outerRadius +innerRadius* cos(angle3)) * cos(angle2);
            float y2 =(outerRadius+ innerRadius *cos(angle3)) * sin(angle2);
            float z2 = innerRadius * sin(angle3);
            
            // Calculate normals 
            float nx1 = cos(angle3) * cos(angle1);
            float ny1 = cos(angle3) * sin(angle1);
            float nz1 =sin(angle3);
            float nx2 = cos(angle3) * cos(angle2);
            float ny2 = cos(angle3)* sin(angle2);
            float nz2 = sin(angle3);
            
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
        }
        glEnd();
    }
    glPopMatrix();
}
void drawOlympicCauldron(float x, float y, float z) {
    glPushMatrix();
    
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float darkGray[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float lightGray[] = {0.8f, 0.8f, 0.8f, 1.0f};
    float silver[] = {0.9f, 0.9f, 0.9f, 1.0f};
    float orange[] = {1.0f, 0.5f, 0.0f, 1.0f};
    float yellow[] = {1.0f, 0.8f, 0.0f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny * 2.0f);  
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, silver);        
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,black);
    
    // Base section with glass texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[12]);
    glColor3f(0.3f, 0.3f, 0.3f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, darkGray);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 32; i++) {
        float angle =2.0f * PI * i / 32;
        float radius=1.5f;
        float x1= x + radius*cos(angle);
        float z1 = z +radius* sin(angle);
        float x2 =x + (radius -0.3f) *cos(angle);
        float z2 = z + (radius- 0.3f) * sin(angle);
        
        // Calculate normals
        float nx1 =cos(angle);
        float nz1 = sin(angle);
        float ny1 = 0.3f/sqrt(0.3f*0.3f+2.0f * 2.0f);  
        float nx2 =cos(angle);
        float nz2 =sin(angle);
        float ny2 = 0.3f / sqrt(0.3f * 0.3f + 2.0f * 2.0f);
        
        // Texture coordinates 
        float u = (float)i / 32.0f * 8.0f; 
        glTexCoord2f(u, 0.0f);
        glNormal3f(nx1, ny1, nz1);
        glVertex3f(x1,y, z1);
        glTexCoord2f(u, 1.0f);  
        glNormal3f(nx2, ny2, nz2);
        glVertex3f(x2, y +2.0f,z2);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Middle section with glass texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[5]);
    glColor3f(0.8f, 0.8f, 0.8f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lightGray);
    glBegin(GL_QUAD_STRIP);
    for (int i= 0; i<= 32; i++) {
        float angle =2.0f * PI * i / 32;
        float radius1= 1.2f;
        float radius2 = 0.8f;
        float x1 = x +radius1*cos(angle);
        float z1= z + radius1*sin(angle);
        float x2 = x + radius2*cos(angle);
        float z2 =z +radius2*sin(angle);
        
        // Calculate normals 
        float nx1 = cos(angle);
        float nz1 = sin(angle);
        float ny1 = 0.4f/sqrt(0.4f * 0.4f +1.5f * 1.5f);  
        float nx2 = cos(angle);
        float nz2 = sin(angle);
        float ny2 = 0.4f/sqrt(0.4f *0.4f + 1.5f *1.5f);
        
        // Texture coordinates 
        float u = (float)i / 32.0f * 6.0f; 
        glTexCoord2f(u, 0.0f);
        glNormal3f(nx1, ny1, nz1);
        glVertex3f(x1, y + 2.0f, z1);
        glTexCoord2f(u, 1.0f);  
        glNormal3f(nx2, ny2, nz2);
        glVertex3f(x2, y + 3.5f, z2);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Top rim (silver)
    glColor3f(0.9f,0.9f, 0.9f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, silver);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i<= 32; i++) {
        float angle = 2.0f *PI * i / 32;
        float radius = 0.8f;
        float x1= x+ radius * cos(angle);
        float z1 = z + radius *sin(angle);

        // Vertical cylinder normals
        float nx = cos(angle);
        float nz = sin(angle);
        
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x1,y+3.5f,z1);
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x1, y+3.7f,z1);
    }
    glEnd();
    
    // Top surface 
    glColor3f(0.9f, 0.9f, 0.9f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, silver);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(x, y + 3.7f, z);
    for (int i = 0; i <= 32; i++) 
    {
        float angle = 2.0f * PI * i / 32;
        float radius = 0.8f;
        float x1 = x+ radius * cos(angle);
        float z1 = z + radius*sin(angle);
        glVertex3f(x1, y + 3.7f, z1);
    }
    glEnd();
    
    //flame (orange)
    glColor3f(1.0f, 0.5f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, orange);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 8; i++) 
    {
        float angle1 = 2.0f * PI * i / 8;
        float angle2 = 2.0f * PI*(i + 1) / 8;
        float radius = 0.3f + (i % 2) * 0.1f;
        
        float x1 = x+ radius*cos(angle1);
        float z1 = z+ radius*sin(angle1);
        float x2 = x + radius*cos(angle2);
        float z2 = z + radius*sin(angle2);
        
        // Calculate normal 
        float nx = (cos(angle1) +cos(angle2)) / 2.0f;
        float nz = (sin(angle1) + sin(angle2)) /2.0f;
        float ny = 0.8f/sqrt(0.8f * 0.8f + radius* radius);
        
        // Normalize
        float length = sqrt(nx*nx+ny*ny + nz*nz);
        if (length > 0) {
            nx/=length;
            ny/=length;
            nz/=length;
        }
        
        glNormal3f(nx,ny, nz);
        glVertex3f(x,y + 4.5f,z);
        glNormal3f(nx, ny,nz);
        glVertex3f(x1,y + 3.7f,z1);
        glNormal3f(nx, ny,nz);
        glVertex3f(x2, y +3.7f,z2);
    }
    glEnd();
    
    // flame (yellow)
    glColor3f(1.0f, 0.8f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, yellow);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 6; i++) {
        float angle1 = 2.0f * PI * i / 6;
        float angle2 = 2.0f * PI * (i + 1) / 6;
        float radius = 0.15f;       
        float x1= x + radius * cos(angle1);
        float z1 = z + radius * sin(angle1);
        float x2 = x + radius * cos(angle2);
        float z2 = z + radius* sin(angle2);

        // Calculate normal 
        float nx = (cos(angle1) + cos(angle2)) / 2.0f;
        float nz = (sin(angle1) + sin(angle2)) / 2.0f;
        float ny = 0.5f / sqrt(0.5f * 0.5f + radius * radius);
        
        // Normalize
        float length = sqrt(nx *nx + ny*ny + nz *nz);
        if (length > 0) {
            nx /= length;
            ny/=length;
            nz /= length;
        }
        
        glNormal3f(nx, ny, nz);
        glVertex3f(x, y+ 4.2f, z);
        glNormal3f(nx, ny, nz);
        glVertex3f(x1, y + 3.7f, z1);
        glNormal3f(nx, ny, nz);
        glVertex3f(x2, y + 3.7f,z2);
    }
    glEnd();
    
    glPopMatrix();
}
void drawOlympicRings(void) {
    glPushMatrix();
    glTranslatef(-2.0f, 1.0f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    drawRing(0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.0f,1.0f,0.0f);
    glRotatef(0.0f, 0.0f, 0.0f,1.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    drawRing(0.2f, 1.0f, 1.0f,1.0f, 0.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 1.0f,0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    drawRing(0.2f, 1.0f, 0.0f, 0.0f, 0.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1.0f, -1.0f, 0.0f);
    glRotatef(0.0f, 0.0f,0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    drawRing(0.2f, 1.0f, 0.0f, 1.0f, 0.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.0f,-1.0f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    drawRing(0.2f, 1.0f, 1.0f, 0.0f, 0.0f);
    glPopMatrix();
}
void drawSkyscraper(float x, float z,float width,float depth,float height, int floors) {
    glPushMatrix();
    
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float gray[] = {0.7f, 0.7f, 0.8f, 1.0f};
    float darkBlue[] = {0.1f, 0.1f, 0.3f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    // Enable texture for skyscraper walls
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode?GL_REPLACE:GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    
    // Main building structure 
    glColor3f(0.7f, 0.7f, 0.8f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gray);
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3f(0.0f,0.0f,-1.0f);
    glTexCoord2f(0.0f,0.0f); glVertex3f(x - width/2,0.0f, z - depth/2);
    glTexCoord2f(skyscraperTextureRepeat, 0.0f);glVertex3f(x +width/2, 0.0f, z-depth/2);
    glTexCoord2f(skyscraperTextureRepeat,skyscraperTextureRepeat); glVertex3f(x+ width/2,height,z - depth/2);
    glTexCoord2f(0.0f,skyscraperTextureRepeat);glVertex3f(x -width/2,height,z -depth/2);
    
    // Back face
    glNormal3f(0.0f,0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x- width/2, 0.0f, z +depth/2);
    glTexCoord2f(0.0f,skyscraperTextureRepeat); glVertex3f(x-width/2,height, z + depth/2);
    glTexCoord2f(skyscraperTextureRepeat, skyscraperTextureRepeat);glVertex3f(x+width/2,height, z + depth/2);
    glTexCoord2f(skyscraperTextureRepeat,0.0f); glVertex3f(x +width/2, 0.0f,z+ depth/2);
    
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f,0.0f); glVertex3f(x - width/2, 0.0f, z - depth/2);
    glTexCoord2f(0.0f, skyscraperTextureRepeat); glVertex3f(x- width/2, height, z -depth/2);
    glTexCoord2f(skyscraperTextureRepeat, skyscraperTextureRepeat);glVertex3f(x- width/2, height, z + depth/2);
    glTexCoord2f(skyscraperTextureRepeat,0.0f); glVertex3f(x -width/2,0.0f, z + depth/2);
    
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f,0.0f);glVertex3f(x + width/2, 0.0f, z -depth/2);
    glTexCoord2f(skyscraperTextureRepeat, 0.0f); glVertex3f(x +width/2,0.0f, z + depth/2);
    glTexCoord2f(skyscraperTextureRepeat, skyscraperTextureRepeat);glVertex3f(x + width/2, height,z + depth/2);
    glTexCoord2f(0.0f,skyscraperTextureRepeat); glVertex3f(x+ width/2, height,z - depth/2);
    
    // Top face
    glNormal3f(0.0f, 1.0f,0.0f);
    glTexCoord2f(0.0f,0.0f); glVertex3f(x -width/2, height,z -depth/2);
    glTexCoord2f(skyscraperTextureRepeat,0.0f); glVertex3f(x+width/2,height,z- depth/2);
    glTexCoord2f(skyscraperTextureRepeat, skyscraperTextureRepeat); glVertex3f(x +width/2, height,z +depth/2);
    glTexCoord2f(0.0f,skyscraperTextureRepeat);glVertex3f(x - width/2,height,z+ depth/2);
    
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Windows (dark blue)
    glColor3f(0.1f, 0.1f, 0.3f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, darkBlue);
    float floorHeight = height / floors;
    float windowWidth = width *0.15f;
    float windowHeight = floorHeight*0.6f;
    
    for (int floor = 1; floor< floors; floor++) {
        float y = floor*floorHeight;
        for (int i = 0; i <3; i++) {
            float windowX = x -width/3 + i * width/3;
            glBegin(GL_QUADS);
            glNormal3f(0.0f,0.0f,-1.0f);  
            glVertex3f(windowX -windowWidth/2, y+ floorHeight*0.2f, z -depth/2-0.01f);
            glVertex3f(windowX + windowWidth/2, y+ floorHeight*0.2f, z - depth/2 - 0.01f);
            glVertex3f(windowX + windowWidth/2,y +floorHeight*0.2f +windowHeight, z - depth/2- 0.01f);
            glVertex3f(windowX- windowWidth/2, y +floorHeight*0.2f + windowHeight, z- depth/2 -0.01f);
            glEnd();
        }
        for (int i = 0; i < 3; i++) {
            float windowX = x - width/3+i *width/3;
            glBegin(GL_QUADS);
            glNormal3f(0.0f,0.0f, 1.0f);  
            glVertex3f(windowX- windowWidth/2, y+ floorHeight*0.2f,z + depth/2 + 0.01f);
            glVertex3f(windowX -windowWidth/2, y +floorHeight*0.2f +windowHeight, z +depth/2 +0.01f);
            glVertex3f(windowX+ windowWidth/2, y+ floorHeight*0.2f +windowHeight, z +depth/2+ 0.01f);
            glVertex3f(windowX +windowWidth/2, y + floorHeight*0.2f, z+ depth/2 + 0.01f);
            glEnd();
        }
    }
    glPopMatrix();
}

void drawCube(float size) {
    float half = size/2.0f;
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float brown[] = {0.4f, 0.2f, 0.1f, 1.0f};
    float green[] = {0.2f, 0.6f, 0.2f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    
    // Bottom face 
    glColor3f(0.4f, 0.2f, 0.1f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, brown);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(-half,-half, -half);
    glVertex3f(half,-half, -half);
    glVertex3f(half, -half,half);
    glVertex3f(-half, -half, half);
    glEnd();

    // Top face 
    glColor3f(0.2f, 0.6f, 0.2f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(-half,half,-half);
    glVertex3f(-half,half, half);
    glVertex3f(half, half,half);
    glVertex3f(half, half, -half);
    glEnd();

    // Front face
    glColor3f(0.4f, 0.2f, 0.1f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, brown);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(-half, -half, half);
    glVertex3f(half,-half,half);
    glVertex3f(half, half,half);
    glVertex3f(-half, half, half);
    glEnd();
    
    // Back face 
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glVertex3f(-half, -half, -half);
    glVertex3f(-half,half,-half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);
    glEnd();
    
    // Right face 
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f); 
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half,half);
    glVertex3f(half, -half, half);
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glVertex3f(-half,-half,-half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half,half,-half);
    glEnd();
}

void drawMountain(float centerX, float centerZ, int baseWidth, int baseDepth, int maxHeight) {   
    float cubeSize = 0.3f; 
    glPushMatrix();
    glTranslatef(centerX, 0.0f, centerZ);
    
    float white[] = {1.0f, 1.0f,1.0f, 1.0f};
    float black[] = {0.0f, 0.0f,0.0f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION, black);   
    for (int layer=0;layer<maxHeight;layer++) {
        float currentY = 0.1f+ (layer *cubeSize);
           
        for (int x =0; x< baseWidth; x++) {
            for (int z =0; z <baseDepth; z++) {
                
                float centerX_f= baseWidth/2.0f;
                float centerZ_f =baseDepth/2.0f;    
                float xDistance =fabs(x-centerX_f) / (baseWidth / 2.0f);  
                float zDistance = fabs(z-centerZ_f) /(baseDepth / 2.0f);    
                int heightAtThisPoint = (int)(maxHeight * (1.0f - xDistance * 0.8f - zDistance * 0.3f));
                heightAtThisPoint+= (int)(sin(x * 0.3f) *cos(z * 0.4f) * 3);  
                heightAtThisPoint += (int)(sin(x * 0.1f + z * 0.1f) * 2);       
                heightAtThisPoint +=(int)(cos(x * 0.2f) * sin(z * 0.3f) * 2);  
                float distanceFromCenter =sqrt((x - centerX_f)*(x - centerX_f) +(z - centerZ_f)*(z - centerZ_f));
                float maxDistance = sqrt(centerX_f * centerX_f + centerZ_f * centerZ_f);
                if (distanceFromCenter < maxDistance * 0.3f){
                    heightAtThisPoint += (int)(sin(x*0.15f)*cos(z*0.15f)*4);
                }

                heightAtThisPoint +=(int)(sin(x*0.7f+ z*0.5f)*1.5f);

                if (layer <heightAtThisPoint && layer >= 0) {
                    
                    if((x +z +layer) % 7!= 0){ 
                        glPushMatrix();
                        glTranslatef((x- baseWidth/2.0f)* cubeSize,currentY,(z -baseDepth/2.0f)*cubeSize);
                        drawCube(cubeSize);
                        glPopMatrix();
                    }
                }
            }
        }
    }
    
    glPopMatrix();
}

void drawTicketBooth(void) {
    glPushMatrix();
    
    // Set material properties for lighting - PROFESSOR CODE referemce (adapted for ticket booth)
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float brown[] = {0.6f, 0.4f, 0.2f, 1.0f};
    float blue[] = {0.1f, 0.1f, 0.6f, 1.0f};
    float yellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    // Floor
    glColor3f(0.1f, 0.1f, 0.6f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blue);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); 
    glVertex3f(-1.5f, 0.0f, -1.5f);  
    glVertex3f(1.5f, 0.0f, -1.5f);   
    glVertex3f(1.5f, 0.0f, 1.5f);    
    glVertex3f(-1.5f, 0.0f, 1.5f);   
    glEnd();
    
    // Enable brick texture for walls
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    
    // Front wall with brick texture
    glColor3f(0.6f, 0.4f, 0.2f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, brown);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, 0.0f, 1.5f);  
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, 0.0f, 1.5f);    
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, 3.0f, 1.5f);    
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, 3.0f, 1.5f);   
    glEnd();

    // Back wall with brick texture
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f,-1.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, 0.0f,-1.5f);  
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f,3.0f, -1.5f);  
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, 3.0f, -1.5f);   
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, 0.0f,-1.5f);   
    glEnd();

    // Left wall with brick texture
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, 0.0f, -1.5f);  
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.5f, 0.0f, 1.5f);   
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, 3.0f, 1.5f);   
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, 3.0f, -1.5f);  
    glEnd();

    // Right wall with brick texture
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f,0.0f,-1.5f);   
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, 3.0f, -1.5f);   
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f,3.0f, 1.5f);    
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, 0.0f, 1.5f);    
    glEnd();
    
    // Disable texture after drawing walls
    glDisable(GL_TEXTURE_2D);

    // Window
    glColor3f(1.0f, 1.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, yellow);
    float windowEmission[] = {0.3f, 0.3f,0.0f, 1.0f};  
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION, windowEmission);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(-0.5f, 1.2f, 1.51f);  
    glVertex3f(0.5f, 1.2f, 1.51f);   
    glVertex3f(0.5f, 2.1f, 1.51f);   
    glVertex3f(-0.5f, 2.1f, 1.51f); 
    glEnd();
   
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    

    // Roof base
    float roofColor[4];
    switch(ticketBoothRoofColor) {
        case 0: 
            roofColor[0] = 1.0f; roofColor[1] = 1.0f; roofColor[2] = 1.0f; roofColor[3] =1.0f;
            break;
        case 1: 
            roofColor[0] = 1.0f; roofColor[1] = 0.0f; roofColor[2]=0.0f; roofColor[3] = 1.0f;
            break;
        case 2: 
            roofColor[0] = 0.0f; roofColor[1] = 0.0f;roofColor[2] = 1.0f; roofColor[3] = 1.0f;
            break;
        case 3: 
            roofColor[0]=0.0f; roofColor[1] = 1.0f; roofColor[2] = 0.0f; roofColor[3] = 1.0f;
            break;
    }
    glColor3f(roofColor[0], roofColor[1], roofColor[2]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, roofColor);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(-1.8f, 3.0f, -1.8f);  
    glVertex3f(1.8f, 3.0f, -1.8f);  
    glVertex3f(1.8f, 3.0f, 1.8f);   
    glVertex3f(-1.8f, 3.0f, 1.8f);  
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE ,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[4]);

    // Roof triangles with texture coordinates
    // Front roof triangle
    glColor3f(roofColor[0], roofColor[1], roofColor[2]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, roofColor);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.447f, 0.894f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.8f, 3.0f, 1.8f);  
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f,3.8f, 0.0f);     
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.8f, 3.0f, 1.8f);
    glEnd();
    
    // Back roof triangle
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f,0.447f,-0.894f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(-1.8f, 3.0f,-1.8f);
    glTexCoord2f(1.0f,0.0f); glVertex3f(1.8f,3.0f, -1.8f);
    glTexCoord2f(0.5f,1.0f); glVertex3f(0.0f, 3.8f,0.0f);     
    glEnd();
    
    // Left roof triangle
    glBegin(GL_TRIANGLES);
    glNormal3f(-0.894f, 0.447f, 0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(-1.8f,3.0f,-1.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.8f,3.0f,1.8f);
    glTexCoord2f(0.5f,1.0f); glVertex3f(0.0f, 3.8f, 0.0f);
    glEnd();
    
    // Right roof triangle
    glBegin(GL_TRIANGLES);
    glNormal3f(0.894f, 0.447f, 0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(1.8f, 3.0f, -1.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.8f,3.0f,1.8f);
    glTexCoord2f(0.5f,1.0f); glVertex3f(0.0f,3.8f, 0.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void drawSwimmingPool(void) {
    glPushMatrix();
  
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float water[] = {0.2f,0.6f, 0.9f,1.0f};
    float poolWall[] ={0.7f,0.9f,1.0f,1.0f};
    float poolBottom[] = {0.4f,0.7f,0.9f,1.0f};
    float deck[] = {0.6f, 0.6f, 0.6f, 1.0f};
    float steps[] ={0.8f, 0.7f, 0.6f, 1.0f};
    float handrail[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float waterSpecular[] ={0.8f, 0.9f, 1.0f, 1.0f};  
   
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny * 1.5f);  
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,waterSpecular);  
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,black);
    
    // Water surface with texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glColor3f(0.2f, 0.6f,0.9f);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,water);
    glBegin(GL_QUADS);
    glNormal3f(0.0f,1.0f,0.0f); 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, 0.0f, -2.0f);  
    glTexCoord2f(1.0f,0.0f); glVertex3f(4.0f,0.0f, -2.0f);   
    glTexCoord2f(1.0f, 1.0f);glVertex3f(4.0f, 0.0f,2.0f);    
    glTexCoord2f(0.0f,1.0f); glVertex3f(-4.0f, 0.0f, 2.0f);   
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Pool walls with brick texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[9]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glColor3f(0.7f,0.9f,1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, poolWall);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glTexCoord2f(0.0f, 0.0f);glVertex3f(-4.0f, -1.5f, 2.0f);  
    glTexCoord2f(8.0f, 0.0f);glVertex3f(4.0f,-1.5f, 2.0f);   
    glTexCoord2f(8.0f,2.0f); glVertex3f(4.0f, 0.0f,2.0f);    
    glTexCoord2f(0.0f, 2.0f); glVertex3f(-4.0f, 0.0f, 2.0f);  
    glEnd();
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, -1.5f,-2.0f); 
    glTexCoord2f(0.0f,2.0f);glVertex3f(-4.0f, 0.0f, -2.0f);  
    glTexCoord2f(8.0f, 2.0f); glVertex3f(4.0f,0.0f, -2.0f);   
    glTexCoord2f(8.0f,0.0f); glVertex3f(4.0f, -1.5f, -2.0f);  
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, -1.5f, -2.0f); 
    glTexCoord2f(4.0f,0.0f); glVertex3f(-4.0f, -1.5f,2.0f);  
    glTexCoord2f(4.0f,2.0f); glVertex3f(-4.0f, 0.0f, 2.0f);  
    glTexCoord2f(0.0f,2.0f); glVertex3f(-4.0f,0.0f, -2.0f); 
    glEnd();
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(4.0f, -1.5f, -2.0f);  
    glTexCoord2f(0.0f, 2.0f);glVertex3f(4.0f,0.0f, -2.0f);   
    glTexCoord2f(4.0f, 2.0f); glVertex3f(4.0f,0.0f, 2.0f);    
    glTexCoord2f(4.0f,0.0f); glVertex3f(4.0f, -1.5f,2.0f);   
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    // Pool bottom (darker blue)
    glColor3f(0.4f, 0.7f, 0.9f);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,poolBottom);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(-4.0f, -1.5f, -2.0f); 
    glVertex3f(4.0f,-1.5f, -2.0f);  
    glVertex3f(4.0f, -1.5f,2.0f);   
    glVertex3f(-4.0f, -1.5f, 2.0f); 
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glColor3f(0.6f, 0.6f, 0.6f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, deck);
    
    float deckThickness = 0.3f;  
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(-5.0f, 0.0f,2.0f);  
    glTexCoord2f(3.4f, 0.0f);glVertex3f(-1.6f, 0.0f, 2.0f);   
    glTexCoord2f(3.4f,1.0f); glVertex3f(-1.6f,0.0f,3.0f);   
    glTexCoord2f(0.0f,1.0f); glVertex3f(-5.0f,0.0f, 3.0f);  
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Bottom surface
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(-5.0f,-deckThickness, 2.0f);  
    glVertex3f(-5.0f, -deckThickness, 3.0f);   
    glVertex3f(-1.6f,-deckThickness,3.0f);   
    glVertex3f(-1.6f,-deckThickness, 2.0f);  
    glEnd();
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glVertex3f(-5.0f, -deckThickness, 2.0f);  
    glVertex3f(-1.6f, -deckThickness, 2.0f);   
    glVertex3f(-1.6f,0.0f,2.0f);   
    glVertex3f(-5.0f,0.0f, 2.0f);  
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(-5.0f, -deckThickness, 3.0f);  
    glVertex3f(-5.0f,0.0f, 3.0f);   
    glVertex3f(-1.6f, 0.0f, 3.0f);   
    glVertex3f(-1.6f, -deckThickness,3.0f);  
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f,0.0f, 0.0f);  
    glVertex3f(-5.0f, -deckThickness, 2.0f);  
    glVertex3f(-5.0f,0.0f,2.0f);   
    glVertex3f(-5.0f,0.0f, 3.0f);   
    glVertex3f(-5.0f,-deckThickness,3.0f);  
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1.0f,0.0f, 0.0f);  
    glVertex3f(-1.6f,-deckThickness, 2.0f);  
    glVertex3f(-1.6f, -deckThickness, 3.0f);   
    glVertex3f(-1.6f, 0.0f, 3.0f);   
    glVertex3f(-1.6f,0.0f, 2.0f);  
    glEnd();
   
    // Top surface with swimming deck texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    
    // Set texture wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f,0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(1.6f, 0.0f,2.0f);  
    glTexCoord2f(3.4f, 0.0f);glVertex3f(5.0f,0.0f,2.0f);   
    glTexCoord2f(3.4f, 1.0f);glVertex3f(5.0f, 0.0f, 3.0f);   
    glTexCoord2f(0.0f,1.0f); glVertex3f(1.6f,0.0f,3.0f);  
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Bottom surface
    glBegin(GL_QUADS);
    glNormal3f(0.0f,-1.0f, 0.0f); 
    glVertex3f(1.6f,-deckThickness, 2.0f);  
    glVertex3f(1.6f,-deckThickness, 3.0f);   
    glVertex3f(5.0f,-deckThickness, 3.0f);   
    glVertex3f(5.0f,-deckThickness, 2.0f);  
    glEnd();
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glVertex3f(1.6f, -deckThickness, 2.0f);  
    glVertex3f(5.0f, -deckThickness, 2.0f);   
    glVertex3f(5.0f, 0.0f, 2.0f);   
    glVertex3f(1.6f, 0.0f, 2.0f);  
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(1.6f, -deckThickness,3.0f);  
    glVertex3f(1.6f, 0.0f, 3.0f);   
    glVertex3f(5.0f, 0.0f, 3.0f);   
    glVertex3f(5.0f,-deckThickness,3.0f);  
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glVertex3f(1.6f, -deckThickness,2.0f);  
    glVertex3f(1.6f, 0.0f,2.0f);   
    glVertex3f(1.6f,0.0f,3.0f);   
    glVertex3f(1.6f,-deckThickness,3.0f);  
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);  
    glVertex3f(5.0f,-deckThickness, 2.0f);  
    glVertex3f(5.0f,-deckThickness, 3.0f);   
    glVertex3f(5.0f,0.0f, 3.0f);   
    glVertex3f(5.0f,0.0f, 2.0f);  
    glEnd();
    
    // Front deck with swimming deck texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    
    // Set texture wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.0f, -3.0f); 
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 0.0f, -2.0f); 
    glTexCoord2f(10.0f, 1.0f); glVertex3f(5.0f, 0.0f, -2.0f);  
    glTexCoord2f(10.0f, 0.0f); glVertex3f(5.0f, 0.0f, -3.0f);  
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Bottom surface
    glBegin(GL_QUADS);
    glNormal3f(0.0f,-1.0f, 0.0f);  
    glVertex3f(-5.0f,-deckThickness, -3.0f); 
    glVertex3f(5.0f,-deckThickness, -3.0f);  
    glVertex3f(5.0f, -deckThickness, -2.0f);  
    glVertex3f(-5.0f, -deckThickness, -2.0f);  
    glEnd();
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f); 
    glVertex3f(-5.0f, -deckThickness, -3.0f); 
    glVertex3f(-5.0f, 0.0f, -3.0f);  
    glVertex3f(5.0f, 0.0f, -3.0f);  
    glVertex3f(5.0f, -deckThickness, -3.0f);  
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(-5.0f, -deckThickness, -2.0f); 
    glVertex3f(5.0f, -deckThickness, -2.0f);  
    glVertex3f(5.0f, 0.0f, -2.0f);  
    glVertex3f(-5.0f, 0.0f, -2.0f);  
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glVertex3f(-5.0f,-deckThickness, -3.0f); 
    glVertex3f(-5.0f, -deckThickness, -2.0f);  
    glVertex3f(-5.0f,0.0f, -2.0f);  
    glVertex3f(-5.0f, 0.0f, -3.0f);  
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f,0.0f);  
    glVertex3f(5.0f,-deckThickness,-3.0f);  
    glVertex3f(5.0f, 0.0f, -3.0f);  
    glVertex3f(5.0f, 0.0f, -2.0f);  
    glVertex3f(5.0f, -deckThickness,-2.0f);  
    glEnd();
    // Top surface
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    
    // Set texture wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glTexCoord2f(0.0f, 0.0f);glVertex3f(-5.0f, 0.0f,-2.0f); 
    glTexCoord2f(1.0f,0.0f); glVertex3f(-4.0f,0.0f, -2.0f); 
    glTexCoord2f(1.0f, 4.0f); glVertex3f(-4.0f, 0.0f, 2.0f);  
    glTexCoord2f(0.0f,4.0f);glVertex3f(-5.0f, 0.0f, 2.0f);  
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Bottom surface
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(-5.0f,-deckThickness,-2.0f); 
    glVertex3f(-5.0f, -deckThickness,2.0f);  
    glVertex3f(-4.0f, -deckThickness,2.0f);  
    glVertex3f(-4.0f,-deckThickness, -2.0f);  
    glEnd();
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f,-1.0f);  
    glVertex3f(-5.0f, -deckThickness,-2.0f); 
    glVertex3f(-4.0f,-deckThickness, -2.0f);  
    glVertex3f(-4.0f, 0.0f, -2.0f);  
    glVertex3f(-5.0f,0.0f,-2.0f);  
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f,1.0f);  
    glVertex3f(-5.0f,-deckThickness, 2.0f);  
    glVertex3f(-5.0f, 0.0f,2.0f);  
    glVertex3f(-4.0f,0.0f, 2.0f);  
    glVertex3f(-4.0f, -deckThickness, 2.0f);  
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);  
    glVertex3f(-5.0f, -deckThickness, -2.0f); 
    glVertex3f(-5.0f,0.0f,-2.0f);  
    glVertex3f(-5.0f,0.0f, 2.0f);  
    glVertex3f(-5.0f,-deckThickness,2.0f);  
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);  
    glVertex3f(-4.0f, -deckThickness, -2.0f);  
    glVertex3f(-4.0f, -deckThickness, 2.0f);  
    glVertex3f(-4.0f, 0.0f, 2.0f);  
    glVertex3f(-4.0f, 0.0f, -2.0f);  
    glEnd();

    // Top surface
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    
    // Set texture wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f,0.0f);  
    glTexCoord2f(0.0f,0.0f); glVertex3f(4.0f, 0.0f,-2.0f);  
    glTexCoord2f(1.0f, 0.0f);glVertex3f(5.0f,0.0f, -2.0f);  
    glTexCoord2f(1.0f, 4.0f); glVertex3f(5.0f,0.0f, 2.0f);   
    glTexCoord2f(0.0f,4.0f); glVertex3f(4.0f, 0.0f, 2.0f);   
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Bottom surface
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(4.0f, -deckThickness,-2.0f);  
    glVertex3f(4.0f,-deckThickness,2.0f);   
    glVertex3f(5.0f, -deckThickness,2.0f);   
    glVertex3f(5.0f,-deckThickness,-2.0f);  
    glEnd();
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);  
    glVertex3f(4.0f,-deckThickness,-2.0f);  
    glVertex3f(5.0f,-deckThickness,-2.0f);  
    glVertex3f(5.0f, 0.0f, -2.0f);  
    glVertex3f(4.0f, 0.0f, -2.0f);  
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  
    glVertex3f(4.0f, -deckThickness, 2.0f);   
    glVertex3f(4.0f, 0.0f, 2.0f);   
    glVertex3f(5.0f, 0.0f, 2.0f);   
    glVertex3f(5.0f, -deckThickness, 2.0f);  
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f); 
    glVertex3f(4.0f,-deckThickness, -2.0f);  
    glVertex3f(4.0f,0.0f,-2.0f);  
    glVertex3f(4.0f, 0.0f, 2.0f);   
    glVertex3f(4.0f, -deckThickness,2.0f);  
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f); 
    glVertex3f(5.0f, -deckThickness,-2.0f);  
    glVertex3f(5.0f,-deckThickness,2.0f);  
    glVertex3f(5.0f,0.0f, 2.0f);  
    glVertex3f(5.0f, 0.0f, -2.0f);  
    glEnd();
    
    // Diving board steps 
    glColor3f(0.8f, 0.7f, 0.6f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, steps);
    
    for (int i = 0; i < 6; i++) {
        float stepY = 0.3f-0.25f-(i* 0.25f);  
        float stepZ = 2.0f + (i * 0.4f);     
        
        // Step top surface
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glVertex3f(-1.5f, stepY, stepZ);      
        glVertex3f(1.5f, stepY,stepZ);       
        glVertex3f(1.5f, stepY, stepZ + 0.4f); 
        glVertex3f(-1.5f,stepY, stepZ+ 0.4f); 
        glEnd();
   
        // Step front face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f,1.0f);  
        glVertex3f(-1.5f,stepY - 0.25f, stepZ + 0.4f);
        glVertex3f(1.5f, stepY -0.25f, stepZ +0.4f);  
        glVertex3f(1.5f, stepY, stepZ+ 0.4f);          
        glVertex3f(-1.5f, stepY, stepZ +0.4f);         
        glEnd();
        
        // Step left face
        glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);  
        glVertex3f(-1.5f,stepY - 0.25f, stepZ);        
        glVertex3f(-1.5f,stepY -0.25f,stepZ+ 0.4f); 
        glVertex3f(-1.5f, stepY,stepZ + 0.4f);         
        glVertex3f(-1.5f,stepY,stepZ);               
        glEnd();
  
        // Step right face
        glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);  
        glVertex3f(1.5f, stepY - 0.25f, stepZ);         
        glVertex3f(1.5f,stepY,stepZ);                
        glVertex3f(1.5f, stepY,stepZ +0.4f);          
        glVertex3f(1.5f,stepY -0.25f,stepZ + 0.4f);  
        glEnd();
    }

    // Handrails with HandRail texture 
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture[11]);
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, handrail);
    
    // Left handrail - vertical posts
    for (int i =0; i< 6;i++) {
        float stepY = -0.125f -(i * 0.25f); 
        float stepZ = 2.0f + (i *0.4f);      
        
        // Front face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);  
        glTexCoord2f(0.0f,0.0f);glVertex3f(-1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 0.0f);glVertex3f(-1.5f,stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, stepY + 0.4f, stepZ + 0.2f); 
        glTexCoord2f(0.0f,1.0f);glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f); 
        glEnd();
        
        // Back face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f,-1.0f);  
        glTexCoord2f(0.0f,0.0f); glVertex3f(-1.5f,stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f,0.0f); glVertex3f(-1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f); 
        glTexCoord2f(0.0f,1.0f); glVertex3f(-1.5f, stepY +0.4f, stepZ + 0.2f); 
        glEnd();
        
        // Left face of post
        glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f,0.0f); glVertex3f(-1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f); 
        glTexCoord2f(0.0f,1.0f); glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f); 
        glEnd();
        
        // Right face of post
        glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);  
        glTexCoord2f(0.0f,0.0f); glVertex3f(-1.5f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f,0.0f);glVertex3f(-1.5f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f);glVertex3f(-1.5f, stepY + 0.4f, stepZ + 0.2f); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, stepY + 0.4f, stepZ + 0.2f); 
        glEnd();
        
        // Top face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f);        
        glTexCoord2f(1.0f,0.0f);glVertex3f(-1.5f, stepY + 0.4f, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, stepY + 0.4f, stepZ + 0.2f); 
        glTexCoord2f(0.0f,1.0f);glVertex3f(-1.6f, stepY + 0.4f, stepZ + 0.2f); 
        glEnd();
    }
    
    // Left handrail - continuous horizontal top rail
    for (int i = 0; i < 5; i++) {
        float stepY1 = -0.125f - (i * 0.25f) + 0.4f; 
        float stepZ1 = 2.0f + (i * 0.4f) + 0.2f;
        float stepY2 = -0.125f - ((i+1) * 0.25f) + 0.4f; 
        float stepZ2 = 2.0f + ((i+1) * 0.4f) + 0.2f;
        
        // Top face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.6f, stepY1, stepZ1);        
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.5f, stepY1, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, stepY2, stepZ2); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.6f, stepY2, stepZ2); 
        glEnd();
        
        // Bottom face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.6f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.6f, stepY2 - 0.05f, stepZ2); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, stepY2 - 0.05f, stepZ2); 
        glEnd();
        
        // Front face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.6f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.5f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, stepY1, stepZ1); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.6f, stepY1, stepZ1); 
        glEnd();
        
        // Back face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.6f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.6f, stepY1, stepZ1); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, stepY1, stepZ1); 
        glEnd();
    }
    
    // Right handrail - vertical posts
    for (int i = 0; i < 6; i++) {
        float stepY = -0.125f - (i * 0.25f); 
        float stepZ = 2.0f + (i * 0.4f);      
        
        // Front face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, stepY, stepZ + 0.2f);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);  
        glEnd();
        
        // Back face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.6f, stepY, stepZ + 0.2f);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);  
        glEnd();
        
        // Left face of post
        glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, stepY, stepZ + 0.2f);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);  
        glEnd();
        
        // Right face of post
        glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.6f, stepY, stepZ + 0.2f);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.6f, stepY, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);  
        glEnd();
        
        // Top face of post
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.6f, stepY + 0.4f, stepZ + 0.2f);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, stepY + 0.4f, stepZ + 0.2f);  
        glEnd();
    }
    
    // Right handrail - continuous horizontal top rail
    for (int i = 0; i < 5; i++) {
        float stepY1 = -0.125f - (i * 0.25f) + 0.4f; 
        float stepZ1 = 2.0f + (i * 0.4f) + 0.2f;
        float stepY2 = -0.125f - ((i+1) * 0.25f) + 0.4f; 
        float stepZ2 = 2.0f + ((i+1) * 0.4f) + 0.2f;
        
        // Top face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, stepY1, stepZ1);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.6f, stepY1, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.6f, stepY2, stepZ2);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, stepY2, stepZ2);  
        glEnd();
        
        // Bottom face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.6f, stepY1 - 0.05f, stepZ1);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, stepY2 - 0.05f, stepZ2);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.6f, stepY2 - 0.05f, stepZ2);  
        glEnd();
        
        // Front face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, stepY1 - 0.05f, stepZ1);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.6f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.6f, stepY1, stepZ1);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, stepY1, stepZ1);  
        glEnd();
        
        // Back face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);  
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.6f, stepY1 - 0.05f, stepZ1);         
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, stepY1 - 0.05f, stepZ1);        
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, stepY1, stepZ1);  
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.6f, stepY1, stepZ1);  
        glEnd();
    }
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawForearm(float x, float y, float z, float length,float radius)
 {
    glPushMatrix();
    glTranslatef(x, y,z);

    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float skin[] = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE, skin);
    
    const int segments = 16;

    // Cylinder side
    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * PI;
        float angle2 = (float)(i + 1) /segments * 2.0f * PI;
        
        float x1 = radius * cos(angle1);
        float z1= radius * sin(angle1);
        float x2 = radius* cos(angle2);
        float z2 =radius * sin(angle2);

        // Calculate normals
        float nx1 = x1 / radius;
        float nz1 = z1 / radius;
        float nx2 = x2 / radius;
        float nz2 = z2 / radius;

        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f,z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, length, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,length, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, length, z1);
        glEnd();
    }

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(0.0f, length, 0.0f);  
    for (int i = 0; i <=segments; i++) {
        float angle =(float)i / segments * 2.0f * PI;
        float x = radius* cos(angle);
        float z= radius * sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);  
        glVertex3f(x,length, z);
    }
    glEnd();

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= segments;i++) 
    {
        float angle = (float)i /segments*2.0f*PI;
        float x = radius * cos(angle);
        float z= radius* sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);  
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
}

void drawPalm(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Set material properties for lighting
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float skin[] = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, skin);

    const int segments = 16;
    const int rings = 8;
    const float radiusX = 0.2f;  
    const float radiusY = 0.2f; 
    const float radiusZ = 0.2f;  

    for (int i =0; i< rings;i++) {
        float phi1 =(float)i / rings * PI;
        float phi2 = (float)(i + 1) / rings * PI;
        
        for (int j = 0; j < segments; j++) 
        {
            float theta1 = (float)j /segments * 2.0f * PI;
            float theta2 = (float)(j + 1) / segments * 2.0f * PI;
            float x1 = radiusX * sin(phi1)* cos(theta1);
            float y1 =radiusY * cos(phi1);
            float z1 = radiusZ* sin(phi1) * sin(theta1);
            
            float x2 =radiusX * sin(phi1) * cos(theta2);
            float y2 = radiusY *cos(phi1);
            float z2 = radiusZ * sin(phi1) *sin(theta2);
            float x3 =radiusX* sin(phi2) * cos(theta1);
            float y3 = radiusY * cos(phi2);
            float z3 = radiusZ *sin(phi2) * sin(theta1);
            
            float x4 =radiusX * sin(phi2) * cos(theta2);
            float y4 = radiusY* cos(phi2);
            float z4 = radiusZ * sin(phi2)* sin(theta2);
            
            // Calculate normals 
            float nx1=x1 / radiusX;
            float ny1 = y1 / radiusY;
            float nz1=z1 / radiusZ;
            float nx2 =x2/radiusX;
            float ny2 = y2 /radiusY;
            float nz2 = z2/radiusZ;
            float nx3 = x3/radiusX;
            float ny3=y3/radiusY;
            float nz3 = z3 / radiusZ;
            float nx4 =x4 /radiusX;
            float ny4=y4/radiusY;
            float nz4 =z4 /radiusZ;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1,z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2, z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3,z3);
            glEnd();
        }
    }
    
    glPopMatrix();
}

void drawMickeyHand(float x, float y, float z, int isLeftHand) {
    glPushMatrix();
    glTranslatef(x, y, z);

    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float skin[] = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    // Forearm (black)
    glColor3f(0.0f,0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 0.0f,1.0f);  
    drawForearm(0.0f, 0.0f,0.0f, 1.0f, 0.1f);  
    glPopMatrix();
 
    // Palm (yellow)
    glColor3f(1.0f,1.0f, 0.0f);  
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, skin);
    glPushMatrix();
    glRotatef(180.0f, 0.0f,0.0f, 1.0f);  
    drawPalm(0.0f, 0.9f, 0.0f); 
    glPopMatrix();
    glPopMatrix();
}

void drawMickeyMouse(void) {
    glPushMatrix();

    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float yellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    // Head (black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);  
    glPushMatrix();
    glTranslatef(0.0f,0.8f, -0.3f);  
    glScalef(0.8f, 0.8f, 0.8f); 

    const int segments= 20;
    const int rings = 10;
    const float radius =1.0f;
    
    for (int i =0; i< rings;i++) {
        float phi1 = (float)i/ rings * PI;
        float phi2 = (float)(i + 1) /rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 = (float)j / segments * 2.0f * PI;
            float theta2 = (float)(j + 1) /segments* 2.0f * PI;
            float x1 = radius * sin(phi1) * cos(theta1);
            float y1 = radius* cos(phi1);
            float z1 =radius * sin(phi1) * sin(theta1); 
            float x2 =radius * sin(phi1) *cos(theta2);
            float y2 = radius* cos(phi1);
            float z2=radius * sin(phi1)* sin(theta2);
            
            float x3 =radius * sin(phi2) *cos(theta1);
            float y3 = radius *cos(phi2);
            float z3 = radius * sin(phi2)* sin(theta1);
            
            float x4 = radius * sin(phi2)* cos(theta2);
            float y4 =radius* cos(phi2);
            float z4 = radius * sin(phi2) *sin(theta2);

            // Calculate normal 
            float nx1 = x1/radius;
            float ny1 =y1 / radius;
            float nz1 = z1 / radius;
            float nx2 = x2 /radius;
            float ny2=y2 / radius;
            float nz2 = z2 / radius;
            float nx3 = x3 /radius;
            float ny3= y3/ radius;
            float nz3=z3 /radius;
            float nx4 = x4/ radius;
            float ny4= y4 /radius;
            float nz4= z4 /radius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);  
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Eyes (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glPushMatrix();
    glTranslatef(-0.25f, 1.0f, 0.3f);  
    glScalef(0.15f, 0.15f, 0.15f);
    const int eyeSegments = 16;
    const int eyeRings = 8;
    const float eyeRadius = 1.0f;
    
    for (int i = 0; i < eyeRings; i++) {
        float phi1 = (float)i / eyeRings * PI;
        float phi2 =(float)(i + 1) / eyeRings * PI;
        for (int j =0; j <eyeSegments; j++) {
            float theta1 =(float)j / eyeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / eyeSegments * 2.0f * PI;
            float x1 = eyeRadius * sin(phi1) * cos(theta1);
            float y1 = eyeRadius * cos(phi1);
            float z1 = eyeRadius * sin(phi1) * sin(theta1);           
            float x2= eyeRadius* sin(phi1) * cos(theta2);
            float y2 = eyeRadius * cos(phi1);
            float z2 =eyeRadius *sin(phi1) * sin(theta2);
            float x3 = eyeRadius * sin(phi2) *cos(theta1);
            float y3 =eyeRadius * cos(phi2);
            float z3 = eyeRadius * sin(phi2) * sin(theta1);
            
            float x4 =eyeRadius * sin(phi2) * cos(theta2);
            float y4 = eyeRadius * cos(phi2);
            float z4 = eyeRadius *sin(phi2) * sin(theta2);

            // Calculate normal 
            float nx1 = x1 / eyeRadius;
            float ny1=y1 / eyeRadius;
            float nz1 =z1 / eyeRadius;
            float nx2 = x2 / eyeRadius;
            float ny2 = y2 / eyeRadius;
            float nz2 =z2/ eyeRadius;
            float nx3 = x3 / eyeRadius;
            float ny3 = y3 / eyeRadius;
            float nz3 = z3 / eyeRadius;
            float nx4 =x4 /eyeRadius;
            float ny4= y4 / eyeRadius;
            float nz4=z4 / eyeRadius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1,y1,z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3,y3,z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3,z3);
            glEnd();
        }
    }
    glPopMatrix();
 
    glPushMatrix();
    glTranslatef(0.25f, 1.0f, 0.3f);  
    glScalef(0.15f, 0.15f, 0.15f);
    
    for (int i = 0; i < eyeRings; i++) {
        float phi1 = (float)i / eyeRings * PI;
        float phi2 = (float)(i + 1) / eyeRings * PI;
        
        for (int j = 0;j < eyeSegments; j++) {
            float theta1 = (float)j / eyeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / eyeSegments * 2.0f * PI;
            float x1 = eyeRadius * sin(phi1) * cos(theta1);
            float y1 = eyeRadius * cos(phi1);
            float z1 =eyeRadius* sin(phi1) *sin(theta1); 
            float x2 = eyeRadius * sin(phi1)* cos(theta2);
            float y2 = eyeRadius *cos(phi1);
            float z2 =eyeRadius * sin(phi1) * sin(theta2);
            
            float x3 =eyeRadius* sin(phi2) * cos(theta1);
            float y3 = eyeRadius *cos(phi2);
            float z3 = eyeRadius * sin(phi2)* sin(theta1);   
            float x4 = eyeRadius* sin(phi2) * cos(theta2);
            float y4= eyeRadius * cos(phi2);
            float z4 = eyeRadius * sin(phi2) *sin(theta2);

            // Calculate normal 
            float nx1 = x1 /eyeRadius;
            float ny1 = y1 / eyeRadius;
            float nz1 = z1 / eyeRadius;
            float nx2 = x2 / eyeRadius;
            float ny2 = y2 / eyeRadius;
            float nz2 =z2 / eyeRadius;
            float nx3= x3 / eyeRadius;
            float ny3 = y3 / eyeRadius;
            float nz3=z3 / eyeRadius;
            float nx4 = x4 / eyeRadius;
            float ny4 =y4 / eyeRadius;
            float nz4= z4 / eyeRadius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1,z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Mouth (white)
    glColor3f(1.0f,1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glPushMatrix();
    glTranslatef(0.0f,0.6f, 0.4f); 
    glRotatef(90.0f,1.0f, 0.0f, 0.0f);  
    glScalef(0.3f, 0.1f,0.1f);
    const int mouthSegments = 32;
    const float mouthRadius= 1.0f;
    
    glBegin(GL_LINE_STRIP);
    for (int i = 0;i <=mouthSegments; i++) {
        float angle = (float)i/ mouthSegments * PI;  
        float x = mouthRadius * cos(angle);
        float y= mouthRadius *sin(angle);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();
    glPopMatrix();
    
    // Left ear (black)
    glColor3f(0.0f,0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(-1.2f, 1.0f,-0.3f);  
    glScalef(0.4f,0.4f, 0.4f);  
 
    for (int i =0; i< rings;i++) {
        float phi1 =(float)i / rings * PI;
        float phi2 = (float)(i + 1) /rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 =(float)j /segments * 2.0f * PI;
            float theta2 = (float)(j + 1) / segments * 2.0f * PI;

            float x1 = radius* sin(phi1) * cos(theta1);
            float y1= radius * cos(phi1);
            float z1 = radius* sin(phi1) *sin(theta1);
            
            float x2 = radius *sin(phi1) * cos(theta2);
            float y2= radius * cos(phi1);
            float z2 = radius *sin(phi1)* sin(theta2);
            
            float x3 = radius * sin(phi2) * cos(theta1);
            float y3= radius* cos(phi2);
            float z3 = radius * sin(phi2)* sin(theta1);
            
            float x4= radius * sin(phi2) *cos(theta2);
            float y4 = radius *cos(phi2);
            float z4= radius* sin(phi2)* sin(theta2);

            // Calculate normal 
            float nx1 = x1 / radius;
            float ny1 = y1 / radius;
            float nz1 = z1 / radius;
            float nx2 = x2 / radius;
            float ny2 = y2 / radius;
            float nz2 = z2 / radius;
            float nx3 =x3 / radius;
            float ny3=y3 / radius;
            float nz3 = z3 / radius;
            float nx4=x4 / radius;
            float ny4 = y4 / radius;
            float nz4=z4 / radius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1,ny1, nz1);
            glVertex3f(x1,y1,z1);
            glNormal3f(nx2,ny2,nz2);
            glVertex3f(x2,y2, z2);
            glNormal3f(nx3,ny3,nz3);
            glVertex3f(x3, y3, z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2, y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4,z4);
            glNormal3f(nx3,ny3,nz3);
            glVertex3f(x3,y3,z3);
            glEnd();
        }
    }
    glPopMatrix();

    // Right ear (black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(1.2f,1.0f, -0.3f);  
    glScalef(0.4f, 0.4f, 0.4f);  
    
    for (int i =0; i< rings;i++) {
        float phi1 = (float)i / rings * PI;
        float phi2 =(float)(i + 1) / rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 = (float)j / segments * 2.0f * PI;
            float theta2 = (float)(j + 1) /segments * 2.0f * PI;

            float x1 = radius * sin(phi1) * cos(theta1);
            float y1 = radius * cos(phi1);
            float z1 = radius *sin(phi1) * sin(theta1);
            
            float x2 = radius * sin(phi1) * cos(theta2);
            float y2 =radius * cos(phi1);
            float z2 = radius * sin(phi1) *sin(theta2);
            float x3 = radius * sin(phi2) * cos(theta1);
            float y3 = radius *cos(phi2);
            float z3 = radius * sin(phi2) *sin(theta1);
            
            float x4 = radius *sin(phi2) * cos(theta2);
            float y4 = radius * cos(phi2);
            float z4 = radius *sin(phi2) * sin(theta2);

            // Calculate normal 
            float nx1 = x1 / radius;
            float ny1 = y1 / radius;
            float nz1 = z1 / radius;
            float nx2 = x2 / radius;
            float ny2 = y2 / radius;
            float nz2 = z2 / radius;
            float nx3 = x3 / radius;
            float ny3 = y3 / radius;
            float nz3 = z3 / radius;
            float nx4 = x4 / radius;
            float ny4 = y4 / radius;
            float nz4 = z4 / radius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2, y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);   
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();

    // Body (yellow)
    glColor3f(1.0f, 0.84f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, yellow);
    glPushMatrix();
    glTranslatef(0.0f,-0.5f, -0.2f);   

    const int bodySegments = 16;
    const float bodyRadius =0.6f;
    const float bodyHeight = 1.0f;
    for (int i = 0;i < bodySegments;i++) {
        float angle1 = (float)i / bodySegments * 2.0f * PI;
        float angle2 = (float)(i + 1)/ bodySegments * 2.0f * PI;
        
        float x1 = bodyRadius * cos(angle1);
        float z1 = bodyRadius * sin(angle1);
        float x2= bodyRadius* cos(angle2);
        float z2 = bodyRadius* sin(angle2);

        // Calculate normals 
        float nx1 = x1 / bodyRadius;
        float nz1= z1 /bodyRadius;
        float nx2 = x2/ bodyRadius;
        float nz2 =z2 /bodyRadius;

        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, bodyHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,bodyHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, bodyHeight,z1);
        glEnd();
    }
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(0.0f, bodyHeight, 0.0f);  
    for (int i = 0; i <= bodySegments; i++) {
        float angle = (float)i / bodySegments* 2.0f * PI;
        float x = bodyRadius * cos(angle);
        float z =bodyRadius * sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, bodyHeight, z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <=bodySegments; i++) {
        float angle = (float)i / bodySegments * 2.0f * PI;
        float x = bodyRadius *cos(angle);
        float z = bodyRadius * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();

    // Shorts (black)
    glColor3f(0.0f,0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f,-0.2f);   
    const int shortsSegments = 16;
    const float shortsRadius =0.5f;
    const float shortsHeight = 0.8f;
    for (int i = 0; i < shortsSegments; i++) {
        float angle1 = (float)i / shortsSegments * 2.0f * PI;
        float angle2= (float)(i + 1)/ shortsSegments * 2.0f * PI;
        
        float x1 = shortsRadius * cos(angle1);
        float z1 = shortsRadius *sin(angle1);
        float x2 = shortsRadius * cos(angle2);
        float z2= shortsRadius * sin(angle2);
        
        // Calculate normals 
        float nx1 = x1 /shortsRadius;
        float nz1 = z1/ shortsRadius;
        float nx2 = x2/shortsRadius;
        float nz2 = z2 /shortsRadius;
        
        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f,z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, shortsHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, shortsHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1,shortsHeight, z1);
        glEnd();
    }

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(0.0f, shortsHeight, 0.0f);  
    for (int i = 0; i <= shortsSegments; i++) {
        float angle = (float)i /shortsSegments * 2.0f * PI;
        float x= shortsRadius * cos(angle);
        float z = shortsRadius * sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, shortsHeight, z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= shortsSegments; i++) {
        float angle = (float)i / shortsSegments * 2.0f * PI;
        float x = shortsRadius * cos(angle);
        float z = shortsRadius* sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();
    
    // Left leg (black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(-0.2f, -1.6f, -0.3f);  
 
    const int legSegments = 16;
    const float legRadius = 0.15f;
    const float legHeight = 0.7f;
    for (int i = 0; i < legSegments; i++) {
        float angle1 = (float)i / legSegments * 2.0f * PI;
        float angle2 = (float)(i + 1) / legSegments * 2.0f * PI;
        
        float x1 = legRadius * cos(angle1);
        float z1= legRadius * sin(angle1);
        float x2 = legRadius* cos(angle2);
        float z2 = legRadius * sin(angle2);

        // Calculate normals 
        float nx1 = x1 / legRadius;
        float nz1 = z1 / legRadius;
        float nx2 = x2 / legRadius;
        float nz2 = z2 / legRadius;

        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, legHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        glEnd();
    }

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(0.0f, legHeight,0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle = (float)i/ legSegments * 2.0f * PI;
        float x = legRadius * cos(angle);
        float z = legRadius *sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, legHeight, z);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle = (float)i /legSegments *2.0f * PI;
        float x = legRadius* cos(angle);
        float z = legRadius * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();

    // Right leg (black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(0.2f, -1.6f, -0.3f);  
   
    for (int i = 0; i < legSegments;i++) {
        float angle1 = (float)i /legSegments * 2.0f *PI;
        float angle2 = (float)(i + 1)/ legSegments* 2.0f * PI;
        
        float x1= legRadius * cos(angle1);
        float z1 = legRadius* sin(angle1);
        float x2 = legRadius * cos(angle2);
        float z2 = legRadius *sin(angle2);

        // Calculate normals 
        float nx1 = x1 / legRadius;
        float nz1 = z1 / legRadius;
        float nx2 = x2 / legRadius;
        float nz2 = z2 / legRadius;

        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1,0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f,z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, legHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1,legHeight, z1);
        glEnd();
    }
 
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);  
    glVertex3f(0.0f, legHeight, 0.0f);  
    for (int i = 0; i<= legSegments; i++) {
        float angle = (float)i/ legSegments * 2.0f * PI;
        float x = legRadius * cos(angle);
        float z= legRadius * sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, legHeight, z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle = (float)i / legSegments * 2.0f * PI;
        float x = legRadius * cos(angle);
        float z = legRadius* sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
    
    // Left shoe (yellow)
    glColor3f(1.0f,1.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, yellow);
    glPushMatrix();
    glTranslatef(-0.2f, -1.6f, -0.3f);  
    const int shoeSegments = 16;
    const int shoeRings = 8;
    const float shoeRadiusX =0.15f;  
    const float shoeRadiusZ = 0.2f;  
    const float shoeHeight = 0.2f;

    for (int i = 0; i < shoeRings; i++) {
        float phi1 = (float)i / shoeRings* PI / 2.0f;  
        float phi2 = (float)(i + 1) / shoeRings * PI / 2.0f;
        
        for (int j = 0; j < shoeSegments; j++) {
            float theta1 = (float)j / shoeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / shoeSegments * 2.0f * PI;

            float x1 =shoeRadiusX * sin(phi1) * cos(theta1);
            float y1 =shoeHeight * cos(phi1);
            float z1 = shoeRadiusZ * sin(phi1) * sin(theta1);
            float x2 = shoeRadiusX * sin(phi1) *cos(theta2);
            float y2 = shoeHeight * cos(phi1);
            float z2 = shoeRadiusZ * sin(phi1) * sin(theta2);
            
            float x3 = shoeRadiusX* sin(phi2) * cos(theta1);
            float y3 =shoeHeight * cos(phi2);
            float z3 = shoeRadiusZ* sin(phi2) * sin(theta1);
            
            float x4 = shoeRadiusX * sin(phi2)*cos(theta2);
            float y4= shoeHeight * cos(phi2);
            float z4 = shoeRadiusZ * sin(phi2)* sin(theta2);
        
            // Calculate normals 
            float nx1 = x1 / shoeRadiusX;
            float ny1 = y1 / shoeHeight;
            float nz1 = z1 / shoeRadiusZ;
            float nx2=x2 / shoeRadiusX;
            float ny2 = y2 / shoeHeight;
            float nz2= z2 / shoeRadiusZ;
            float nx3 = x3 / shoeRadiusX;
            float ny3 = y3 / shoeHeight;
            float nz3 =z3 / shoeRadiusZ;
            float nx4 = x4 / shoeRadiusX;
            float ny4 = y4 / shoeHeight;
            float nz4 =z4 / shoeRadiusZ;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1,z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f); 
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <=shoeSegments; i++) {
        float angle = (float)i / shoeSegments * 2.0f * PI;
        float x =shoeRadiusX * cos(angle);
        float z = shoeRadiusZ* sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
 
    // Right shoe (yellow)
    glColor3f(1.0f,1.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, yellow);
    glPushMatrix();
    glTranslatef(0.2f,-1.6f, -0.3f);  

    for (int i = 0; i < shoeRings; i++) {
        float phi1 = (float)i /shoeRings * PI / 2.0f;  
        float phi2 = (float)(i + 1) / shoeRings *PI / 2.0f;
        
        for (int j = 0; j < shoeSegments; j++) {
            float theta1 = (float)j / shoeSegments * 2.0f *PI;
            float theta2 = (float)(j + 1) /shoeSegments * 2.0f * PI;
            

            float x1 =shoeRadiusX * sin(phi1) * cos(theta1);
            float y1 = shoeHeight * cos(phi1);
            float z1 = shoeRadiusZ *sin(phi1) * sin(theta1);
            
            float x2 = shoeRadiusX * sin(phi1) * cos(theta2);
            float y2 =shoeHeight * cos(phi1);
            float z2 = shoeRadiusZ * sin(phi1) * sin(theta2);
            
            float x3 = shoeRadiusX * sin(phi2) * cos(theta1);
            float y3 = shoeHeight *cos(phi2);
            float z3 = shoeRadiusZ * sin(phi2) *sin(theta1);
            
            float x4 = shoeRadiusX * sin(phi2) * cos(theta2);
            float y4 =shoeHeight * cos(phi2);
            float z4 = shoeRadiusZ * sin(phi2) * sin(theta2);

            // Calculate normals 
            float nx1 = x1 /shoeRadiusX;
            float ny1 = y1 / shoeHeight;
            float nz1 = z1 / shoeRadiusZ;
            float nx2 = x2 /shoeRadiusX;
            float ny2 = y2/ shoeHeight;
            float nz2 = z2 / shoeRadiusZ;
            float nx3 =x3 / shoeRadiusX;
            float ny3=y3 / shoeHeight;
            float nz3 =z3 / shoeRadiusZ;
            float nx4 = x4 / shoeRadiusX;
            float ny4 = y4 / shoeHeight;
            float nz4 = z4/ shoeRadiusZ;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3,y3, z3);
            glEnd();
        }
    }

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);  
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= shoeSegments; i++) {
        float angle =(float)i / shoeSegments * 2.0f * PI;
        float x = shoeRadiusX * cos(angle);
        float z = shoeRadiusZ *sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.6f, 0.0f, -0.3f);  
    glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);  
    drawMickeyHand(0.0f, 0.0f, 0.0f, 1);  
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.6f, 0.0f, -0.3f);  
    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);  
    drawMickeyHand(0.0f, 0.0f, 0.0f, 0);  
    glPopMatrix();
    
    glPopMatrix();
}

void drawMickeyMouseVariant(void) {
    glPushMatrix();
 
    float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float lightBlue[] = {0.5f, 0.8f, 1.0f, 1.0f};
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    
    // Head (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);  
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, -0.3f);  
    glScalef(0.8f, 0.8f, 0.8f); 

    const int segments = 20;
    const int rings = 10;
    const float radius = 1.0f;
    
    for (int i =0; i< rings;i++) {
        float phi1 = (float)i / rings * PI;
        float phi2 = (float)(i + 1) / rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 = (float)j / segments * 2.0f * PI;
            float theta2 = (float)(j + 1) / segments * 2.0f * PI;
            float x1 = radius * sin(phi1) * cos(theta1);
            float y1 =radius * cos(phi1);
            float z1 = radius * sin(phi1)* sin(theta1);
            float x2 = radius * sin(phi1) * cos(theta2);
            float y2 = radius* cos(phi1);
            float z2=radius * sin(phi1)* sin(theta2);
            float x3 = radius * sin(phi2) * cos(theta1);
            float y3= radius * cos(phi2);
            float z3 = radius * sin(phi2)* sin(theta1);
            float x4 = radius * sin(phi2) * cos(theta2);
            float y4 =radius * cos(phi2);
            float z4 = radius *sin(phi2) *sin(theta2);

            // Calculate normals 
            float nx1 = x1 /radius;
            float ny1=y1 / radius;
            float nz1= z1 / radius;
            float nx2 =x2 / radius;
            float ny2 = y2 / radius;
            float nz2 = z2 / radius;
            float nx3 = x3 / radius;
            float ny3 =y3 / radius;
            float nz3 = z3 / radius;
            float nx4 = x4 / radius;
            float ny4 = y4 / radius;
            float nz4 = z4 / radius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Left eye (black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);  
    glPushMatrix();
    glTranslatef(-0.25f, 1.0f, 0.3f);  
    glScalef(0.15f, 0.15f, 0.15f);
    
    const int eyeSegments = 16;
    const int eyeRings = 8;
    const float eyeRadius = 1.0f;
    
    for (int i = 0; i < eyeRings; i++) {
        float phi1 = (float)i / eyeRings * PI;
        float phi2 = (float)(i + 1) / eyeRings * PI;
        
        for (int j = 0; j < eyeSegments; j++) {
            float theta1 = (float)j / eyeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / eyeSegments * 2.0f * PI;
            float x1 = eyeRadius * sin(phi1) * cos(theta1);
            float y1 = eyeRadius* cos(phi1);
            float z1 = eyeRadius * sin(phi1) *sin(theta1);
            float x2 = eyeRadius * sin(phi1) * cos(theta2);
            float y2 = eyeRadius * cos(phi1);
            float z2 = eyeRadius* sin(phi1) * sin(theta2);
            
            float x3 = eyeRadius * sin(phi2) * cos(theta1);
            float y3 =eyeRadius * cos(phi2);
            float z3 = eyeRadius * sin(phi2) * sin(theta1);
            
            float x4 = eyeRadius * sin(phi2) * cos(theta2);
            float y4 =eyeRadius * cos(phi2);
            float z4 = eyeRadius * sin(phi2) *sin(theta2);

            // Calculate normals 
            float nx1 = x1 / eyeRadius;
            float ny1 = y1/eyeRadius;
            float nz1 = z1 / eyeRadius;
            float nx2 = x2/eyeRadius;
            float ny2 = y2/eyeRadius;
            float nz2 = z2 / eyeRadius;
            float nx3 =x3 / eyeRadius;
            float ny3= y3 / eyeRadius;
            float nz3 =z3 / eyeRadius;
            float nx4= x4 / eyeRadius;
            float ny4= y4 / eyeRadius;
            float nz4 =z4 / eyeRadius;

            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3,y3, z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Right eye (black)
    glPushMatrix();
    glTranslatef(0.25f, 1.0f, 0.3f);  
    glScalef(0.15f, 0.15f, 0.15f);
    
    for (int i = 0; i < eyeRings; i++) {
        float phi1 = (float)i / eyeRings * PI;
        float phi2 = (float)(i + 1) / eyeRings * PI;
        
        for (int j = 0; j < eyeSegments; j++) {
            float theta1 = (float)j / eyeSegments * 2.0f * PI;
            float theta2 =(float)(j + 1) / eyeSegments * 2.0f * PI;
            float x1 =eyeRadius * sin(phi1) * cos(theta1);
            float y1=eyeRadius * cos(phi1);
            float z1= eyeRadius * sin(phi1) * sin(theta1);
            float x2=eyeRadius *sin(phi1) * cos(theta2);
            float y2=eyeRadius * cos(phi1);
            float z2=eyeRadius * sin(phi1) * sin(theta2);
            
            float x3 = eyeRadius *sin(phi2) * cos(theta1);
            float y3= eyeRadius * cos(phi2);
            float z3 = eyeRadius * sin(phi2) *sin(theta1);
            float x4 = eyeRadius * sin(phi2) * cos(theta2);
            float y4 = eyeRadius * cos(phi2);
            float z4 = eyeRadius*sin(phi2) * sin(theta2);

            // Calculate normals 
            float nx1 = x1 / eyeRadius;
            float ny1 = y1 / eyeRadius;
            float nz1 = z1 / eyeRadius;
            float nx2 =x2 / eyeRadius;
            float ny2 = y2 / eyeRadius;
            float nz2= z2 / eyeRadius;
            float nx3 = x3 / eyeRadius;
            float ny3 =y3 / eyeRadius;
            float nz3 = z3 / eyeRadius;
            float nx4 = x4 / eyeRadius;
            float ny4 = y4 / eyeRadius;
            float nz4 = z4 / eyeRadius;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3,z3);
            
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3,y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Mouth
    glColor3f(0.0f, 0.0f, 0.0f);  
    glPushMatrix();
    glTranslatef(0.0f, 0.6f, 0.3f); 
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); 
    glScalef(0.3f, 0.1f, 0.1f);
    
    const int mouthSegments = 32;
    const float mouthRadius = 1.0f;
 
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= mouthSegments; i++) {
        float angle = (float)i / mouthSegments * PI;  
        float x = mouthRadius * cos(angle);
        float y = mouthRadius * sin(angle);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();
    
    glPopMatrix();
    
    // Left ear (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glPushMatrix();
    glTranslatef(-1.2f, 1.0f, -0.3f);  
    glScalef(0.4f, 0.4f, 0.4f);  
 
    for (int i =0; i< rings;i++) {
        float phi1 = (float)i / rings * PI;
        float phi2 = (float)(i + 1) / rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 = (float)j / segments * 2.0f * PI;
            float theta2 = (float)(j + 1) /segments * 2.0f *PI;

            float x1= radius * sin(phi1) * cos(theta1);
            float y1 = radius* cos(phi1);
            float z1 = radius *sin(phi1) * sin(theta1);
            float x2= radius * sin(phi1) * cos(theta2);
            float y2=radius * cos(phi1);
            float z2 =radius* sin(phi1) * sin(theta2);
            
            float x3= radius* sin(phi2) * cos(theta1);
            float y3 = radius * cos(phi2);
            float z3 =radius *sin(phi2) * sin(theta1); 
            float x4 = radius * sin(phi2) * cos(theta2);
            float y4 = radius * cos(phi2);
            float z4= radius*sin(phi2) *sin(theta2);
            
            // Calculate normals 
            float nx1 = x1 / radius;
            float ny1 = y1 / radius;
            float nz1 =z1 / radius;
            float nx2 = x2/radius;
            float ny2 = y2 / radius;
            float nz2=z2 / radius;
            float nx3 = x3 / radius;
            float ny3 = y3 / radius;
            float nz3 = z3 / radius;
            float nx4=x4 / radius;
            float ny4 = y4 / radius;
            float nz4=z4/radius;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1,z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4,y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();

    // Right ear (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glPushMatrix();
    glTranslatef(1.2f, 1.0f, -0.3f);  
    glScalef(0.4f,0.4f, 0.4f);  
    
    for (int i = 0;i < rings; i++) {
        float phi1 =(float)i / rings * PI;
        float phi2 = (float)(i + 1) / rings * PI;
        
        for (int j = 0; j < segments; j++) {
            float theta1 = (float)j / segments * 2.0f * PI;
            float theta2 = (float)(j + 1) / segments * 2.0f * PI;
            float x1 = radius *sin(phi1) * cos(theta1);
            float y1 = radius * cos(phi1);
            float z1 = radius*sin(phi1) * sin(theta1);
            float x2 = radius* sin(phi1) * cos(theta2);
            float y2 = radius * cos(phi1);
            float z2=radius * sin(phi1)* sin(theta2);
            float x3 = radius * sin(phi2) * cos(theta1);
            float y3 = radius * cos(phi2);
            float z3 = radius * sin(phi2) * sin(theta1);
            
            float x4 = radius * sin(phi2) * cos(theta2);
            float y4 = radius * cos(phi2);
            float z4= radius *sin(phi2) *sin(theta2);
            
            // Calculate normals 
            float nx1 = x1 / radius;
            float ny1 = y1 / radius;
            float nz1 = z1 / radius;
            float nx2 = x2 /radius;
            float ny2 = y2/ radius;
            float nz2 = z2 / radius;
            float nx3 =x3 / radius;
            float ny3 = y3 / radius;
            float nz3=z3 / radius;
            float nx4 = x4 / radius;
            float ny4=y4 / radius;
            float nz4 = z4 / radius;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }
    glPopMatrix();
    
    // Body 
    glColor3f(0.5f, 0.8f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lightBlue);  
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, -0.2f);   

    const int bodySegments = 16;
    const float bodyRadius = 0.6f;
    const float bodyHeight = 1.0f;
    for (int i = 0; i < bodySegments; i++) {
        float angle1 = (float)i / bodySegments * 2.0f * PI;
        float angle2 = (float)(i + 1) / bodySegments * 2.0f * PI;
        
        float x1 = bodyRadius *cos(angle1);
        float z1 = bodyRadius * sin(angle1);
        float x2 =bodyRadius * cos(angle2);
        float z2 = bodyRadius * sin(angle2);

        // Calculate normals 
        float nx1 = x1 / bodyRadius;
        float nz1 =z1 / bodyRadius;
        float nx2=x2/bodyRadius;
        float nz2 = z2/bodyRadius;
        
        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1,bodyHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, bodyHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, bodyHeight, z1);
        glEnd();
    }

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, bodyHeight, 0.0f);  
    for (int i = 0; i<= bodySegments; i++) {
        float angle = (float)i / bodySegments * 2.0f * PI;
        float x = bodyRadius * cos(angle);
        float z = bodyRadius *sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, bodyHeight, z);
    }
    glEnd();

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= bodySegments; i++) {
        float angle = (float)i / bodySegments * 2.0f * PI;
        float x =bodyRadius * cos(angle);
        float z = bodyRadius*sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();

    glPopMatrix();
    
    // Shorts (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f, -0.2f);   
    const int shortsSegments = 16;
    const float shortsRadius = 0.5f;
    const float shortsHeight = 0.8f;
    for (int i = 0; i < shortsSegments; i++) {
        float angle1 =(float)i / shortsSegments* 2.0f* PI;
        float angle2 = (float)(i + 1)/ shortsSegments *2.0f * PI;
        
        float x1 = shortsRadius *cos(angle1);
        float z1 = shortsRadius * sin(angle1);
        float x2 = shortsRadius* cos(angle2);
        float z2 =shortsRadius * sin(angle2);

        // Calculate normals 
        float nx1 = x1 / shortsRadius;
        float nz1=z1/shortsRadius;
        float nx2=x2 /shortsRadius;
        float nz2 = z2/ shortsRadius;
        
        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, shortsHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, shortsHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, shortsHeight, z1);
        glEnd();
    }

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f,shortsHeight, 0.0f);  
    for (int i = 0;i <= shortsSegments; i++) {
        float angle = (float)i /shortsSegments * 2.0f * PI;
        float x = shortsRadius* cos(angle);
        float z= shortsRadius *sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, shortsHeight, z);
    }
    glEnd();
    
    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= shortsSegments; i++) {
        float angle =(float)i / shortsSegments * 2.0f * PI;
        float x = shortsRadius * cos(angle);
        float z = shortsRadius * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
    
    // Left leg 
    glColor3f(0.5f, 0.8f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lightBlue);
    glPushMatrix();
    glTranslatef(-0.2f, -1.6f, -0.3f);  

    const int legSegments = 16;
    const float legRadius= 0.15f;
    const float legHeight = 0.7f;
    
    for (int i = 0; i < legSegments; i++) {
        float angle1 = (float)i/ legSegments * 2.0f * PI;
        float angle2 = (float)(i + 1) / legSegments * 2.0f * PI;
        
        float x1 = legRadius * cos(angle1);
        float z1=legRadius * sin(angle1);
        float x2 = legRadius * cos(angle2);
        float z2 = legRadius * sin(angle2);

        // Calculate normals 
        float nx1 = x1 / legRadius;
        float nz1 =z1 / legRadius;
        float nx2 =x2/legRadius;
        float nz2=z2 / legRadius;
        
        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, legHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        glEnd();
    }

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, legHeight, 0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle = (float)i / legSegments * 2.0f * PI;
        float x = legRadius * cos(angle);
        float z = legRadius * sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, legHeight, z);
    }
    glEnd();
    
    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle = (float)i / legSegments * 2.0f * PI;
        float x = legRadius * cos(angle);
        float z = legRadius * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();
    
    // Right leg 
    glColor3f(0.5f, 0.8f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lightBlue);
    glPushMatrix();
    glTranslatef(0.2f,-1.6f, -0.3f);  
   
    for (int i = 0; i < legSegments; i++) {
        float angle1 = (float)i / legSegments * 2.0f * PI;
        float angle2 = (float)(i + 1)/ legSegments * 2.0f * PI;
        
        float x1 = legRadius* cos(angle1);
        float z1=legRadius * sin(angle1);
        float x2 = legRadius * cos(angle2);
        float z2 = legRadius * sin(angle2);

        // Calculate normals 
        float nx1 =x1 / legRadius;
        float nz1 =z1/legRadius;
        float nx2=x2 / legRadius;
        float nz2=z2 / legRadius;
        
        glBegin(GL_TRIANGLES);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, 0.0f, z1);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,0.0f, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight, z1);
        
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2, 0.0f, z2);
        glNormal3f(nx2, 0.0f, nz2);
        glVertex3f(x2,legHeight, z2);
        glNormal3f(nx1, 0.0f, nz1);
        glVertex3f(x1, legHeight,z1);
        glEnd();
    }
 
    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, legHeight, 0.0f);  
    for (int i = 0; i <= legSegments; i++) {
        float angle= (float)i / legSegments* 2.0f * PI;
        float x = legRadius * cos(angle);
        float z =legRadius *sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, legHeight, z);
    }
    glEnd();
    
    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i= 0; i <= legSegments; i++) {
        float angle = (float)i /legSegments * 2.0f * PI;
        float x =legRadius* cos(angle);
        float z= legRadius * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
    
    // Left shoe
    glColor3f(0.0f,0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);  
    glPushMatrix();
    glTranslatef(-0.2f, -1.6f,-0.3f);  
    const int shoeSegments = 16;
    const int shoeRings = 8;
    const float shoeRadiusX = 0.15f;  
    const float shoeRadiusZ = 0.2f;  
    const float shoeHeight= 0.2f;

    for (int i = 0; i<shoeRings; i++) {
        float phi1 = (float)i /shoeRings * PI / 2.0f;  
        float phi2 =(float)(i + 1) /shoeRings * PI/ 2.0f;
        
        for (int j = 0; j < shoeSegments; j++) {
            float theta1 = (float)j /shoeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / shoeSegments * 2.0f * PI;
            float x1 = shoeRadiusX * sin(phi1) * cos(theta1);
            float y1 =shoeHeight * cos(phi1);
            float z1 = shoeRadiusZ * sin(phi1) * sin(theta1);
            float x2=shoeRadiusX* sin(phi1) * cos(theta2);
            float y2 = shoeHeight * cos(phi1);
            float z2 =shoeRadiusZ * sin(phi1)* sin(theta2);
            
            float x3 = shoeRadiusX * sin(phi2) * cos(theta1);
            float y3 = shoeHeight *cos(phi2);
            float z3=shoeRadiusZ * sin(phi2) *sin(theta1);
            float x4 = shoeRadiusX*sin(phi2) * cos(theta2);
            float y4=shoeHeight * cos(phi2);
            float z4 =shoeRadiusZ * sin(phi2) * sin(theta2);
        
            // Calculate normals 
            float nx1 = x1 / shoeRadiusX;
            float ny1 = y1 / shoeHeight;
            float nz1 =z1 / shoeRadiusZ;
            float nx2 = x2 / shoeRadiusX;
            float ny2= y2 / shoeHeight;
            float nz2 = z2 / shoeRadiusZ;
            float nx3=x3 / shoeRadiusX;
            float ny3 = y3 / shoeHeight;
            float nz3 =z3/shoeRadiusZ;
            float nx4 = x4 / shoeRadiusX;
            float ny4=y4 / shoeHeight;
            float nz4 = z4/shoeRadiusZ;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);  
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i<= shoeSegments; i++) {
        float angle = (float)i /shoeSegments * 2.0f * PI;
        float x = shoeRadiusX * cos(angle);
        float z= shoeRadiusZ * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();

    // Right shoe (black)
    glColor3f(0.0f,0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
    glTranslatef(0.2f, -1.6f, -0.3f);  

    for (int i = 0; i < shoeRings; i++) {
        float phi1 = (float)i / shoeRings * PI / 2.0f;  
        float phi2 =(float)(i + 1)/ shoeRings* PI / 2.0f;
        
        for (int j = 0; j < shoeSegments; j++) {
            float theta1 = (float)j / shoeSegments * 2.0f * PI;
            float theta2 = (float)(j + 1) / shoeSegments * 2.0f * PI;
            
            float x1 = shoeRadiusX * sin(phi1) * cos(theta1);
            float y1 =shoeHeight * cos(phi1);
            float z1 = shoeRadiusZ *sin(phi1) * sin(theta1);
            
            float x2 =shoeRadiusX * sin(phi1) * cos(theta2);
            float y2= shoeHeight * cos(phi1);
            float z2 =shoeRadiusZ *sin(phi1) *sin(theta2);
            
            float x3 = shoeRadiusX * sin(phi2) * cos(theta1);
            float y3 =shoeHeight * cos(phi2);
            float z3 = shoeRadiusZ * sin(phi2)*sin(theta1);
            float x4 = shoeRadiusX *sin(phi2) * cos(theta2);
            float y4= shoeHeight * cos(phi2);
            float z4 = shoeRadiusZ *sin(phi2) * sin(theta2);
            // Calculate normals 
            float nx1 =x1 / shoeRadiusX;
            float ny1 = y1 / shoeHeight;
            float nz1 = z1 / shoeRadiusZ;
            float nx2= x2 / shoeRadiusX;
            float ny2 = y2 / shoeHeight;
            float nz2=z2 / shoeRadiusZ;
            float nx3 = x3 / shoeRadiusX;
            float ny3 = y3 / shoeHeight;
            float nz3=z3/ shoeRadiusZ;
            float nx4 = x4 /shoeRadiusX;
            float ny4 = y4/ shoeHeight;
            float nz4 = z4/shoeRadiusZ;
            
            glBegin(GL_TRIANGLES);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(x2,y2,z2);
            glNormal3f(nx4, ny4, nz4);
            glVertex3f(x4, y4, z4);
            glNormal3f(nx3, ny3, nz3);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f,0.0f,0.0f);  
    for (int i = 0; i <= shoeSegments; i++) {
        float angle = (float)i / shoeSegments * 2.0f * PI;
        float x = shoeRadiusX * cos(angle);
        float z = shoeRadiusZ * sin(angle);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
    
    // Arms 
    glColor3f(0.5f, 0.8f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lightBlue);  
    glPushMatrix();
    glTranslatef(-0.6f, 0.0f, -0.3f);  
    glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);  
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);  
    drawForearm(0.0f, 0.0f, 0.0f, 1.0f, 0.1f);  
    glPopMatrix();
    glColor3f(0.5f, 0.8f, 1.0f);  
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);  
    drawPalm(0.0f, 0.9f, 0.0f); 
    glPopMatrix();
    
    glPopMatrix();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.6f, 0.0f, -0.3f);  
    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);  
    
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);  
    drawForearm(0.0f, 0.0f, 0.0f, 1.0f, 0.1f);  
    glPopMatrix();
    glColor3f(0.5f, 0.8f, 1.0f);  
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);  
    drawPalm(0.0f, 0.9f, 0.0f); 
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

// Animation function 
void idle(void) {
    static float orbitalLastTime = 0;
    
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    
    if (orbitalMove) {
        float orbitalDeltaTime = currentTime - orbitalLastTime;
        float adjustedSpeed = orbitalSpeed * (15.0f / (float)currentEffectiveDistance);
        orbitalZh += adjustedSpeed * orbitalDeltaTime;
        if (orbitalZh >= 360.0f) orbitalZh -= 360.0f;
        if (orbitalZh < 0) orbitalZh += 360.0f;
    }
    
    float rotationSpeed = 0.5f;
    if (!stadiumAnimationPaused) {
        stadiumTextureOffset = currentTime * rotationSpeed;
        while (stadiumTextureOffset >= 8.0f) stadiumTextureOffset -=8.0f;
        while (stadiumTextureOffset < 0.0f) stadiumTextureOffset+= 8.0f;
    }
    
    orbitalLastTime = currentTime;
    
    glutPostRedisplay();
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    if (currentViewMode == VIEW_ORTHOGONAL||currentViewMode ==VIEW_PERSPECTIVE) 
    {
        glTranslatef(cameraX, cameraY, -cameraZ);
        glRotatef(rotationX, 1.0f, 0.0f,0.0f);
        glRotatef(rotationY,0.0f, 1.0f, 0.0f);
        glScalef(zoomScale, zoomScale, zoomScale); 
    }
     else if (currentViewMode ==VIEW_FIRST_PERSON) 
     {
        updateFirstPersonLookAt();
        lookAt(fpCameraX, fpCameraY, fpCameraZ,fpLookAtX, fpLookAtY, fpLookAtZ, fpUpX, fpUpY, fpUpZ);
    }

    // Lighting setup
    glShadeModel(orbitalSmooth ? GL_SMOOTH : GL_FLAT);
    
    float zeroAmbient[] = {0.0f, 0.0f,0.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zeroAmbient);
    
    float sceneCenterX= 0.0f;
    float sceneCenterY =0.0f;
    float sceneCenterZ= 0.0f;
    int useCustomDistance= 0;
    int customDistance =50;
    
    if (obj==0)
    {
        sceneCenterX= 0.0f;
        sceneCenterY=0.0f;
        sceneCenterZ =0.0f;
    }
    else if (obj==1)
    {
        useCustomDistance = 1;
        customDistance =15;
        drawAxes(15.0f);
        glPushMatrix();
        glTranslatef(0.0f,1.0f,0.0f);  
        drawSwimmingPool();
        glPopMatrix();
        
    }
    else if (obj==2)
    {
        useCustomDistance = 1;
        customDistance = 25;
        drawAxes(15.0f);
        drawStadiumField(8.0f, 6.0f);
        drawSoccerGoalPost(-4.0f,0.0f,1.8f, 1.2f);
        drawSoccerGoalPost(4.0f, 0.0f, 1.8f,1.2f);
        drawStadiumTier(0.0f, 0.0f,6.0f,4.5f, 12, 64);
        drawStadiumWalls(0.0f, 0.0f,16.0f,12.0f,4.0f,64);
        drawLightTower(-13.0f,-10.0f,8.0f);
        drawLightTower(13.0f,-10.0f,8.0f);
        drawLightTower(-13.0f, 10.0f,8.0f);
        drawLightTower(13.0f,10.0f, 8.0f);
    }
    else if (obj==3)
    {
        useCustomDistance =1;
        customDistance=15;
        drawAxes(15.0f);
        glPushMatrix();
        glTranslatef(0.0f,0.0f,-18.0f);
        drawOlympicCauldron(0.0f,0.0f, 18.0f);
        glPopMatrix();
    }
    else if (obj==4)
    {
        useCustomDistance=1;
        customDistance = 20;
        drawAxes(15.0f);
        glPushMatrix();
        glTranslatef(0.0f,0.1f,0.0f);  
        drawSkyscraper(0.0f, 0.0f,4.0f,3.0f, 15.0f,25);
        glPopMatrix();
    }
    else if (obj==5)
    {
       
        useCustomDistance = 1;
        customDistance =40;
        drawAxes(18.0f);
        glPushMatrix();
        glTranslatef(0.0f,0.1f,0.0f);
        drawMountain(0.0f, 0.0f,120, 20, 35);
        glPopMatrix();
    }
    else if (obj==6)
    {
        useCustomDistance = 1;
        customDistance =10;
        drawAxes(15.0f);
        glPushMatrix();
        glTranslatef(0.0f,1.0f,0.0f); 
        drawMickeyMouse();
        glPopMatrix();
        
    }
    else if (obj==7)
    {
        useCustomDistance =1;
        customDistance = 12;
        drawAxes(15.0f);
        drawPalmTree();
    }
    else if (obj==8)
    {
        useCustomDistance = 1;
        customDistance = 10;
        drawAxes(15.0f);
        drawTicketBooth();
    }
    else
    {
        useCustomDistance=1;
        customDistance =8;
        drawAxes(15.0f);
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, 0.0f); 
        drawOlympicRings();
        glPopMatrix();
    }
    
    
    // Large orbital light setup
    if (orbitalLight) {
    
        float OrbitalAmbient[] = {0.01f * orbitalAmbient,0.01f * orbitalAmbient,0.01f * orbitalAmbient, 1.0f};
        float OrbitalDiffuse[] = {0.01f*orbitalDiffuse, 0.01f*orbitalDiffuse, 0.01f*orbitalDiffuse, 1.0f};
        float OrbitalSpecular[]={0.01f*orbitalSpecular, 0.01f *orbitalSpecular,0.01f * orbitalSpecular, 1.0f};
        int effectiveDistance = useCustomDistance ? customDistance : orbitalDistance;
        currentEffectiveDistance = effectiveDistance;
        
        float orbitalLightX=sceneCenterX + effectiveDistance * cos(orbitalZh * PI / 180.0f);
        float orbitalLightY=sceneCenterY + orbitalYlight;
        float orbitalLightZ =sceneCenterZ + effectiveDistance * sin(orbitalZh * PI / 180.0f);
        float OrbitalPosition[] = {orbitalLightX, orbitalLightY, orbitalLightZ, 1.0f};
        
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);  
        drawLightSphere(orbitalLightX, orbitalLightY, orbitalLightZ, 1.0f);  
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2,GL_AMBIENT,OrbitalAmbient);
        glLightfv(GL_LIGHT2,GL_DIFFUSE,OrbitalDiffuse);
        glLightfv(GL_LIGHT2,GL_SPECULAR,OrbitalSpecular);
        glLightfv(GL_LIGHT2,GL_POSITION,OrbitalPosition);
    } 
    else
    {
      
        glDisable(GL_LIGHT2);
    }

    if (!orbitalLight) 
    {
        glDisable(GL_LIGHTING);
    } 
    else {
        glEnable(GL_LIGHTING);
    }
    
    if (obj == 0)
    {
        drawMountain(0.0f,-25.0f,120, 20,35);
        drawAthleticsStadium();
        drawOlympicCauldron(0.0f,0.0f, 18.0f);
        drawSkyscraper(-35.0f, -8.0f, 4.0f, 3.0f, 15.0f, 25); 
        drawSkyscraper(-25.0f, -12.0f, 3.5f, 2.8f, 14.0f, 22); 
        drawSkyscraper(-20.0f, -16.0f, 3.0f, 2.5f, 12.5f, 20); 
        drawSkyscraper(-30.0f, -20.0f, 2.5f, 2.0f, 10.0f, 16);  
        drawSkyscraper(-17.0f, -8.0f, 2.0f, 1.8f, 9.0f, 14);  
        glPushMatrix();
        glTranslatef(20.0f, 1.5f,16.0f);  
        drawSwimmingPool();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(5.0f,1.7f,25.0f); 
        drawMickeyMouse();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(12.0f,1.7f, 25.0f);  
        drawMickeyMouseVariant();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-20.0f, 0.0f, 16.0f);
        drawTicketBooth();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-8.0f, 0.0f, -16.0f);
        drawPalmTree();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(8.0f,0.0f, -16.0f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.2f, 1.2f,1.2f);
        drawPalmTree();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-8.0f,0.0f, 16.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(0.8f,0.8f, 0.8f);
        drawPalmTree();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(8.0f, 0.0f,16.0f);
        glRotatef(180.0f,0.0f, 1.0f, 0.0f);
        glScalef(1.1f, 1.3f, 1.1f);
        drawPalmTree();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f,0.0f, -18.0f);
        glScalef(1.0f, 1.4f,1.0f);
        drawPalmTree();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f,12.0f, -10.0f);
        glScalef(0.8f, 0.8f,0.8f);
        drawOlympicRings();
        glPopMatrix();
    }
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0,glutGet(GLUT_WINDOW_HEIGHT),-1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT)- 20);
    
    const char* modeText;
    if (currentViewMode==VIEW_ORTHOGONAL) 
    {
        modeText ="View Mode: Orthogonal (Press 'm' to switch)";
    }
    else if (currentViewMode == VIEW_PERSPECTIVE)
     {
        modeText = "View Mode: Perspective (Press 'm' to switch)";
    } 
     else if(currentViewMode == VIEW_FIRST_PERSON)
      {
         modeText = "View Mode: First Person";
     }
    else
    {
        modeText ="View Mode: Unknown (Press 'm' to switch)";
    }
    
     for (const char* c= modeText;*c; c++) 
     {
         glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
     } 
    
     glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 40);
     const char* commonControls = "Press 'r': reset, 'l/L': cycle objects, 'h': help";
     for (const char* c = commonControls; *c; c++) 
     {
         glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
     }
     
     // Display orbital lighting information
     if (orbitalLight) {
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 60);
         char orbitalLightInfo[200];
         sprintf(orbitalLightInfo, "Orbital Light: ON  Ambient:%d%%  Diffuse:%d%%  Specular:%d%%  Shininess:%.0f",orbitalAmbient,orbitalDiffuse,orbitalSpecular,shiny);
         for (const char* c = orbitalLightInfo; *c; c++) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
         
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 80);
         sprintf(orbitalLightInfo, "Orbital Pos: (%.1f, %.1f, %.1f)  Distance:%d  Elevation:%.1f", orbitalDistance * cos(orbitalZh * PI / 180.0f), orbitalYlight, orbitalDistance * sin(orbitalZh * PI / 180.0f), orbitalDistance, orbitalYlight);
         for (const char* c = orbitalLightInfo; *c; c++) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
     } else {
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 60);
         const char* orbitalLightOff = "Orbital Light: OFF";
         for (const char* c = orbitalLightOff; *c; c++) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
     }
     
     if (currentViewMode == VIEW_FIRST_PERSON) 
     {
         glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT)- 160);
         const char* controls1 ="Forward : press up arrow";
         for (const char* c = controls1; *c; c++)
          {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
         }

         
         glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT)- 180);
         const char* controls2 ="Backward : press down arrow";
         for (const char* c = controls2; *c; c++)
          {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
         }
         glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT)- 200);
         const char* controls5 ="Press 'q' : Turn left, Press 'e': Turn right";
         for (const char* c = controls5; *c; c++)
          {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
         }
         glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT)- 220);
         const char* controls6 ="Press 'z' : Turn up, Press 'x': Turn down";
         for (const char* c = controls6; *c; c++)
          {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
         }
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 240);
         const char* controls4 = "For changing view direction, hold left mouse button and move mouse";
         for (const char* c = controls4; *c; c++) 
         {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT)- 260);
         const char* controls3 = "+ or -: speed, 1 or 2: sensitivity,'h':help";
         for (const char* c = controls3; *c; c++) 
         {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
         glRasterPos2f(10,glutGet(GLUT_WINDOW_HEIGHT) - 280);
        
        char cameraText[200];
        sprintf(cameraText, "Camera Pos:(%.1f, %.1f, %.1f)", fpCameraX,fpCameraY, fpCameraZ);
        for (const char* c = cameraText; *c; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
        }
        
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT)- 300);
         
         sprintf(cameraText, "Look At:(%.1f, %.1f, %.1f)",fpLookAtX, fpLookAtY, fpLookAtZ);
         for (const char* c =cameraText; *c; c++) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 320);
         sprintf(cameraText, "Up Vector: (%.1f, %.1f, %.1f)", fpUpX,fpUpY, fpUpZ); 
         for (const char* c =cameraText; *c; c++) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
         }
         
         glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 340);
        
        sprintf(cameraText, "Yaw: %.1f, Pitch: %.1f", fpYaw,fpPitch);
        for (const char* c =cameraText; *c; c++) 
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
     }
    if (currentViewMode == VIEW_ORTHOGONAL||currentViewMode==VIEW_PERSPECTIVE) {
        glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT)-100);
        char rotationText[100];
        sprintf(rotationText, "Rotation X: %.1f,Rotation Y: %.1f",rotationX,rotationY);
        
        for (const char* c = rotationText; *c; c++) 
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    } 
     if (currentViewMode ==VIEW_FIRST_PERSON) 
     {
         
    }

    if (orbitalLight) {
        glEnable(GL_LIGHTING);
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glutSwapBuffers();
}
void keyboard(unsigned char key,int x,int y) 
{
    (void)x; (void)y;
    switch (key) 
    {
        case 27: 
            exit(0);
            break;
        case 'r':
        case 'R':
            if (currentViewMode == VIEW_FIRST_PERSON)
             {
                
                fpCameraX =-4.0f;
                fpCameraY = 3.8f;
                fpCameraZ =36.9f;
                fpLookAtX= -3.9f;
                fpLookAtY = 3.9f;
                fpLookAtZ= 35.9f;
                fpUpX = 0.0f;
                fpUpY = 1.0f;
                fpUpZ =0.0f;
                fpYaw= -187.5f;
                fpPitch = 6.0f;
            }
            else 
            {
                cameraX =0.0f;
                cameraY = 0.0f;
                cameraZ = 50.0f;
                rotationX= 20.0f;
                rotationY =-1.0f;
            }
            break;
        case 'm':
        case 'M':
            currentViewMode = (currentViewMode +1)%3; 
            printf("Switched to view mode: %d\n", currentViewMode);
            int width = glutGet(GLUT_WINDOW_WIDTH);
            int height = glutGet(GLUT_WINDOW_HEIGHT);
            reshape(width, height);
            break;
        case '+':
            if (currentViewMode==VIEW_FIRST_PERSON) 
            {
                fpMoveSpeed+= 0.1f;
                if (fpMoveSpeed >2.0f) fpMoveSpeed =2.0f;
                printf("Movement speed: %.1f\n",fpMoveSpeed);
            }
            else 
            {
                zoomScale*=1.1f;
                if (zoomScale > 5.0f) zoomScale=5.0f;
               
            }
            break;
        case '-':
            if (currentViewMode ==VIEW_FIRST_PERSON) 
            {
                fpMoveSpeed-= 0.1f;
                if (fpMoveSpeed < 0.1f) fpMoveSpeed = 0.1f;
                printf("Movement speed: %.1f\n", fpMoveSpeed);
            }
            else 
            {
                zoomScale/=1.1f;
                if (zoomScale < 0.1f) zoomScale = 0.1f;
            }
            break;
        case '1':
            if(currentViewMode == VIEW_FIRST_PERSON) {
                fpMouseSensitivity -= 0.05f;
                if (fpMouseSensitivity < 0.05f) fpMouseSensitivity = 0.05f;
                printf("Mouse sensitivity: %.2f\n",fpMouseSensitivity);
            }
            break;
        case '2':
            if (currentViewMode ==VIEW_FIRST_PERSON) {
                fpMouseSensitivity+= 0.05f;
                if (fpMouseSensitivity > 1.0f) fpMouseSensitivity = 1.0f;
                printf("Mouse sensitivity: %.2f\n",fpMouseSensitivity);
            }
            break;
        case 'e':
        case 'E': 
            if (currentViewMode== VIEW_FIRST_PERSON)
             {
                fpYaw -=fpRotationSpeed;
                updateFirstPersonLookAt();
            } 
                
            break;
        case 'q':
        case 'Q':
            if (currentViewMode== VIEW_FIRST_PERSON) 
            {
                fpYaw += fpRotationSpeed;
                updateFirstPersonLookAt();
            }
            break;
        case 'z':
        case 'Z':
            if (currentViewMode== VIEW_FIRST_PERSON) 
            {
                fpPitch+= fpRotationSpeed;
                if (fpPitch > 89.0f) fpPitch =89.0f;
                updateFirstPersonLookAt();

            }
            break;
        case 'x':
        case 'X':
            if (currentViewMode == VIEW_FIRST_PERSON) 
            {
                fpPitch -= fpRotationSpeed;
                if (fpPitch < -85.0f) fpPitch = -85.0f;
                updateFirstPersonLookAt();
            }
            break;
        case 'h':
        case 'H':

            printf("\nObject Selection:\n");
            printf("l/L: Cycle through objects/scenes\n");
            
            printf("\nOrbital Light Controls:\n");
            printf("o/O: Toggle orbital light on/off\n");
            printf("p/P: Toggle automatic orbital light orbiting\n");
            printf("y/Y: Lower/rise orbital light elevation\n");
            printf("t/T: Rotate orbital light position\n");
            printf("i/I: Decrease/increase orbital ambient light\n");
            printf("j/J: Decrease/increase orbital diffuse light\n");
            printf("k/K: Decrease/increase orbital specular light\n");
            printf("b/B: Decrease/increase orbital shininess\n");
            printf("g: Toggle orbital light distance (10/50)- This works only in the full scene not in individual objects\n");
            printf("G: Toggle orbital smooth/flat shading: look at any curved objects(like Olympic rings)\n");
             printf("w/W: Reset orbital light to default position\n");
            printf("s/S: Toggle stadium wall animation pause/play\n");
            printf("c/C: Cycle ticket booth roof color (white -> red -> blue -> green)\n");
            printf("n: Increase skyscraper texture repetition\n");
            printf("N: Decrease skyscraper texture repetition\n");
            printf("b: Expand tree (increase height and leaf size)\n");
            printf("B: Shrink tree (decrease height and leaf size)\n");
            printf("ESC to exit\n");

            printf("\nFirst-Person Controls\n");
            printf("Movement: Arrow keys (Up/Down:forward/back, Left/Right: strafe)\n");
            printf("Looking: Hold left mouse button and move mouse\n");
            printf("Keyboard Controls: Q/E (turn left/right), Z/X (look up/down) - This work in first person view\n");
            printf("Speed: + or - to adjust movement speed (The current Speed: %.1f)\n", fpMoveSpeed);
            printf("Sensitivity: 1 or 2 to adjust mouse sensitivity (The Current Mouse sensitivity: %.2f)\n",fpMouseSensitivity);
            printf("Mode: 'm' to switch view modes\n");
            printf("Reset: 'r' to reset camera position\n");
            printf("Zoom: Mouse wheel or +/- keys to zoom in/out (Current zoom: %.2f)\n", zoomScale);
            break;
            
        // Object selection
        case 'l':
            obj = (obj+1)%10;
            break;
        case 'L':
            obj = (obj+9)%10;
            break;

        // Orbital light controls
        case 'o':
        case 'O':
            orbitalLight = 1 - orbitalLight;
            break;
        case 'p':
        case 'P':
            orbitalMove = 1 - orbitalMove;
            break;
        case 'y':
            orbitalYlight -= 2.0f;
            if (orbitalYlight < -10.0f) orbitalYlight = -10.0f;

            break;
        case 'Y':
            orbitalYlight += 2.0f;
            if (orbitalYlight > 25.0f) orbitalYlight = 25.0f;
            break;
        case 't':
            orbitalZh += 10;
            break;
        case 'T':
            orbitalZh -= 10;
            break;
        case 'w':
        case 'W':
            resetOrbitalLight();
            break;
        case 'i':
            if (orbitalAmbient > 0) orbitalAmbient -= 5;
            break;
        case 'I':
            if (orbitalAmbient < 100) orbitalAmbient += 5;
            break;
        case 'j':
            if (orbitalDiffuse > 0) orbitalDiffuse -= 5;
            break;
        case 'J':
            if (orbitalDiffuse < 100) orbitalDiffuse += 5;
            break;
        case 'k':
            if (orbitalSpecular > 0) orbitalSpecular -= 5;
        
            break;
        case 'K':
            if (orbitalSpecular < 100) orbitalSpecular += 5;
            break;
        case 'b':
            if (orbitalShininess > -1) orbitalShininess -= 1;
            shiny = orbitalShininess < 0 ? 0 : pow(2.0f, orbitalShininess);
            break;
        case 'B':
            if (orbitalShininess < 7) orbitalShininess += 1;
            shiny = orbitalShininess < 0 ? 0 : pow(2.0f, orbitalShininess);
            break;
        case 'g':
            orbitalDistance = (orbitalDistance == 10) ? 50 : 10;
            break;
        case 'G':
            orbitalSmooth = 1 - orbitalSmooth;
            break;
        case 's':
        case 'S':
            stadiumAnimationPaused = 1 - stadiumAnimationPaused;
        
            break;
        case 'c':
        case 'C':
            ticketBoothRoofColor = (ticketBoothRoofColor + 1) % 4;
            break;
        case 'n':
            skyscraperTextureRepeat += 0.5f;
            if (skyscraperTextureRepeat > 10.0f) skyscraperTextureRepeat = 10.0f;
            break;
        case 'N':
            skyscraperTextureRepeat -= 0.5f;
            if (skyscraperTextureRepeat < 0.5f) skyscraperTextureRepeat = 0.5f;
            break;
        case '6':
            treeTrunkHeight += 0.5f;
            treeLeafScale += 0.1f;
            if (treeTrunkHeight > 10.0f) treeTrunkHeight = 10.0f;
            if (treeLeafScale > 3.0f) treeLeafScale = 3.0f;
            break;
        case '7':
            treeTrunkHeight -= 0.5f;
            treeLeafScale -= 0.1f;
            if (treeTrunkHeight < 1.0f) treeTrunkHeight = 1.0f;
            if (treeLeafScale < 0.3f) treeLeafScale = 0.3f;
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) 
{
    (void)x; (void)y; 
    
    if (currentViewMode==VIEW_FIRST_PERSON)
     {
        float forwardX = fpLookAtX- fpCameraX;
        float forwardY = fpLookAtY - fpCameraY;
        float forwardZ=fpLookAtZ -fpCameraZ;
        float length = sqrt(forwardX*forwardX +forwardY *forwardY + forwardZ * forwardZ);
        forwardX/= length;
        forwardY /=length;
        forwardZ/=length;
        float rightX = forwardY*fpUpZ -forwardZ * fpUpY;
        float rightY = forwardZ* fpUpX -forwardX * fpUpZ;
        float rightZ = forwardX *fpUpY - forwardY* fpUpX;
        length = sqrt(rightX* rightX+rightY *rightY + rightZ * rightZ);
        rightX /=length;
        rightY/= length;
        rightZ /= length;
        
        switch (key) {
            case GLUT_KEY_UP:
                fpCameraX += forwardX * fpMoveSpeed;
                fpCameraY+= forwardY *fpMoveSpeed;
                fpCameraZ += forwardZ * fpMoveSpeed;
                fpLookAtX+= forwardX *fpMoveSpeed;
                fpLookAtY+= forwardY * fpMoveSpeed;
                fpLookAtZ +=forwardZ *fpMoveSpeed;
                break;
            case GLUT_KEY_DOWN:
                fpCameraX -= forwardX * fpMoveSpeed;
                fpCameraY-= forwardY * fpMoveSpeed;
                fpCameraZ -= forwardZ* fpMoveSpeed;
                fpLookAtX -=forwardX * fpMoveSpeed;
                fpLookAtY-= forwardY * fpMoveSpeed;
                fpLookAtZ -= forwardZ * fpMoveSpeed;
                break;
            case GLUT_KEY_LEFT:
                fpCameraX-=rightX * fpMoveSpeed;
                fpCameraY -= rightY *fpMoveSpeed;
                fpCameraZ -= rightZ* fpMoveSpeed;
                fpLookAtX-= rightX * fpMoveSpeed;
                fpLookAtY -= rightY *fpMoveSpeed;
                fpLookAtZ-= rightZ* fpMoveSpeed;
                break;
            case GLUT_KEY_RIGHT:
                fpCameraX+= rightX * fpMoveSpeed;
                fpCameraY += rightY * fpMoveSpeed;
                fpCameraZ+= rightZ *fpMoveSpeed;
                fpLookAtX += rightX * fpMoveSpeed;
                fpLookAtY+=rightY * fpMoveSpeed;
                fpLookAtZ += rightZ* fpMoveSpeed;
                break;
        }
    } 
    else
     {
        switch (key) {
            case GLUT_KEY_LEFT:
                rotationY -= 5.0f;
                break;
            case GLUT_KEY_RIGHT:
                rotationY += 5.0f;
                break;
            case GLUT_KEY_UP:
                rotationX -= 5.0f;
                break;
            case GLUT_KEY_DOWN:
                rotationX += 5.0f;
                break;
        }
    }
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    if (currentViewMode == VIEW_FIRST_PERSON && mouseLookActive) 
    {
    
        int deltaX=x - lastMouseX;
        int deltaY = y - lastMouseY; 
        fpYaw += deltaX* fpMouseSensitivity;
        fpPitch -= deltaY *fpMouseSensitivity;
        if (fpPitch > 89.0f) fpPitch = 89.0f;
        if (fpPitch < -89.0f) fpPitch =-89.0f;
        lastMouseX =x;
        lastMouseY= y;
    } 
    else 
    {
        rotationY = (x- 400)* 0.5f;
        rotationX= (y-300) *0.5f;
    }
    
        glutPostRedisplay();
    }
void mouseButton(int button, int state, int x, int y)
 {
    if(currentViewMode == VIEW_FIRST_PERSON) 
    {
        if (button == GLUT_LEFT_BUTTON &&state == GLUT_DOWN)
         {
            mouseLookActive= 1;
            lastMouseX= x;
            lastMouseY =y;
            glutMotionFunc(mouseMotion);
        }
         else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP) 
        {
            mouseLookActive = 0;
        }
    }
     else 
     {
        (void)x; 
        (void)y;
        if (button ==GLUT_LEFT_BUTTON && state==GLUT_DOWN) 
        {
            glutMotionFunc(mouseMotion);
        }
    }
    
    if (button == 3 && state==GLUT_DOWN) 
    { 
        zoomScale*=1.1f;
        if (zoomScale > 5.0f) zoomScale=5.0f; 
    }
    else if (button==4 && state==GLUT_DOWN) 
    { 
        zoomScale/=1.1f;
        if (zoomScale < 0.1f) zoomScale=0.1f; 
    }
}
void reshape(int width,int height) 
{
    if (height== 0) height= 1;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect= (float)width/ (float)height;
    
    if (currentViewMode == VIEW_ORTHOGONAL) 
    {
        float stadiumAspect =1.0f;
        if (aspect > stadiumAspect) 
        {
            float left = -20.0f* aspect;
            float right= 20.0f * aspect;
            glOrtho(left,right, -20.0f,20.0f, 0.1f,100.0f);
        } 
        else{
            float bottom = -20.0f / aspect;
            float top = 20.0f /aspect;
            glOrtho(-20.0f,20.0f,bottom, top,0.1f, 100.0f);
        }
    } 
    else if (currentViewMode == VIEW_PERSPECTIVE) 
    {
        perspective(45.0f, aspect,0.1f, 100.0f);
    } 
    else if (currentViewMode==VIEW_FIRST_PERSON) 
    {
        perspective(60.0f, aspect, 0.1f, 100.0f);
    }
    
    glMatrixMode(GL_MODELVIEW);
}
void init(void) {
    glClearColor(0.1f,0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Disable all OpenGL lights 
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);

    float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ruban Chakaravarthi Vairava Sundaram"); 
    texture[0] = LoadTexBMP("Stadium_wall.bmp");
    texture[1] = LoadTexBMP("Building_wall.bmp");
    texture[2] = LoadTexBMP("Stone_brick.bmp");
    texture[3] = LoadTexBMP("Water.bmp");
    texture[4] = LoadTexBMP("Roof.bmp");
    texture[5] = LoadTexBMP("Cauldron_Texture.bmp");
    texture[6] = LoadTexBMP("Tree_bark.bmp");
    texture[7] = LoadTexBMP("leaf.bmp");
    texture[8] = LoadTexBMP("light_pole.bmp");
    texture[9] = LoadTexBMP("Swimming_Brick.bmp");
    texture[10] = LoadTexBMP("Swimming_Top.bmp");
    texture[11] = LoadTexBMP("HandRail.bmp");
    texture[12] = LoadTexBMP("Cauldron_Texture_1.bmp");
    texture[13] = LoadTexBMP("football_field.bmp");
    texture[14] = LoadTexBMP("Stadium.bmp");
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    
    glutIdleFunc(idle); 
    glutMainLoop();
    return 0;
}