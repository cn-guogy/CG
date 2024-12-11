#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// 光源位置
GLfloat light_position[] = {0.0f, 0.0f, 1.0f, 1.0f};

// 行星结构体
typedef struct
{
    float distance; // 距离中心天体的距离
    float size;     // 行星的大小
    float color[3]; // 行星的颜色
    float angle;    // 行星的当前角度
    float speed;    // 行星的旋转速度
} Planet;

// 定义太阳和八大行星
Planet sun = {0.0f, 0.5f, {1.0f, 0.0f, 0.0f}, 0.0f, 0.0f}; // 太阳
Planet planets[8] = {
    {2.0f, 0.1f, {0.5f, 0.5f, 0.5f}, 0.0f, 0.05f},   // 水星
    {3.0f, 0.15f, {1.0f, 0.5f, 0.0f}, 0.0f, 0.03f},  // 金星
    {4.0f, 0.2f, {0.0f, 0.0f, 1.0f}, 0.0f, 0.02f},   // 地球
    {5.0f, 0.17f, {1.0f, 0.0f, 0.0f}, 0.0f, 0.015f}, // 火星
    {6.0f, 0.4f, {1.0f, 1.0f, 0.0f}, 0.0f, 0.01f},   // 木星
    {7.0f, 0.35f, {0.5f, 0.5f, 0.5f}, 0.0f, 0.008f}, // 土星
    {8.0f, 0.3f, {0.0f, 1.0f, 1.0f}, 0.0f, 0.006f},  // 天王星
    {9.0f, 0.25f, {0.0f, 0.0f, 1.0f}, 0.0f, 0.005f}  // 海王星
};

// 定义月球
Planet moon = {0.3f, 0.05f, {0.8f, 0.8f, 0.8f}, 0.0f, 0.1f}; // 月球

// 视角控制变量
float camera_angle_x = 0.0f;
float camera_angle_y = 0.0f;
float camera_distance = 20.0f; // 初始视角距离
int last_mouse_x, last_mouse_y;
bool is_dragging = false;

// 速度倍数
float speed_multiplier = 1.0f;

// 纹理ID
GLuint textureID;

// 加载纹理函数
GLuint loadTexture(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return 0;

    unsigned char header[54];
    fread(header, 1, 54, file);

    unsigned int dataPos = *(int *)&(header[0x0A]);
    unsigned int imageSize = *(int *)&(header[0x22]);
    unsigned int width = *(int *)&(header[0x12]);
    unsigned int height = *(int *)&(header[0x16]);

    if (imageSize == 0)
        imageSize = width * height * 3;
    if (dataPos == 0)
        dataPos = 54;

    unsigned char *data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return textureID;
}

// 显示速度倍数
void displaySpeed()
{
    char speed_text[20];
    sprintf(speed_text, "SPEED: %.1f倍", speed_multiplier);

    // 设置文本颜色为红色
    glDisable(GL_LIGHTING); // 禁用光照以确保颜色设置正确
    glColor3f(1.0f, 0.0f, 0.0f);

    // 设置文本位置为左上角
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600); // 设置正交投影，窗口大小为800x600
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2f(10, 580); // 设置文本位置，距离左上角有一定的偏移

    // 绘制文本
    for (char *c = speed_text; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING); // 重新启用光照
}

// 显示函数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓冲区

    // 绘制背景
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // 设置光源
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // 设置视角
    glPushMatrix();
    glRotatef(camera_angle_x, 1.0f, 0.0f, 0.0f);
    glRotatef(camera_angle_y, 0.0f, 1.0f, 0.0f);

    // 绘制太阳
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sun.color); // 设置太阳颜色
    glutSolidSphere(sun.size, 50, 50);                         // 绘制太阳
    glPopMatrix();

    // 绘制八大行星及其轨迹
    for (int i = 0; i < 8; i++)
    {
        // 绘制轨迹
        glBegin(GL_LINE_LOOP);
        glColor3fv(planets[i].color);
        for (int j = 0; j < 360; j++)
        {
            float theta = j * M_PI / 180.0f;
            float x = planets[i].distance * cos(theta);
            float z = planets[i].distance * sin(theta);
            glVertex3f(x, 0.0f, z);
        }
        glEnd();

        // 绘制行星
        glPushMatrix();
        float x = planets[i].distance * cos(planets[i].angle);
        float z = planets[i].distance * sin(planets[i].angle);
        glTranslatef(x, 0.0f, z);                                         // 设置行星位置
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, planets[i].color); // 设置行星颜色
        glutSolidSphere(planets[i].size, 50, 50);                         // 绘制行星

        // 绘制月球（绕地球旋转）
        if (i == 2) // 地球
        {
            glPushMatrix();
            float moon_x = moon.distance * cos(moon.angle);
            float moon_z = moon.distance * sin(moon.angle);
            glTranslatef(moon_x, 0.0f, moon_z);                         // 设置月球位置
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, moon.color); // 设置月球颜色
            glutSolidSphere(moon.size, 50, 50);                         // 绘制月球
            glPopMatrix();
        }

        // 绘制土星环
        if (i == 5) // 土星
        {
            glPushMatrix();
            glColor3f(0.8f, 0.8f, 0.5f); // 设置土星环颜色

            // 绘制内环
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < 360; j++)
            {
                float theta = j * M_PI / 180.0f;
                float ring_x = (planets[i].size + 0.1f) * cos(theta);
                float ring_z = (planets[i].size + 0.1f) * sin(theta);
                glVertex3f(ring_x, 0.0f, ring_z);
            }
            glEnd();

            // 绘制外环
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < 360; j++)
            {
                float theta = j * M_PI / 180.0f;
                float ring_x = (planets[i].size + 0.2f) * cos(theta);
                float ring_z = (planets[i].size + 0.2f) * sin(theta);
                glVertex3f(ring_x, 0.0f, ring_z);
            }
            glEnd();

            glPopMatrix();
        }

        glPopMatrix();
    }

    glPopMatrix();

    // 显示速度倍数
    displaySpeed();

    glutSwapBuffers(); // 交换前后缓冲区
}

// 更新行星和月球位置
void update(int value)
{
    for (int i = 0; i < 8; i++)
    {
        planets[i].angle += planets[i].speed; // 更新行星角度
        if (planets[i].angle > 2 * M_PI)
        {
            planets[i].angle -= 2 * M_PI;
        }
    }

    // 更新月球角度
    moon.angle += moon.speed;
    if (moon.angle > 2 * M_PI)
    {
        moon.angle -= 2 * M_PI;
    }

    glutPostRedisplay();          // 请求重新绘制
    glutTimerFunc(16, update, 0); // 16毫秒后再次调用update函数
}

// 键盘事件处理
void keyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
        for (int i = 0; i < 8; i++)
        {
            planets[i].speed *= 0.9f; // 减慢速度
        }
        moon.speed *= 0.9f;       // 减慢月球速度
        speed_multiplier *= 0.9f; // 更新速度倍数
        break;
    case GLUT_KEY_RIGHT:
        for (int i = 0; i < 8; i++)
        {
            planets[i].speed *= 1.1f; // 加快速度
        }
        moon.speed *= 1.1f;       // 加快月球速度
        speed_multiplier *= 1.1f; // 更新速度倍数
        break;
    default:
        break;
    }
    glutPostRedisplay(); // 请求重新绘制以更新速度显示
}

// 鼠标按下事件处理
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        camera_distance -= 1.0f; // 缩小视角
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        camera_distance += 1.0f; // 放大视角
    }
    if (camera_distance < 5.0f)
    {
        camera_distance = 5.0f; // 限制最小距离
    }
    if (camera_distance > 50.0f)
    {
        camera_distance = 50.0f; // 限制最大距离
    }
    glutPostRedisplay();

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        is_dragging = true;
        last_mouse_x = x;
        last_mouse_y = y;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        is_dragging = false;
    }
}

// 鼠标移动事件处理
void motion(int x, int y)
{
    if (is_dragging)
    {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;
        camera_angle_y += dx * 0.5f;
        camera_angle_x += dy * 0.5f;
        last_mouse_x = x;
        last_mouse_y = y;
        glutPostRedisplay();
    }
}

// 初始化函数
void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // 设置背景颜色为黑色
    glEnable(GL_DEPTH_TEST);              // 启用深度测试
    glEnable(GL_LIGHTING);                // 启用光照
    glEnable(GL_LIGHT0);                  // 启用0号光源

    // 设置光源属性
    GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // 设置材质属性
    GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mat_shininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // 加载星空背景纹理
    textureID = loadTexture("./textures/stars.bmp");
}

// 视口和投影变换
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0); // 设置透视投影
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, camera_distance, // 眼睛位置
              0.0, 0.0, 0.0,             // 观察点
              0.0, 1.0, 0.0);            // 上方向
}

// 主函数
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Solar System");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(keyboard);    // 注册特殊键盘事件处理函数
    glutTimerFunc(16, update, 0); // 16毫秒后调用update函数
    glutMainLoop();
    return 0;
}