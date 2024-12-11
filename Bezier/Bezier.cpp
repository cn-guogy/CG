#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>

// 控制点结构体
typedef struct
{
    float x, y;
} ControlPoint;

ControlPoint controlPoints[10];   // 控制点数组
int ncpts = 0;                 // 当前控制点数量
int width = 600, height = 600; // 窗口大小

// 计算Bernstein多项式
float bernstein(int n, int i, float t)
{
    return (float)(tgamma(n + 1) / (tgamma(i + 1) * tgamma(n - i + 1))) * pow(t, i) * pow(1 - t, n - i);
}

// 贝塞尔曲线计算函数
void bezierCurve(ControlPoint *points, int n, float t, float *x, float *y)
{
    *x = 0.0f;
    *y = 0.0f;
    for (int i = 0; i <= n; i++)
    {
        float b = bernstein(n, i, t);
        *x += points[i].x * b;
        *y += points[i].y * b;
    }
}

// 绘制控制点
void drawControlPoints()
{
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < ncpts; i++)
    {
        glVertex2f(controlPoints[i].x, controlPoints[i].y);
    }
    glEnd();

    // 绘制控制点坐标
    for (int i = 0; i < ncpts; i++)
    {
        char coord[20];
        sprintf(coord, "(%.2f, %.2f)", controlPoints[i].x, controlPoints[i].y);
        glRasterPos2f(controlPoints[i].x, controlPoints[i].y);
        for (char *c = coord; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
}

// 绘制贝塞尔方程
void drawBezierEquation()
{
    char equation[1000];
    sprintf(equation, "Bezier Equation: B(t) = ");
    for (int i = 0; i < ncpts; i++)
    {
        char term[100];
        if (i == 0)
            sprintf(term, "B%d,%d(t) * P%d", i, ncpts - 1, i);
        else
            sprintf(term, " + B%d,%d(t) * P%d", i, ncpts - 1, i);
        strcat(equation, term);
    }
    glRasterPos2f(-0.9f, 0.9f); // 设置文本位置
    for (char *c = equation; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// 生成渐变颜色
void getRainbowColor(float t, float *r, float *g, float *b)
{
    // t 从 0 到 1 生成从红到紫的彩虹渐变
    if (t < 0.2f)
    {
        *r = 1.0f;
        *g = t / 0.2f;
        *b = 0.0f;
    }
    else if (t < 0.4f)
    {
        *r = 1.0f - (t - 0.2f) / 0.2f;
        *g = 1.0f;
        *b = 0.0f;
    }
    else if (t < 0.6f)
    {
        *r = 0.0f;
        *g = 1.0f;
        *b = (t - 0.4f) / 0.2f;
    }
    else if (t < 0.8f)
    {
        *r = 0.0f;
        *g = 1.0f - (t - 0.6f) / 0.2f;
        *b = 1.0f;
    }
    else
    {
        *r = (t - 0.8f) / 0.2f;
        *g = 0.0f;
        *b = 1.0f;
    }
}

// 绘制Bezier曲线
void drawBezierCurve()
{
    if (ncpts < 2)
        return; // 至少需要两个点

    glLineWidth(5.0f); // 设置线条宽度为5.0
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 1.0; t += 0.001) // 增加采样点
    {
        float x, y;
        bezierCurve(controlPoints, ncpts - 1, t, &x, &y);

        float r, g, b;
        getRainbowColor(t, &r, &g, &b); // 获取渐变颜色
        glColor3f(r, g, b);

        glVertex2f(x, y);
    }
    glEnd();
}

// 绘制函数
void display()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 设置背景颜色为白色
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 0.0f); // 设置绘制颜色为黑色

    // 绘制控制点
    drawControlPoints();

    // 绘制Bezier曲线
    drawBezierCurve();

    // 绘制贝塞尔方程
    glColor3f(0.0f, 0.0f, 0.0f); // 设置文本颜色为黑色
    drawBezierEquation();

    glFlush();
}

// 鼠标点击事件处理函数
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        controlPoints[ncpts].x = (float)x / (width / 2) - 1.0f;
        controlPoints[ncpts].y = 1.0f - (float)y / (height / 2);
        ncpts++;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Bezier Curve");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glutDisplayFunc(display);
    glutMouseFunc(mouse);

    glutMainLoop();
    return 0;
}