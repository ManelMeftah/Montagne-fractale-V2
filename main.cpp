#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#include <sstream>
#include <armadillo>


#include<string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

using namespace std ;
using namespace arma ;

bool isSeaEnabled = true;

void affichage(void);

void clavier(unsigned char touche,int x,int y);
void affiche_repere(void);

void mouse(int, int, int, int);
void mouseMotion(int, int);


//void reshape(int,int);

// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
bool ctrlKeyPressed = false;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance=0.;

// constantes pour les materieux
  float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float mat_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f};
    float mat_ambient_color[] = {0.8f, 0.8f, 0.2f, 1.0f};
    float mat_diffuse[] = {0.1f, 0.5f, 0.8f, 1.0f};
    float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float no_shininess = 0.0f;
    float low_shininess = 5.0f;
    float high_shininess = 100.0f;
    float mat_emission[] = {0.3f, 0.2f, 0.2f, 0.0f};

float t = 0.5;
float sea_level = 5.0;
bool isFilaire;

arma::mat montagne;

void initOpenGl() 
{ 

//lumiere 

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
 
	glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat l_pos[] = { 3.,3.5,3.0,1.0 };
  glLightfv(GL_LIGHT0,GL_POSITION,l_pos);

  glLightfv(GL_LIGHT0,GL_DIFFUSE,l_pos);
 glLightfv(GL_LIGHT0,GL_SPECULAR,l_pos);
 glEnable(GL_COLOR_MATERIAL);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);


//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
// glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE|GLUT_RGB);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
  // gluPerspective(45.0f,(GLfloat)200/(GLfloat)200,0.1f,10.0f);
gluPerspective(60.0, 1.0, 1.0, 10000.0);


	glMatrixMode(GL_MODELVIEW);
      // gluLookAt(0.,0.,4., 
      //           0.,0.,0., 
      //           0.,1.,0.);

      gluLookAt(0.0, 0.0, 100.0,   // caméra 
          0.0, 0.0, 0.0,    // points to
          0.0, 1.0, 0.0);   // orientation


}

//------------------------------------------------------

//affichage console
void printMatrix(const arma::mat& m) {
    for (unsigned int i = 0; i < m.n_rows; ++i) {
        for (unsigned int j = 0; j < m.n_cols; ++j) {
            cout << m(i, j) << " ";
        }
        cout << endl;
    }
}

//retourne un montagne de taille n x n 
arma::mat diamant_carre(int n, double scale, double h)
{
    arma::mat montagne_tmp = zeros<arma::mat>(n, n);

    // init les coins
    montagne_tmp(0, 0) = montagne_tmp(0, n-1) 
        = montagne_tmp(n-1, 0) 
        = montagne_tmp(n-1, n-1) = ((double) rand() / (RAND_MAX)) * scale;

    int pas = n-1;

    while (pas > 1) {
        int demi_pas = pas / 2;

        for (int x = 0; x < n-1; x += pas) {
            for (int y = 0; y < n-1; y += pas) {
                // Étape de diamant
                double avg = (montagne_tmp(x, y) + montagne_tmp(x+pas, y)
                 + montagne_tmp(x, y+pas) + montagne_tmp(x+pas, y+pas)) / 4.0;
                
                montagne_tmp(x+demi_pas, y+demi_pas) = 
                        avg + ((double) rand() / (RAND_MAX) - 0.5) * scale;
            }
        }
        scale = ((double) rand() / (RAND_MAX) * 2 - 1) * pas * pow(2, -h); //Δ (d)=rand (−1,1)×d×2−h
        for (int x = 0; x < n-1; x += demi_pas) {
            for (int y = (x+demi_pas)%pas; y < n-1; y += pas) {
                // Étape de carré
                double avg = (montagne_tmp((x-demi_pas+n)%n, y) 
                              + montagne_tmp((x+demi_pas)%n, y) 
                              + montagne_tmp(x, (y-demi_pas+n)%n) 
                              + montagne_tmp(x, (y+demi_pas)%n) 
                              + montagne_tmp(x, y)) / 5.0;
                
                montagne_tmp(x, y) = 
                      avg + ((double) rand() / (RAND_MAX) - 0.5) * scale;
            }
        }
        pas /= 2;
    }

    // for (int i = 0; i < n; ++i) {
    //     for (int j = 0; j < n; ++j) {
    //         montagne_tmp(i, j) *= -1; 
    //     }
    // }

    return montagne_tmp;
}

void run()
{
   montagne = diamant_carre(65, 50.0, 0.5);
// printMatrix(montagne);

}

void drawmontagne() {
    int n = montagne.n_rows;

    for (int i = 0; i < n-1; ++i) {
        for (int j = 0; j < n-1; ++j) {
            // Centrer la montagne
             float x = i - n/2; 
             float z = j - n/2; 

            // Coordonnées des sommets de la facette
            glm::vec3 v1(x, montagne(i, j), z);
            glm::vec3 v2(x, montagne(i, j+1), z+1);
            glm::vec3 v3(x+1, montagne(i+1, j), z);
            glm::vec3 v4(x+1, montagne(i+1, j), z);
            glm::vec3 v5(x, montagne(i, j+1), z+1);
            glm::vec3 v6(x+1, montagne(i+1, j+1), z+1);

            // Vecteurs de la facette
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            // Calcul des composantes de la lumière
            glm::vec3 Ia = glm::vec3(0.7, 0.7, 0.7); 
            glm::vec3 Id = glm::vec3(0.1, 0.5, 0.8); 
            glm::vec3 Is = glm::vec3(1.0, 1.0, 1.0); 

            glm::vec3 Ka = glm::vec3(0.8, 0.8, 0.2); 
            glm::vec3 Kd = glm::vec3(0.1, 0.5, 0.8); 
            glm::vec3 Ks = glm::vec3(1.0, 1.0, 1.0); 
            float shininess = 100.0; 

            glm::vec3 N = normal;
            glm::vec3 L = glm::normalize(glm::vec3(3.0, 3.5, 3.0) - v1); // direction de la lumière
            glm::vec3 V = glm::normalize(glm::vec3(0.0, 0.0, 4.0) - v1); // direction de la vue

            glm::vec3 ambient = Ia * Ka;
            glm::vec3 diffuse = glm::max(glm::dot(N, L), 0.0f) * Id * Kd;
            glm::vec3 R = glm::reflect(-L, N);
            glm::vec3 specular = pow(glm::max(glm::dot(R, V), 0.0f), shininess) * Is * Ks;

            glm::vec3 intensity = ambient + diffuse + specular;

            glColor3f(intensity.x, intensity.y, intensity.z);

            // Rendu des triangles de la facette
            glBegin(GL_TRIANGLES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glVertex3f(v3.x, v3.y, v3.z);

            glVertex3f(v4.x, v4.y, v4.z);
            glVertex3f(v5.x, v5.y, v5.z);
            glVertex3f(v6.x, v6.y, v6.z);
            glEnd();
        }
    }


    if(isFilaire) {
      glColor3f(0.0, 0.0, 0.0);
            //rendu filaire
        glBegin(GL_LINES);
        for (int i = 0; i < n-1; ++i) {
            for (int j = 0; j < n-1; ++j) {
                // Centrer la montagne
                float x = i - n/2; 
                float z = j - n/2; 

                glVertex3f(x, montagne(i, j), z);
                glVertex3f(x, montagne(i, j+1), z+1);
                glVertex3f(x+1, montagne(i+1, j), z);

                glVertex3f(x+1, montagne(i+1, j), z);
                glVertex3f(x, montagne(i, j+1), z+1);
                glVertex3f(x+1, montagne(i+1, j+1), z+1);

            }
        }
        glEnd();
    }
    
}


// void drawmontagne() {
//     cout << "drawing.." << endl;
//     int n = montagne.n_rows;

  
//     glColor3f(0.5, 1.0, 0.5);

// //rendu facettes
//     glBegin(GL_TRIANGLES);
//     for (int i = 0; i < n-1; ++i) {
//         for (int j = 0; j < n-1; ++j) {
//             // Centrer la montagne
//              float x = i - n/2; 
//              float z = j - n/2; 

//             glVertex3f(x, montagne(i, j), z);
//             glVertex3f(x, montagne(i, j+1), z+1);
//             glVertex3f(x+1, montagne(i+1, j), z);

//             glVertex3f(x+1, montagne(i+1, j), z);
//             glVertex3f(x, montagne(i, j+1), z+1);
//             glVertex3f(x+1, montagne(i+1, j+1), z+1);

//         }
//     }
//     glEnd();

//     glColor3f(0.0, 0.0, 0.0);
//       //rendu filaire
//     glBegin(GL_LINES);
//     for (int i = 0; i < n-1; ++i) {
//         for (int j = 0; j < n-1; ++j) {
//             // Centrer la montagne
//              float x = i - n/2; 
//              float z = j - n/2; 

//             glVertex3f(x, montagne(i, j), z);
//             glVertex3f(x, montagne(i, j+1), z+1);
//             glVertex3f(x+1, montagne(i+1, j), z);

//             glVertex3f(x+1, montagne(i+1, j), z);
//             glVertex3f(x, montagne(i, j+1), z+1);
//             glVertex3f(x+1, montagne(i+1, j+1), z+1);

//         }
//     }
//     glEnd();

// }

void drawmeer()
{
  int n = montagne.n_rows;

  glColor3f(0.0, 1.0, 1.0);
  glBegin(GL_QUADS);
    glVertex3f(-n/2, sea_level, -n/2);
    glVertex3f(-n/2, sea_level, n/2);
    glVertex3f(n/2, sea_level, n/2);
    glVertex3f(n/2, sea_level, -n/2);
  glEnd();

  // glDisable(GL_LIGHT1);
}

void afficherMenuConsole() {
    std::cout << "Menu :\n";
    std::cout << "+ : Augmente le niveau de la mer\n";
    std::cout << "- : Diminue le niveau de la mer\n";
    std::cout << "f : Affiche en mode fil de fer seuls\n";
    std::cout << "p : Affiche le carré plein seul\n";
    std::cout << "s : Affiche en mode sommets seuls\n";
    std::cout << "m : Active/Désactive l'affichage de la mer\n";
    std::cout << "F : Active/Désactive l'affichage du mode fil de fer par dessus le mode remplis \n";
    std::cout << "q : Quitte le programme\n";

    std::cout << "  - Maintenir le bouton gauche de la souris : Déplacer la caméra" << std::endl;
    std::cout << "  - Maintenir le bouton droit de la souris : Zoom avant/arrière" << std::endl;

}


int main(int argc,char **argv)
{
  srand (time(NULL));
afficherMenuConsole();
  /* initialisation de glut et creation
     de la fenetre */
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB);
  glutInitWindowPosition(200,200);
  glutInitWindowSize(600,600);
  glutCreateWindow("fractale");

  /* Initialisation d'OpenGL */
  glClearColor(0.0,0.0,0.0,0.0);
  glColor3f(1.0,1.0,1.0);
  glPointSize(1.0);
	
	//ifs = new Ifs();
  run();
  /* enregistrement des fonctions de rappel */
  glutDisplayFunc(affichage);

  glutKeyboardFunc(clavier);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  //-------------------------------


  //-------------------------------
    initOpenGl() ;
//-------------------------------

/* Entree dans la boucle principale glut */
  glutMainLoop();
  return 0;
}
//------------------------------------------------------
void affiche_repere(void)
{
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex2f(0.,0.);
  glVertex2f(1.,0.);
  glEnd(); 

	 glBegin(GL_LINES);
  glColor3f(0.0,1.0,0.0);
  glVertex2f(0.,0.);
  glVertex2f(0.,1.);
  glEnd(); 
   glBegin(GL_LINES);
  glColor3f(0.0,0.0,1.0);
  glVertex3f(0.,0.,0.);
  glVertex3f(0.,0.,1.);
  glEnd(); 
}

//-----------------------------------------------------



//------------------------------------------------------

void affichage(void)
{
  

    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(0,0,cameraDistance);
    glRotatef(cameraAngleX,1.,0.,0.);
    glRotatef(cameraAngleY,0.,1.,0.);
    affiche_repere();

    glEnable(GL_CLIP_PLANE0);
    double eqn[4] = {0.0, 1.0, 0.0, -sea_level+0.2};
    glClipPlane(GL_CLIP_PLANE0, eqn);
    if (isSeaEnabled) {
        drawmeer();
    }
    drawmontagne();


    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}

//------------------------------------------------------


//------------------------------------------------------
void clavier(unsigned char touche,int x,int y)
{

  switch (touche)
    {
    case '+': //
      sea_level+=1.0;
       
      glutPostRedisplay();
      break;
    case '-': //* ajustement du t
       sea_level -= 1.0;
      glutPostRedisplay();
      break;
    case 'f': //* affichage en mode fil de fer 
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glutPostRedisplay();
      break;
      case 'p': //* affichage du carre plein 
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glutPostRedisplay();
      break;
  case 's' : //* Affichage en mode sommets seuls 
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glutPostRedisplay();
      break;

  case 'm':
        isSeaEnabled = !isSeaEnabled;
        glutPostRedisplay();
        break;

    case 'F':
    isFilaire = !isFilaire;
    glutPostRedisplay();
    break;

    case 'q' : //*la touche 'q' permet de quitter le programme 
      exit(0);
    }
    
}
void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}

void mouseMotion(int x, int y)
{

    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}

    
    
