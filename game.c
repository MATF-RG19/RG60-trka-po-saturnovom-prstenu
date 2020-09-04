#include <GL/glut.h>

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "image.h"

#define PI (3.14159265359)
#define MAX_NUM_OF_STARS (100)

#define TIMER_INTERVAL_ON_START (35)
#define TIMER_MINIMAL_INTERVAL (15)

#define TIMER_INTERVAL_ROTATION_SPHERE (50)
#define TIMER_INTERVAL_SPACEMAN_JUMP (10)

#define TIMER_VALUE (0)
#define TIMER_VALUE_JUMP (0)
#define TIMER_VALUE_ROTATION_SPHERE_MENI (0)
#define TIMER_VALUE_ROTATION_SPHERE_GAME (1)

#define STATE_UP (1)
#define STATE_DOWN (0)

#define FILENAME0 "texture.bmp"
#define FILENAME1 "1.bmp"
#define FILENAME2 "2.bmp"
#define FILENAME3 "3.bmp"
#define FILENAME4 "4.bmp"
#define FILENAME5 "1.bmp"
#define FILENAME6 "uran.bmp"
#define FILENAME7 "barrier1.bmp"

/* Deklaracija funkcija za obradu događaja. */
static void on_reshape(int width, int height);
static void on_display(void);
static void on_keyboard(unsigned char key, int x, int y);

static void on_timer_rotation_sphere(int value);
static void on_timer_barriers(int value);
static void on_timer_jump(int value);

/* Funkcija za iscrtavanje pomoćnih osa. 
static void draw_axes(void); */

/* Funkcije za iscrtavanje objekata na sceni (Zvezda, planeta, karaktera i prepreka). */
static void draw_ring(float in_radius, float out_radius, int precision);

static void draw_Saturn_ring(void);
static void draw_Saturn_sphere(void);
static void draw_Spaceman(void);

static void draw_barrier(float speedway_translation, int type, int hasDiamond);
static void draw_barriers(void);

static void draw_stars(void);
static void draw_star_vortex(void);
static void draw_Uran(void);

/* Iscrtavanje glavnog menija. */
static void draw_meni(void);

/* Iscrtavanje pozadine i glavnih elemenata igrice. */
static void draw_background(void);
static void draw_main_objects(void);

/* Iscrtavanje torusa. */
static void draw_torus(double r, double c, int rSeg, int cSeg, int texture);


/* Funkcije za generisanje inicijalnih vrednosti.*/
static void generate_barriers(void);
static void generate_random_stars(void);

/* Inicijalizacija parametara za postavljanje tekstura. */
static void initialize(void);
static GLuint names[10];

/* Ugao rotacije prepreka. */
static int rotation_angle = 0;

/* Ugao rotacije sfere i prstena u meniju. */
static int rotation_angle_sphere_meni = 0;

/* Ugao rotacije sfere u glavnom delu animacije. */
static int rotation_angle_sphere = 0;

/* Parametar translacije glavnog karaktera. */
static float translation_left_right = 0;
static float translation_up = 0;

/* Parametar koji označava stanja animacije. 
 * Početno stanje (0) stanje animcije u pokretu (1), stanje pauze (2), završno stanje (3). */
static int animation_ongoing = 0;

/* Broj milisekundi koji određuje periode ponovnog pozivanja funkcije on_timer_barriers. */
static unsigned int timer_interval = TIMER_INTERVAL_ON_START;

/* Broj staze na kojoj se nalazi glavni karakter. */
static int Spaceman_position=3;

/* Funkcija detekcije sudara. */
static void collision_detection(void);

/* Funkcija iscrtavanja niza karaktera. */
static void output(GLfloat x, GLfloat y, char *text);

/* Pomocna funkcija za sortiranje niza. */
static int compare(const void *p1, const void *p2) {
    return (*(int*)p1  - *(int*)p2)>=0;
}

/* Funkcija koja obrađuje živote.*/
static void end();

/* Strukture koje opisuju položaj barijere. */
typedef struct {
    int type;
    int speedway_position;
    float speedway_translation;
    float diamond;
} Barrier;

/* Strukture koje opisuju položaj zvezda. */
typedef struct {
    float x, y;
} Star;

/* Niz prepreka koje će se iscrtavati. */
static Barrier barriers[5];

/* Niz zvezda koje će se iscrtavati. */
static Star stars_positions[MAX_NUM_OF_STARS];

/* Pomoćni parametar koji označava da li je glavni karakter u skoku. */
static bool Spaceman_jump = false;

/* Pomoćni parametar koji označava stanje skoka glavnog karaktera. */
static int jump_state = 0;

/* Rezultat. */
static int score = 0;

/* Inicijana vrednost života. */
static int life = 3;




int main(int argc, char** argv) {
    
    /* Inicijalizacija GLUT-a. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    
    /* Kreiranje prozora. */
    glutCreateWindow(argv[0]);
    glutFullScreen();

    /* Registracija funkcija za obradu događaja. */
    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutKeyboardFunc(on_keyboard);
    
    /* OpenGL Inicijalizacija. */
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    
    /* Generisanje inicijanih vrednosti. */
    generate_barriers();
    generate_random_stars();
    
    /* Inicijalizacija parametara za postavljanje teksture i učitavanje tekstura. */
    initialize();

    /* Ulazi se u glavnu petlju. */
    glutMainLoop();
    
    return 0;
}

/* Pomoćna funkcija za iscrtavanje koordinatnih osa. 
void draw_axes()
{
glBegin( GL_LINES );
glColor3f( 1.0, 0.0, 0.0);
glVertex3f( -50.0, 0.0, 0.0 );
glVertex3f( 50.0, 0.0, 0.0 );
glEnd();

glBegin( GL_LINES );
glColor3f( 0.0, 0.0, 1.0 );
glVertex3f( 0.0, -50.0, 0.0);
glVertex3f( 0.0, 50.0, 0.0);
glEnd();

glBegin( GL_LINES );
glColor3f( 0.0, 1.0, 0.0);
glVertex3f( 0.0, 0.0, -50.0);
glVertex3f( 0.0, 0.0, 50.0);
glEnd();
}*/

/* Obrada događaja on on_display. */
static void on_display(void) {
    
    /* Obavlja se čišćenje bafera. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Podesavanje vidne tacke. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Podešavanje parametara scene u koju se gleda i "oka" posmatrača. */
    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);
        
    switch(animation_ongoing) {
        case 0:
        /* Animacija u početnom stanju:
         * Vrši se iscrtavanje menija i odgovarajuće poruke. */
            
            glPushMatrix();
                draw_meni();
            glPopMatrix();
            
            glPushMatrix();
                
                /* Podesavanje parametara ispisivanja poruke, 
                 * veličine linija kojom se iscrtavaju slova,
                 * boje slova, omogućavanje mešanje definisanih boja
                 * i boja u baferu. */
                
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                glEnable(GL_LINE_SMOOTH);
            
                glScalef(0.0005,0.0005,0.0005);
                glLineWidth(5.0);
                glColor3f(0.9,0.9,0.83);
                
                output(100,1200, "Welcome to Saturn!      ");
                glLineWidth(2.5);
                
                glScalef(0.7,0.7,0.7);
                
                
                
                output(200, 1000, "Your challenge is to avoid cosmic obstacles and collect");
                output(200, 800, "as many diamonds as possible. You own 3 lives.");
                output(200, 400, "Press 'G' or 'g' to start.");
                output(200, 200, "Press 'P' or 'p' to pause.");
                output(200, 0, "To move left press 'A' or 'a'.");
                output(200, -200, "To move right press 'D' or 'd'.");
                output(200, -400, "To make jump press space.");
                output(200, -800,   "To exit game press escape.");
                output(200, -1000,   "The value of the diamond is 100 points.");
            
                glScalef(1.2,1.2,1.2);
                glLineWidth(4);
                output(160, -1200,"Have fun :)");
            
            glPopMatrix();
            break;
        
        case 1:
        /* Animacija u pokretu:
         * Vrši se iscrtavanje pozadine, glavnih objekata, 
         * poziva se funkcija detekcije sudara i ispisuje se vrednost rezultata.  */
           
            
                glPushMatrix();
             
                draw_background();
                draw_main_objects();
                collision_detection();
              
                glColor3f(1,1,1);
            
                glScalef(0.0005,0.0005,0.0005);
                glColor3f(0.9,0.9,0.83);
            
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                glEnable(GL_LINE_SMOOTH);
            
                glLineWidth(5.0);
                
                char result[100];
                sprintf(result, "Score: %d", score/10);
                output(2000,1200, result);

                char lifes[15];
                  switch (life) {
                        case 3:     sprintf(lifes, "LIFE: o o o "); break;
                        case 2:     sprintf(lifes, "LIFE: o o "); break;
                        case 1:     sprintf(lifes, "LIFE: o "); break;
                } 
                output(2000,1400, lifes);

                glPopMatrix();
            
        break;
        
        case 2:
        /* Animacija u pauzi:
         * Vrši se iscrtavanje pozadine, glavnih objekata, 
         * poziva se funkcija detekcije sudara i 
         * ispisivanja odgovarajuće poruke. */
             
            glPushMatrix();
            
            draw_background();
            draw_main_objects();
            collision_detection();
            
            
            glScalef(0.0005,0.0005,0.0005);
            glColor3f(1,1,1);
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(5.0);
            glColor3f(0.9,0.9,0.83);
            output(-100,1300, "GAME PAUSED");
            glLineWidth(2.5);
            output(-100, 1100,"Press 'G' or 'g' to resume.");
            
            glPopMatrix();
            break;
            
        case 3:
        /* Animacija na završetku: Iscrtavanje scene, main objekata i poziva se funkcija detekcije sudara. */
            glPushMatrix();
            draw_background();
            draw_main_objects();
            
            
            glPushMatrix();
                glScalef(0.0005,0.0005,0.0005);
                glLineWidth(5.0);
                
                glColor3f(1,1,1);
                output(100,1200, "GAME OVER      ");
                glLineWidth(2.5);
                sprintf(result, "Your score is: %d", score/10);
                output(100, 800, result);
                output(100, 600, "Press 'R' or 'r' to play again.");
            glPopMatrix();
            
            /* Iscrtava se pravougaonik koji je transparentan. */
            glPushMatrix();
                glColor4f(0.2,0.2,0.2,0.3);
                glTranslatef(0,0,1.7);
                glBegin(GL_POLYGON);
                glVertex3f(2, 2, 0);
                glVertex3f(2, -2, 0);
                glVertex3f(-2, -2, 0);
                glVertex3f(-2, 2, 0);
                glEnd();
            glPopMatrix();
            
            
            glPopMatrix();
            break;
        case 4: 
        /* Animacija u pauzi, izgubljen zivot. */    
            
        glPushMatrix();
            
            generate_barriers();
            /* Parametri se restartuju. */
            Spaceman_jump = false;
            translation_up = 0;
            rotation_angle=0;
            timer_interval=TIMER_INTERVAL_ON_START;
                           
            draw_background();
            draw_main_objects();
            collision_detection();
            
            glScalef(0.0005,0.0005,0.0005);
            glColor3f(1,1,1);
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(5.0);
            glColor3f(0.9,0.9,0.83);
            output(-100,1300, "You lost life!");
            glLineWidth(2.5);
            output(-100, 1100,"Press 'G' or 'g' to resume.");
            
            glPopMatrix();
                
                
            break;
    }
    
    glutSwapBuffers();
}

/* Iscrtavanje barijera i transliranje po stazama. */
static void draw_barrier(float speedway_translation, int type, int hasDiamond) {
    glPushMatrix();
    
    GLUquadric* barrier_type = gluNewQuadric();
    gluQuadricTexture(barrier_type, GL_TRUE);
        
        /* Rotacija prepreka oko y ose.*/
        glRotatef(-rotation_angle, 0, 1, 0);
        
        /* Postavljanje prepreka na adekvatne staze. */
        glTranslatef(0,0.1/2, -0.8-0.1/2-speedway_translation);
        
        
        glPushMatrix();
        
        if (hasDiamond) {
            glColor3f(1,1,0);
            glTranslatef(0,0.15,0);
            glRotatef(45,1,0,0);
            glScalef(0.2,1,1);
            glutSolidCube(0.04);
            /*gluSphere(sphere, (GLdouble) 0.03, (GLint) 100, (GLint) 100);*/
        }
        glPopMatrix();
        
         
        glBindTexture(GL_TEXTURE_2D, names[7]);
        glEnable(GL_TEXTURE_2D);
        
        switch (type) {
            case 1: gluSphere(barrier_type, (GLdouble) 0.05, (GLint) 100, (GLint) 100); break;
                        
            case 2: glRotatef(-rotation_angle*5, 1, 0, 1); draw_torus(0.009,0.02,16,8,0); break;
            case 3: glRotatef(-rotation_angle*2, 1, 1, 1); gluCylinder(barrier_type, (GLdouble) 0.06, (GLdouble) 0.02, (GLdouble) 0.07, (GLint) 100, (GLint) 100); break;
        }
        
        
   
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

    
    glPopMatrix();
}

/* Iscrtavanje prstena. */
static void draw_ring(float in_radius, float out_radius, int precision) {
    GLUquadric* disk = gluNewQuadric();
    
    /* Generišu se koordinate teksture. */
    gluQuadricTexture(disk, GL_TRUE);
    
    gluDisk(disk, (GLdouble) in_radius , (GLdouble) out_radius, (GLint) precision, (GLint) precision);        
}

/* Obrada događaja on on_reshape. */
static void on_reshape(int width, int height)
{
    /* Podešavanje pogleda. */
    glViewport(0,0, width, height);

    /* Primenjivanje narednih operacija na stek matrice projekcije. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    /* Podešavanje perspektive projekcije.*/
    gluPerspective(30, (float) width / height, 1, 1000);
}

/* Obrada događaja on on_keyboard. */
static void on_keyboard(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    
    /* Na taster 'g' ili 'G'  se pokreće anmacija, 
     * tasteri 'a', 'A', 'd', 'D' su rezervisani za kretanje karaktera po stazama,
     * pritiskom na taster 'p' ili 'P' animacija se pauzira,
     * pritiskom na taster 'r' ili 'R' animacija se restatruje nakon završetka,
     * pritiskom na taster "esc" animacija se prekida. */
    
    
    /* Parametar animation_ongoing može imati vrednosti 
     * 0 (Animacija nije pokrenuta), 1 (Animacija je pokrenuta),  2 (Animacija je pauzirana), 3 (Animacija je završena), 4 (Animacija pauzirana, izgubljen život). */
    switch(key) {
        case 27: 
            exit(0);
            break;
        case 'g':
        case 'G':            
            /* Animacija se pokreće ako je u početnom stanju ili u stanju pauze. */
            if (animation_ongoing == 0 || animation_ongoing == 2 || animation_ongoing == 4 ) {
                /* Pozivaju se funkcije za obradu tajmera koji pokreću rotaciju prepreka i rotaciju planete Saturn. */
                glutTimerFunc(timer_interval, on_timer_barriers, TIMER_VALUE);
                glutTimerFunc(TIMER_INTERVAL_ROTATION_SPHERE, on_timer_rotation_sphere, TIMER_VALUE_ROTATION_SPHERE_GAME);
                
                /* Animacija je pokrenuta. */
                animation_ongoing = 1;
            }
            break;
            
            
        /* Karakter se translira levo ili desno ako je animacija u pokretu. */
        case 'a':
        case 'A':
            /* Translacija u stazu levo. */
            
            if (translation_left_right>-0.2 &&  animation_ongoing == 1) {
                
                /* Karakter se pomera ako nije u stanju skoka. */
                if (Spaceman_jump == false) {
                    translation_left_right -= 0.1;
                    
                    /* Karakter se pomera u stazu levo.*/
                    Spaceman_position-=1;
                    
                    glutPostRedisplay();
                }
            }
            break;
            
        case 'd':
        case 'D':
            /* Translacija u stazu desno. */
            
            if (translation_left_right<0.2 && animation_ongoing == 1) {
                
                /* Karakter se pomera ako nije u stanju skoka. */
                if (Spaceman_jump == 0) {
                    translation_left_right += 0.1;
                    
                    /* Karakter se pomera u stazu desno.*/
                    Spaceman_position+=1;
                    
                    glutPostRedisplay();
                }
            }
            break;
            
        case 'p':
        case 'P':    
        /* Animacija u pokretu se prebacuje u stanje pauze. */
        
            if (animation_ongoing == 1) {
                /* Animacija je pauzirana. */
                animation_ongoing=2;
            }
            break;
            
        case 'r':
        case 'R':
        /* Animacija se restartuje. */
        
            if (animation_ongoing == 3) {
                    
                /* Parametri se restartuju. */
                score=0;
                Spaceman_jump = false;
                translation_up = 0;
                rotation_angle=0;
                timer_interval=TIMER_INTERVAL_ON_START;
                life = 3; 
                
                /* Animacija se ponovo pokreće */
                glutTimerFunc(timer_interval, on_timer_barriers, TIMER_VALUE);
                glutTimerFunc(TIMER_INTERVAL_ROTATION_SPHERE, on_timer_rotation_sphere, TIMER_VALUE_ROTATION_SPHERE_GAME);
                
                /* Animacija je pokrenuta. */
                animation_ongoing = 1;
            }
            break;
            
        case ' ':
        /* Obavlja se skok glavnog karaktera ako je animacija pokrenuta. */
            if (animation_ongoing == 1) {
                if (Spaceman_jump == false) {
                    Spaceman_jump = true;
                    /* Stanje skoka prelazi u stanje "u vis". */
                    jump_state = STATE_UP;
                    glutTimerFunc(TIMER_INTERVAL_SPACEMAN_JUMP, on_timer_jump, TIMER_VALUE_JUMP);
                }
                
            }
        }
}

/* Iscrtavanje Saturnovog prstena. */
static void draw_Saturn_ring(void) {
    glPushMatrix();
        glRotatef(90, 1,0,0);
        
        int i=0;
        int num_of_speedways = 5;
    
        float inner = 0.8, outer = 0.9;
    
        /* Iscrtavanje 5 spojenih kružnih prstenova. */
        for (i=0; i<num_of_speedways; i++) {
            
            /* Uključivanje adekvatne teksture prstena. */
            glBindTexture(GL_TEXTURE_2D, names[i+1]);
            glEnable(GL_TEXTURE_2D);
            
            /* Iscrtavanje prstena. */
            draw_ring(inner + 0.1*i, outer + 0.1*i, 200);
        
            /* Isključivanje teksture. */
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }
    glPopMatrix();
}

/* Iscrtavanje planete. */
static void draw_Saturn_sphere(void) {
    
    glPushMatrix();
    
        /* Uključivanje adekvatne teksture sfere. */
        glBindTexture(GL_TEXTURE_2D, names[0]);
        glEnable(GL_TEXTURE_2D);
        
        
        /* Iscrtavanje sfere. */
        GLUquadric* sphere = gluNewQuadric();
        
        /* Generišu se koordinate teksture. */
        gluQuadricTexture(sphere, GL_TRUE);
        
        glRotatef(90,1,0,0);
        
        gluSphere(sphere, (GLdouble) 0.7, (GLint) 100, (GLint) 100);        

        /* Isključivanje teksture. */
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    
}

/* Iscrtavanje glavnog karaktera. */
static void draw_Spaceman(void) {
    
    /* Iscrtava se telo glavnog karaktera. */
    glPushMatrix();
        
        glColor3f(0.1,0.5,0.4);
        glScalef(1,0.6,1);
        glTranslatef(0,0.4,0);
        glutSolidSphere(0.6, 50, 50);
    glPopMatrix();

    /* Iscrtavaju se uši. */
    glPushMatrix();
    /* Podešava se boja ušiju koja će konstantno da se menja, davajući iluziju svetlucanja. */
  
    glTranslatef(-0.25,0.4,0);

    glPushMatrix();
      glColor3f((float)rand()/RAND_MAX,(float)rand()/RAND_MAX,(float)rand()/RAND_MAX);
   
        glPushMatrix();
            glRotatef(30,0,0,1);
            glRotatef(-90,1,0,0);
            glutSolidCone(0.25, 0.4, 50, 50);
        glPopMatrix();
    
        glPushMatrix();
                glTranslatef(0.5,0,0);
                glRotatef(-30,0,0,1);
                glRotatef(-90,1,0,0);
                glutSolidCone(0.25, 0.4, 50, 50);
        glPopMatrix();
      glPopMatrix();  
        
      
        glColor3f(0.1,0.5,0.4);
        glTranslatef(0,-0.4,0);
        glPushMatrix();
            glRotatef(90,1,0,0);
            glutSolidCone(0.1, 0.4, 50, 50);
        glPopMatrix();
    
                glPushMatrix();
                glTranslatef(0.2,0,0);
                glRotatef(90,1,0,0);
                glutSolidCone(0.1, 0.4, 50, 50);
        glPopMatrix();
        
        
        glPushMatrix();
                glTranslatef(0.4,0,0);
                glRotatef(90,1,0,0);
                glutSolidCone(0.1, 0.4, 50, 50);
        glPopMatrix();
        
        
           glPushMatrix();
                glTranslatef(0.6,0,0);
                glRotatef(90,1,0,0);
                glutSolidCone(0.1, 0.4, 50, 50);
        glPopMatrix();
        
    glPopMatrix();
    
}

/* Inicijalizacija položaja prepreka. */
static void generate_barriers(void) {
    int i;
    
    /* Pomoćni niz za izbacivanje duplikata pri izboru prepreka. */
    int speedway_position[5];
    
    
    /* Srand će omogućiti da pri svakom generisanju položaja prepreka
     * one budu u drugom položaju.*/
    srand(time(NULL));
    
    int diamond = (rand() % 5)+1;
    
    /* Generisanje 5 random brojeva i smestanje u pomocni niz. */
    for (i=0; i<5; i++) {
        speedway_position[i] = (rand() % 5)+1;
        barriers[i].type = (rand() % 3)+1;
    }

    /* Sortiranje niza zbog lakseg izbacivanja duplikata i izbegavanja suvisnog iscrtavanja prepreka. */
    qsort(speedway_position, 5, sizeof(int), compare);
    
    /* Zapis jedinstvenih polozaja u stazama i parametra translacije po z osi. */
    for (i=0; i<4; i++) {
        if (speedway_position[i]<speedway_position[i+1]) {
            barriers[i].speedway_position=speedway_position[i];
            barriers[i].speedway_translation=(speedway_position[i]-1)/10.0;
        } else {
            barriers[i].speedway_position=0;
            barriers[i].speedway_translation=0;
        }
    }
    
    /* Poslednji član je najveći član u pomoćnom nizu, 
     * bez bilo kakve provere neće imati duplikata. */
    barriers[4].speedway_position=speedway_position[4];
    barriers[4].speedway_translation=(speedway_position[4]-1)/10.0;
    
    
    barriers[diamond].diamond = 1;
}

/* Iscrtavanje prepreka. */
static void draw_barriers(void) {
    int i;
    
    /* Ako je funkcija koja generiše slučajne položaje označila staze iscrtavanja
     * sledi iscrtavanje tih prepreka. (Staze su numerisane brojevima od 1 do 5). */
    
    for (i=0; i<5; i++) 
    
        if (barriers[i].speedway_position != 0) {
            draw_barrier(barriers[i].speedway_translation, barriers[i].type, barriers[i].diamond);
        }

}

/* Inicijalizacija položaja zvezda. */
static void generate_random_stars(void) {
    
    /* Srand će omogućiti da pri svakom generisanju položaja zvezda
     * zvezde budu u drugom položaju.*/
    srand(time(NULL));
    int i;
    for (i=0; i<MAX_NUM_OF_STARS; i++) {
        
        /* Koordinata x će imati vrednost u intervalu (-4, 4). */
        stars_positions[i].x=pow(-1, i) * 4.0 * rand()/RAND_MAX;
        
        /* Koordinata y će imati vrednost u intervalu (-2.5, 2). */
        stars_positions[i].y=pow(-1, i+1) * 2.5 * rand()/RAND_MAX;
    
    }
}

/* Iscrtavanje zvezda. */
static void draw_stars(void) {
    glColor3f(1,1,1);
    int i;
    glBegin(GL_POINTS);
    glPointSize(0.4);
    for (i=0; i<MAX_NUM_OF_STARS; i++) {
        /* Iscrtavanje zvezda u svim kvadrantima. */
        glVertex3f(stars_positions[i].x, stars_positions[i].y, -5);    
        glVertex3f(stars_positions[i].x*(-1), stars_positions[i].y, -5);  
    }
    glEnd();
}

/* Iscrtavanje zvedanog vrtloga. */
static void draw_star_vortex(void) {
    float i;
    glColor3f(1,1,1);
     glBegin(GL_POINTS);
    for(i=0; i<200; i+=1) {
        glVertex2f(cos(i)*1.0/i, sin(i)*1.0/i);
    }
    glEnd();
}

/* Iscrtavanje nebeskih tela na sceni, u pozadini. */
static void draw_background(void) {
    glPushMatrix();
        draw_Uran();
        draw_stars();
        glTranslatef(0.5,0.5,0);
        draw_star_vortex();
        glTranslatef(0.7,-0.4,0);
        draw_star_vortex();
    glPopMatrix();
}

/* Iscrtavanje glavnih objekata na sceni.*/
static void draw_main_objects(void) {
    
    
    glPushMatrix();
    
        glRotatef(10,1,0,0);
        glTranslatef(-0.70,-0.1,0.33);
        
        /* Iscrtavanje sfere planete Saturn, koja se rotira. */
        glPushMatrix();
        glRotatef(-rotation_angle_sphere,0,1,0);
        draw_Saturn_sphere();
        glPopMatrix();
        
        /* Iscrtavanje prstena planete Saturn.*/
        glPushMatrix();
        glScalef(1,1,2.5);
        draw_Saturn_ring();
        glPopMatrix();
        
        
        /* Iscrtavanje prepreka. */
        glPushMatrix();
        glScalef(1,1.2,1);
        glScalef(1,1,2.5);
        draw_barriers();
        glPopMatrix();
        
        /* Iscrtavanje glavnog karaktera. */
        glPushMatrix();
        glTranslatef(translation_left_right, translation_up, 0);
        glTranslatef(0.94,0.07,1.2);
        glScalef(0.07,0.1,0.08);
        glColor3f(0,0.7,0);
        draw_Spaceman();
        glPopMatrix(); 
    glPopMatrix();

}


/* Detekcija sudara. */
static void collision_detection(void) {
    int i;

    /* Zbog kružnog kretanja prepreka uglovi sudara će biti različiti u različitim stazama. */
    int danger_zone_parametar=0;
    
    for (i=0; i<5; i++)  {
            switch (barriers[i].speedway_position) {
                case 1:
                    danger_zone_parametar = 0;
                    break;
                case 2:
                    danger_zone_parametar = 2;
                    break;
                case 3:
                    danger_zone_parametar = 4;
                    break;
                case 4:
                    danger_zone_parametar = 6;
                    break;
                case 5:
                    danger_zone_parametar = 8;
                    break;
            }
                
            /* Od ugla rotacije 116 položaji glavnog karakter i prepreka se poklapaju u poslednjoj stazi. 
             * U prethodnim stazama je ugao sudara manji za danger_zone_parametar. */
            
            if (rotation_angle>=(116-danger_zone_parametar) && rotation_angle<(116-danger_zone_parametar+6)
                && barriers[i].speedway_position==Spaceman_position && translation_up<=0.12 )   {
                /* Ako je ugao rotacije prepreke jednak 116-danger_zone_parametar, 
                 * ako se glavni karakter nalazi u istoj stazi kao i ta prepreka 
                 * doći će do direktnog sudara. */
                
                /* Ako je ugao rotacije prepreke veći 116-danger_zone_parametar, 
                 * ako se glavni karakter nalazi u istoj stazi kao i ta prepreka 
                 * a parametar translacije manji od 0.12 skok se nije desio na vreme. */
                
                /* Prekidamo skok i animacija prelazi u završno stanje.*/
                Spaceman_jump = false;
                end();
                    
                }    
                
            /* Ukoliko na mestu na kom se nalazi Spaceman postoji dijamant i ukoliko ga pokupi score se poveca za 100. */    
            if (rotation_angle>=(116-danger_zone_parametar) && rotation_angle<(116-danger_zone_parametar+6)
                && barriers[i].speedway_position==Spaceman_position && translation_up>=0.12 && barriers[i].diamond == 1) {
                score=score+1000;
                barriers[i].diamond = 0;
                glutPostRedisplay();
            }
        }           

}

/* Funkcija koja prati stanja zivota igraca. */
static void end() {
        if (--life <= 0) {
            animation_ongoing = 3;
        } else {
            animation_ongoing = 4;
        }
        glutPostRedisplay();
}

/* Iscrtavanje početnog menija. */
static void draw_meni(void) {
    
    glutTimerFunc(TIMER_INTERVAL_ROTATION_SPHERE, on_timer_rotation_sphere, TIMER_VALUE_ROTATION_SPHERE_MENI); 
    
    glPushMatrix();
        /* Iscrtava se vrtlog zvezda. */
        glPushMatrix();
            draw_stars();
            glTranslatef(1,0,0);
            draw_star_vortex();
        glPopMatrix();
        
        /* Iscrtava se rotirajuća planeta Saturn.*/
        glPushMatrix();
            glTranslatef(-0.5,0.2,0);
            glScalef(0.4,0.4,0.4);
            glPushMatrix();
            glRotatef(-rotation_angle_sphere_meni, 0,1,0);
            draw_Saturn_sphere();
            glPushMatrix();
                glRotatef(10,1,0,0);
                draw_Saturn_ring();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

/* Obrada funkcije tajmera za obradu rotacije prepreka. */
static void on_timer_barriers(int value) {
    
    if (value != TIMER_VALUE) 
        return; 
    
    /* Ugao se povećava do 180 stepeni. */
    if (rotation_angle<180) {
        rotation_angle += 2;
        glutPostRedisplay();
    }
    /* Ugao se restatruje i generišu se nove prepreke. */
    else {
        rotation_angle=0;    
        generate_barriers();
        /* Interval pozivanja tajmera se smanjuje do minimalne definisane vrednosti,  
         * što dovodi do ubrzavanja rotacije prepreka dok se ne dostigne ta minimalna vrednost, 
         * nakon toga brzina rotacije ostaje ista. */
        if (timer_interval>TIMER_MINIMAL_INTERVAL) {
            timer_interval -= 5;
        }
    }
    
    /* Nastavljanje animacije. */
    if (animation_ongoing == 1)
            glutTimerFunc(timer_interval, on_timer_barriers, TIMER_VALUE);
    
}

/* Obrada funkcije main tajmera za rotaciju Saturna. */
static void on_timer_rotation_sphere(int value) {
    
    switch(value) {
    
    case TIMER_VALUE_ROTATION_SPHERE_MENI: 
        if (animation_ongoing == 0) {
            /* Uvećava se ugao rotacije planete u meniju. */
            rotation_angle_sphere_meni+=2;
            
        }
        break;
        
    case TIMER_VALUE_ROTATION_SPHERE_GAME:
        if (animation_ongoing == 1) {
            /* Uvećava se rezultat i ugao rotacije sfere u glavnom delu animacije. */
            score++;
            rotation_angle_sphere+=2;
            glutTimerFunc(TIMER_INTERVAL_ROTATION_SPHERE, on_timer_rotation_sphere, TIMER_VALUE_ROTATION_SPHERE_GAME);
        }
    }
    glutPostRedisplay();
}


/* Obrada funkcije jump tajmera za skok glavnog karaktera. */
static void on_timer_jump(int value) {
    
    /* Ako je animacija pokrenuta i stanje skoka postavljeno na STATE_UP 
     * glavni karakter se translira pozitivnom vrednošću po y osi. */
    if (animation_ongoing == 1) {
    
        /* Ako je animacija pokrenuta i stanje skoka postavljeno na STATE_UP 
         * glavni karakter se translira pozitivnom vrednošću po y osi. */
        if (translation_up < 0.24 && jump_state == STATE_UP) {
            translation_up += 0.02;
        } 
        /* Inače se stanje postavlja na STATE_DOWN koje je okidač za promenu smera translacije glavnog karaktera. */
        else if (translation_up >= 0.24) {
                jump_state = STATE_DOWN;
            }
        
        /* Ako je animacija pokrenuta i stanje skoka postavljeno na STATE_DOWN 
         * glavni karakter se translira negativnom vrednošću po y osi sve dok se ne vrati u početno stanje. */
        if (jump_state == STATE_DOWN && translation_up > 0)
            translation_up -= 0.015;
        
        /* Ako karakter vraćen u početno stanje, restartuju se parametri skoka. */
        if (translation_up <= 0) {
            Spaceman_jump = 0;
            translation_up = 0;
        }
    }
    
    /* Ako smo u stanju skoka, nastavljamo sa animacijom. */
    if (Spaceman_jump == 1) {
        glutTimerFunc(10,on_timer_jump, 3);
    }
    
    
        glutPostRedisplay();    
}


/* Iscrtavanje niza karaktera. */
static void output(GLfloat x, GLfloat y, char *text) {
           char *p;
           
           glPushMatrix();
           glTranslatef(x, y, 0);
           for (p = text; *p; p++)
             glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
           glPopMatrix();
}




/* Iscrtavanje Urana. */
static void draw_Uran() {
    
      glPushMatrix();
    
        glBindTexture(GL_TEXTURE_2D, names[6]);
        glEnable(GL_TEXTURE_2D);
            
        GLUquadric* sphere = gluNewQuadric();
        gluQuadricTexture(sphere, GL_TRUE);
        
        
        glTranslatef(2,1.1,-4);
        glRotatef(90,1,0,0);
        gluSphere(sphere, (GLdouble) 0.1, (GLint) 100, (GLint) 100);        

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

    glPopMatrix();
  
}

/* Inicijalizacija parametara za postavljanje tekstura. */
static void initialize(void)
{
    /* Deklaracija teskture učitane iz fajla. */
    Image * image;

    /* Uključuju se teksture. */
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV,
              GL_TEXTURE_ENV_MODE,
              GL_REPLACE);

    /* Inicijalizuje se promenljiva image koji ce sadrzati teksture ucitane iz fajlova. */
    image = image_init(0, 0);

    /* Kreira se prva tekstura. */
    /* Generisu se identifikatori tekstura. */
    glGenTextures(7, names);

    image_read(image, FILENAME0);

    glBindTexture(GL_TEXTURE_2D, names[0]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    image_read(image, FILENAME1);

    glBindTexture(GL_TEXTURE_2D, names[1]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    image_read(image, FILENAME2);

    glBindTexture(GL_TEXTURE_2D, names[2]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    image_read(image, FILENAME3);

    glBindTexture(GL_TEXTURE_2D, names[3]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    image_read(image, FILENAME4);

    glBindTexture(GL_TEXTURE_2D, names[4]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    image_read(image, FILENAME5);

    glBindTexture(GL_TEXTURE_2D, names[5]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    image_read(image, FILENAME6);

    glBindTexture(GL_TEXTURE_2D, names[6]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);


    image_read(image, FILENAME7);

    glBindTexture(GL_TEXTURE_2D, names[7]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    
    /* Iskljucujemo aktivnu teksturu */
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Unistava se objekat za citanje tekstura iz fajla. */
    image_done(image);
}

static void draw_torus(double r, double c, int rSeg, int cSeg, int texture){
    glFrontFace(GL_CW);
    int i,j,k;
    const double TAU = 2 * PI;

    for (i = 0; i < rSeg; i++) {
    glBegin(GL_QUAD_STRIP);
    for (j = 0; j <= cSeg; j++) {
      for (k = 0; k <= 1; k++) {
        double s = (i + k) % rSeg + 0.5;
        double t = j % (cSeg + 1);

        double x = (c + r * cos(s * TAU / rSeg)) * cos(t * TAU / cSeg);
        double y = (c + r * cos(s * TAU / rSeg)) * sin(t * TAU / cSeg);
        double z = r * sin(s * TAU / rSeg);

        double u = (i + k) / (float) rSeg;
        double v = t / (float) cSeg;

        glTexCoord2d(u, v);
        glNormal3f(2 * x, 2 * y, 2 * z);
        glVertex3d(2 * x, 2 * y, 2 * z);
      }
    }
    glEnd();
  }

  glFrontFace(GL_CCW);
}


